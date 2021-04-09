/**
 * @file src/play.c  Audio-file player
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h> 
#include <stdlib.h>
#include <string.h>
#include <rem.h>
#include <baresip.h>
#include "core.h"


#define DEBUG_MODULE "play"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


enum {SILENCE_DUR = 2000};

/** Audio file player */
struct play {
	struct le le;
	struct play **playp;
	struct lock *lock;
	struct mbuf *mb;
	struct auplay_st *auplay;
	struct tmr tmr;
	int repeat;
	bool eof;
	bool loop_without_silence;
};


static char play_path[512] = {0};
static struct list playl;
static struct config_audio cfg_audio;


static void tmr_polling(void *arg);


static void tmr_stop(void *arg)
{
	struct play *play = arg;
	mem_deref(play);
}


static void tmr_repeat(void *arg)
{
	struct play *play = arg;

	lock_write_get(play->lock);

	play->mb->pos = 0;
	play->eof = false;

	tmr_start(&play->tmr, 1000, tmr_polling, arg);

	lock_rel(play->lock);
}


static void tmr_polling(void *arg)
{
	struct play *play = arg;

	lock_write_get(play->lock);

	tmr_start(&play->tmr, 1000, tmr_polling, arg);

	if (play->eof) {
		if (play->repeat > 0)
			play->repeat--;

		if (play->repeat == 0)
			tmr_start(&play->tmr, 1, tmr_stop, arg);
		else
			tmr_start(&play->tmr, SILENCE_DUR, tmr_repeat, arg);
	}

	lock_rel(play->lock);
}


/**
 * NOTE: DSP cannot be destroyed inside handler
 */
static bool write_handler(uint8_t *buf, size_t sz, void *arg)
{
	struct play *play = arg;

	lock_write_get(play->lock);

	if (play->eof)
		goto silence;

	if (play->loop_without_silence && mbuf_get_left(play->mb) < sz &&
			(play->repeat > 0 || play->repeat < 0 /* repeat indefinitely */)) {
		play->mb->pos = 0;
		play->repeat--;
	}
	
	if (mbuf_get_left(play->mb) < sz) {
		play->eof = true;
	}
	else {
		(void)mbuf_read_mem(play->mb, buf, sz);
	}

 silence:
	if (play->eof)
		memset(buf, 0, sz);

	lock_rel(play->lock);

	return true;
}


static void destructor(void *arg)
{
	struct play *play = arg;

	list_unlink(&play->le);
	tmr_cancel(&play->tmr);

	lock_write_get(play->lock);
	play->eof = true;
	lock_rel(play->lock);

	mem_deref(play->auplay);
	mem_deref(play->mb);
	mem_deref(play->lock);

	if (play->playp)
		*play->playp = NULL;
}


static int aufile_load(struct mbuf *mb, const char *filename,
		       uint32_t *srate, uint8_t *channels)
{
	struct aufile_prm prm;
	struct aufile *af;
	int err;

	err = aufile_open(&af, &prm, filename, AUFILE_READ);
	if (err)
		return err;

	while (!err) {
		uint8_t buf[4096];
		size_t i, n;

		n = sizeof(buf);

		err = aufile_read(af, buf, &n);
		if (err || !n)
			break;

		switch (prm.fmt) {

		case AUFMT_S16LE:
			err = mbuf_write_mem(mb, buf, n);
			break;

		case AUFMT_PCMA:
			for (i=0; i<n; i++) {
				err |= mbuf_write_u16(mb,
						      g711_alaw2pcm(buf[i]));
			}
			break;

		case AUFMT_PCMU:
			for (i=0; i<n; i++) {
				err |= mbuf_write_u16(mb,
						      g711_ulaw2pcm(buf[i]));
			}
			break;

		default:
			err = ENOSYS;
			break;
		}
	}

	mem_deref(af);

	if (!err) {
		mb->pos = 0;

		*srate    = prm.srate;
		*channels = prm.channels;
	}

	return err;
}


/**
 * Play a tone from a PCM buffer
 *
 * @param playp    Pointer to allocated player object
 * @param mod      Audio module type used for playing
 * @param dev      Audio device name used for playing
 * @param tone     PCM buffer to play
 * @param srate    Sampling rate
 * @param ch       Number of channels
 * @param repeat   Number of times to repeat
 *
 * @return 0 if success, otherwise errorcode
 */
int play_tone(struct play **playp, const char *mod, const char *dev, struct mbuf *tone, uint32_t srate,
		  uint8_t ch, int repeat, bool loop_without_silence)
{
	struct auplay_prm wprm;
	struct play *play;
	int err;

	if (playp && *playp)
		return EALREADY;

	play = mem_zalloc(sizeof(*play), destructor);
	if (!play)
		return ENOMEM;

	tmr_init(&play->tmr);
	play->repeat = repeat;
	play->loop_without_silence = loop_without_silence;
	play->mb     = mem_ref(tone);

	err = lock_alloc(&play->lock);
	if (err)
		goto out;

	wprm.fmt        = AUFMT_S16LE;
	wprm.ch         = ch;
	wprm.srate      = srate;
	wprm.frame_size = srate * ch * 100 / 1000;

	err = auplay_alloc(&play->auplay, mod, &wprm,
			   dev, write_handler, play);
	if (err)
		goto out;

	list_append(&playl, &play->le, play);
	tmr_start(&play->tmr, 1000, tmr_polling, play);

 out:
	if (err) {
		mem_deref(play);
	}
	else if (playp) {
		play->playp = playp;
		*playp = play;
	}

	return err;
}


/**
 * Play an audio file in WAV format
 *
 * @param playp    Pointer to allocated player object
 * @param mod      Audio module type used for playing
 * @param dev      Audio device name used for playing 
 * @param filename Name of WAV file to play
 * @param repeat   Number of times to repeat
 * @param loop_without_silence Concatenate samples when looping, without additional silence
 *
 * @return 0 if success, otherwise errorcode
 */
int play_file(struct play **playp, const char *mod, const char *dev, const char *filename, int repeat, bool loop_without_silence)
{
	struct mbuf *mb;
	char path[768];
	uint32_t srate;
	uint8_t ch;
	int err;

	if (playp && *playp)
		return EALREADY;

	if (re_snprintf(path, sizeof(path), "%s%s",
			play_path, filename) < 0)
		return ENOMEM;

	mb = mbuf_alloc(1024);
	if (!mb)
		return ENOMEM;

	err = aufile_load(mb, path, &srate, &ch);
	if (err) {
		DEBUG_WARNING("Could not load %s: %m\n", path, err);
		goto out;
	}

	err = play_tone(playp, mod, dev, mb, srate, ch, repeat, loop_without_silence);

 out:
	mem_deref(mb);

	return err;
}


void play_init(const struct config *cfg)
{
	if (!cfg)
		return;

	cfg_audio = cfg->audio;
}


/**
 * Close all active audio players
 */
void play_close(void)
{
	list_flush(&playl);
}


void play_set_path(const char *path)
{
	str_ncpy(play_path, path, sizeof(play_path));
}
