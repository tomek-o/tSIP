/**
 * @file aumix.c Audio Mixer
 *
 * Copyright (C) 2010 Creytiv.com
 */

#define _BSD_SOURCE 1
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <re.h>
#include <rem_au.h>
#include <rem_aubuf.h>
#include <rem_aufile.h>
#include <rem_aumix.h>


/** Defines an Audio mixer */
struct aumix {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	struct list srcl;
	pthread_t thread;
	struct aufile *af;
	uint32_t ptime;
	uint32_t frame_size;
	uint32_t srate;
	uint8_t ch;
	bool run;
};

/** Defines an Audio mixer source */
struct aumix_source {
	struct le le;
	int16_t *frame;
	struct aubuf *aubuf;
	struct aumix *mix;
	aumix_frame_h *fh;
	void *arg;
};


static void dummy_frame_handler(const int16_t *sampv, size_t sampc, void *arg)
{
	(void)sampv;
	(void)sampc;
	(void)arg;
}


static void destructor(void *arg)
{
	struct aumix *mix = arg;

	if (mix->run) {

		pthread_mutex_lock(&mix->mutex);
		mix->run = false;
		pthread_cond_signal(&mix->cond);
		pthread_mutex_unlock(&mix->mutex);

		pthread_join(mix->thread, NULL);
	}

	mem_deref(mix->af);
}


static void source_destructor(void *arg)
{
	struct aumix_source *src = arg;

	if (src->le.list) {
		pthread_mutex_lock(&src->mix->mutex);
		list_unlink(&src->le);
		pthread_mutex_unlock(&src->mix->mutex);
	}

	mem_deref(src->aubuf);
	mem_deref(src->frame);
	mem_deref(src->mix);
}


static void *aumix_thread(void *arg)
{
	uint8_t *silence, *frame, *base_frame;
	struct aumix *mix = arg;
	int16_t *mix_frame;
	uint64_t ts = 0;

	silence   = mem_zalloc(mix->frame_size*2, NULL);
	frame     = mem_alloc(mix->frame_size*2, NULL);
	mix_frame = mem_alloc(mix->frame_size*2, NULL);

	if (!silence || !frame || !mix_frame)
		goto out;

	pthread_mutex_lock(&mix->mutex);

	while (mix->run) {

		struct le *le;
		uint64_t now;

		if (!mix->srcl.head) {
			mix->af = mem_deref(mix->af);
			pthread_cond_wait(&mix->cond, &mix->mutex);
			ts = 0;
		}
		else {
			pthread_mutex_unlock(&mix->mutex);
			(void)usleep(4000);
			pthread_mutex_lock(&mix->mutex);
		}

		now = tmr_jiffies();
		if (!ts)
			ts = now;

		if (ts > now)
			continue;

		if (mix->af) {

			size_t n = mix->frame_size*2;

			if (aufile_read(mix->af, frame, &n) || n == 0) {
				mix->af = mem_deref(mix->af);
				base_frame = silence;
			}
			else if (n < mix->frame_size*2) {
				memset(frame + n, 0, mix->frame_size*2 - n);
				mix->af = mem_deref(mix->af);
				base_frame = frame;
			}
			else {
				base_frame = frame;
			}
		}
		else {
			base_frame = silence;
		}

		for (le=mix->srcl.head; le; le=le->next) {

			struct aumix_source *src = le->data;

			aubuf_read_samp(src->aubuf, src->frame,
					mix->frame_size);
		}

		for (le=mix->srcl.head; le; le=le->next) {

			struct aumix_source *src = le->data;
			struct le *cle;

			memcpy(mix_frame, base_frame, mix->frame_size*2);

			for (cle=mix->srcl.head; cle; cle=cle->next) {

				struct aumix_source *csrc = cle->data;
				size_t i;
#if 1
				/* skip self */
				if (csrc == src)
					continue;
#endif
				for (i=0; i<mix->frame_size; i++)
					mix_frame[i] += csrc->frame[i];
			}

			src->fh(mix_frame, mix->frame_size, src->arg);
		}

		ts += mix->ptime;
	}

	pthread_mutex_unlock(&mix->mutex);

 out:
	mem_deref(mix_frame);
	mem_deref(silence);
	mem_deref(frame);

	return NULL;
}


/**
 * Allocate a new Audio mixer
 *
 * @param mixp  Pointer to allocated audio mixer
 * @param srate Sample rate in [Hz]
 * @param ch    Number of channels
 * @param ptime Packet time in [ms]
 *
 * @return 0 for success, otherwise error code
 */
int aumix_alloc(struct aumix **mixp, uint32_t srate,
		uint8_t ch, uint32_t ptime)
{
	struct aumix *mix;
	int err;

	if (!mixp || !srate || !ch || !ptime)
		return EINVAL;

	mix = mem_zalloc(sizeof(*mix), destructor);
	if (!mix)
		return ENOMEM;

	mix->ptime      = ptime;
	mix->frame_size = srate * ch * ptime / 1000;
	mix->srate      = srate;
	mix->ch         = ch;

	err = pthread_mutex_init(&mix->mutex, NULL);
	if (err)
		goto out;

	err = pthread_cond_init(&mix->cond, NULL);
	if (err)
		goto out;

	mix->run = true;

	err = pthread_create(&mix->thread, NULL, aumix_thread, mix);
	if (err) {
		mix->run = false;
		goto out;
	}

 out:
	if (err)
		mem_deref(mix);
	else
		*mixp = mix;

	return err;
}


/**
 * Load audio file for mixer announcements
 *
 * @param mix      Audio mixer
 * @param filepath Filename of audio file with complete path
 *
 * @return 0 for success, otherwise error code
 */
int aumix_playfile(struct aumix *mix, const char *filepath)
{
	struct aufile_prm prm;
	struct aufile *af;
	int err;

	if (!mix || !filepath)
		return EINVAL;

	err = aufile_open(&af, &prm, filepath, AUFILE_READ);
	if (err)
		return err;

	if (prm.fmt != AUFMT_S16LE || prm.srate != mix->srate ||
	    prm.channels != mix->ch) {
		mem_deref(af);
		return EINVAL;
	}

	pthread_mutex_lock(&mix->mutex);
	mem_deref(mix->af);
	mix->af = af;
	pthread_mutex_unlock(&mix->mutex);

	return 0;
}


/**
 * Count number of audio sources in the audio mixer
 *
 * @param mix Audio mixer
 *
 * @return Number of audio sources
 */
uint32_t aumix_source_count(const struct aumix *mix)
{
	if (!mix)
		return 0;

	return list_count(&mix->srcl);
}


/**
 * Allocate an audio mixer source
 *
 * @param srcp Pointer to allocated audio source
 * @param mix  Audio mixer
 * @param fh   Mixer frame handler
 * @param arg  Handler argument
 *
 * @return 0 for success, otherwise error code
 */
int aumix_source_alloc(struct aumix_source **srcp, struct aumix *mix,
		       aumix_frame_h *fh, void *arg)
{
	struct aumix_source *src;
	size_t sz;
	int err;

	if (!srcp || !mix)
		return EINVAL;

	src = mem_zalloc(sizeof(*src), source_destructor);
	if (!src)
		return ENOMEM;

	src->mix = mem_ref(mix);
	src->fh  = fh ? fh : dummy_frame_handler;
	src->arg = arg;

	sz = mix->frame_size*2;

	src->frame = mem_alloc(sz, NULL);
	if (!src->frame) {
		err = ENOMEM;
		goto out;
	}

	err = aubuf_alloc(&src->aubuf, sz * 6, sz * 12);
	if (err)
		goto out;

 out:
	if (err)
		mem_deref(src);
	else
		*srcp = src;

	return err;
}


/**
 * Enable/disable aumix source
 *
 * @param src    Audio mixer source
 * @param enable True to enable, false to disable
 */
void aumix_source_enable(struct aumix_source *src, bool enable)
{
	struct aumix *mix;

	if (!src)
		return;

	if (src->le.list && enable)
		return;

	if (!src->le.list && !enable)
		return;

	mix = src->mix;

	pthread_mutex_lock(&mix->mutex);

	if (enable) {
		list_append(&mix->srcl, &src->le, src);
		pthread_cond_signal(&mix->cond);
	}
	else {
		list_unlink(&src->le);
	}

	pthread_mutex_unlock(&mix->mutex);
}


/**
 * Write PCM samples for a given source to the audio mixer
 *
 * @param src   Audio mixer source
 * @param sampv PCM samples
 * @param sampc Number of samples
 *
 * @return 0 for success, otherwise error code
 */
int aumix_source_put(struct aumix_source *src, const int16_t *sampv,
		     size_t sampc)
{
	if (!src || !sampv)
		return EINVAL;

	return aubuf_write_samp(src->aubuf, sampv, sampc);
}


/**
 * Flush the audio buffer of a given audio mixer source
 *
 * @param src Audio mixer source
 */
void aumix_source_flush(struct aumix_source *src)
{
	if (!src)
		return;

	aubuf_flush(src->aubuf);
}
