/** \file
	\brief Audio recorder
*/

#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "baresip_recorder.h"
#include "wavfile.h"
#include "opusenc.h"

#define DEBUG_MODULE "recorder"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

static recorder_state_h *recorder_state_handler = NULL;

static DWORD WINAPI ThreadRecWrite(LPVOID data);

void recorder_init(recorder_state_h *state_h) {
	recorder_state_handler = state_h;
}

static void notify_state(struct recorder_st *recorder, enum recorder_state state) {
	if (recorder_state_handler) {
		recorder_state_handler(recorder, state);
	}
}

struct recorder_st {
	bool active;
	bool paused;
	bool terminated;
	FILE *pFile;
	OggOpusEnc *enc;
	OggOpusComments *comments;
	HANDLE thread;

	struct lock* rec_lock;

	unsigned int call_uid;

	bool filename_set;
	char filename[512];
	unsigned int channels;
	enum recorder_side side;
	enum recorder_file_format format;
	unsigned int bitrate;
	bool pause_request;

	unsigned frame_size;
	unsigned int srate;
	struct aubuf *abrx;
	struct aubuf *abtx;
};

int recorder_start(struct recorder_st *st, unsigned int call_uid, const char* const file, unsigned int rec_channels, enum recorder_side rec_side, enum recorder_file_format rec_format, unsigned int rec_bitrate) {
	if (st->rec_lock == NULL)
		return -1;
	lock_write_get(st->rec_lock);
	st->call_uid = call_uid;
	if (st->filename_set == false) {
		st->channels = rec_channels;
		st->side = rec_side;
		st->format = rec_format;
		st->bitrate = rec_bitrate;
		str_ncpy(st->filename, file, sizeof(st->filename));
		st->filename_set = true;
	}
	st->pause_request = false;
	lock_rel(st->rec_lock);
	return 0;
}

void recorder_pause_resume(struct recorder_st *st) {
	st->pause_request = !st->pause_request;
}

void recorder_pause(struct recorder_st *st) {
	st->pause_request = true;
}

unsigned int recorder_get_call_uid(struct recorder_st *st) {
	assert(st);
	return st->call_uid;
}

struct enc_st {
	struct aufilt_enc_st af;  /* base class */
	struct recorder_st *st;
};

struct dec_st {
	struct aufilt_dec_st af;  /* base class */
	struct recorder_st *st;
};

static void enc_destructor(void *arg)
{
	struct enc_st *st = arg;

	list_unlink(&st->af.le);
	mem_deref(st->st);
}

static void dec_destructor(void *arg)
{
	struct dec_st *st = arg;

	list_unlink(&st->af.le);
	mem_deref(st->st);
}

static void recorder_destructor(void *arg)
{
	struct recorder_st *st = arg;

	DEBUG_NOTICE("Unloading recorder\n");

	st->active = false;
	while (!st->terminated) {
		Sleep(10);
	}
	st->paused = false;
	Sleep(10);
	CloseHandle(st->thread);

	mem_deref(st->abrx);
	mem_deref(st->abtx);

	if (st->pFile) {
		wavfile_close(st->pFile);
		st->pFile = NULL;
		notify_state(st, RECORDER_STATE_IDLE);		
	}
	if (st->enc) {
		ope_encoder_drain(st->enc);
		ope_encoder_destroy(st->enc);
		st->enc = NULL;
	}
	if (st->comments) {
		ope_comments_destroy(st->comments);
		st->comments = NULL;
	}
	if (st->rec_lock) {
		lock_rel(st->rec_lock);
		mem_deref(st->rec_lock);
		st->rec_lock = NULL;
	}	
}


static int recorder_alloc(struct recorder_st **stp, void **ctx, struct aufilt_prm *prm)
{
	DWORD dwtid;
	struct recorder_st *st;
	int err = 0;

	if (!stp || !ctx || !prm)
		return EINVAL;

	if (prm->ch != 1) {
    	return EINVAL;
	}

	if (*ctx) {
		*stp = mem_ref(*ctx);
		return 0;
	}

	st = mem_zalloc(sizeof(*st), recorder_destructor);
	if (!st)
		return ENOMEM;

	err = lock_alloc(&st->rec_lock);
	if (err)
		goto out;
	st->channels = prm->ch;
	st->side = RECORDER_SIDE_BOTH;
	st->format = RECORDER_FILE_FORMAT_OPUS_OGG;
	st->bitrate = 64000;
	st->pause_request = false;

	st->srate = prm->srate;

	st->frame_size = prm->frame_size;
	err = aubuf_alloc(&st->abrx, prm->frame_size, prm->frame_size*250);
	if (err)
		goto out;
	err = aubuf_alloc(&st->abtx, prm->frame_size, prm->frame_size*250);
	if (err)
		goto out;

	st->active = true;
	st->thread = CreateThread(NULL, 0, ThreadRecWrite, st, 0, &dwtid);
	if (st->thread == NULL) {
		DEBUG_WARNING("recorder: failed to create worker thread\n");
		err = ENOMEM;
		goto out;
	}

 out:
	if (err)
		mem_deref(st);
	else {
		*ctx = *stp = st;
		DEBUG_NOTICE("Recorder loaded: enc=%uHz\n", prm->srate);		
	}

	return err;
}


static int encode_update(struct aufilt_enc_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm)
{
	struct enc_st *st;
	int err;

	if (!stp || !ctx || !af || !prm)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), enc_destructor);
	if (!st)
		return ENOMEM;

	err = recorder_alloc(&st->st, ctx, prm);

	if (err)
		mem_deref(st);
	else
		*stp = (struct aufilt_enc_st *)st;

	return err;
}


static int decode_update(struct aufilt_dec_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm)
{
	struct dec_st *st;
	int err;

	if (!stp || !ctx || !af || !prm)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), dec_destructor);
	if (!st)
		return ENOMEM;

	err = recorder_alloc(&st->st, ctx, prm);

	if (err)
		mem_deref(st);
	else
		*stp = (struct aufilt_dec_st *)st;

	return err;
}


static int encode(struct aufilt_enc_st *st, int16_t *sampv, size_t *sampc)
{
	struct enc_st *est = (struct enc_st *)st;
	struct recorder_st *rec = est->st;

	aubuf_write(rec->abrx, (uint8_t *)sampv, (*sampc)*sizeof(int16_t));

	return 0;
}


static int decode(struct aufilt_dec_st *st, int16_t *sampv, size_t *sampc)
{
	struct dec_st *dst = (struct dec_st *)st;
	struct recorder_st *rec = dst->st;

	aubuf_write(rec->abtx, (uint8_t *)sampv, (*sampc)*sizeof(int16_t));

	return 0;
}

struct recorder_st* baresip_recorder_st_from_enc(struct aufilt_enc_st *enc)
{
	struct enc_st *est = (struct enc_st *)enc;
	return est->st;
}

struct recorder_st* baresip_recorder_st_from_dec(struct aufilt_dec_st *dec)
{
	struct dec_st *dst = (struct dec_st *)dec;
	return dst->st;
}


static struct aufilt recorder = {
	LE_INIT, "recorder", encode_update, encode, decode_update, decode
};


static int module_init(void)
{
	aufilt_register(&recorder);
	return 0;
}


static int module_close(void)
{
	aufilt_unregister(&recorder);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(recorder) = {
	"recorder",
	"filter",
	module_init,
	module_close
};

/** This thread is intended to decouple disk write (potentially blocking
	for a long time in case of slow or networked media) from
	audio thread
*/
DWORD WINAPI ThreadRecWrite(LPVOID data)
{
	struct recorder_st *rec = (struct recorder_st*)data;

	while (rec->active)
	{
		char bufrx[8000];
		char buftx[8000];
		int cnt, i;
		size_t delta;		
		size_t sizerx = aubuf_cur_size(rec->abrx);
		size_t sizetx = aubuf_cur_size(rec->abrx);
		//DEBUG_WARNING("sizerx=%d, sizetx=%d\n", (int)sizerx, (int)sizetx);

		if (rec->pFile == NULL && rec->enc == NULL) {
			lock_write_get(rec->rec_lock);
			if (rec->filename_set) {
				if (rec->format == RECORDER_FILE_FORMAT_OPUS_OGG) {
					int error;
					/** \todo VBR? */
					rec->comments = ope_comments_create();
					/** \todo OGG comments? */
				#if 0
					ope_comments_add(rec->comments, "ARTIST", "Someone");
					ope_comments_add(rec->comments, "TITLE", "Some track");
				#endif
					rec->enc = ope_encoder_create_file(rec->filename, rec->comments, rec->srate, rec->channels, 0, &error);
					if (!rec->enc) {
						DEBUG_WARNING("recorder: failed to create Opus/OGG file\n");
						lock_rel(rec->rec_lock);
						break;
					}
					ope_encoder_ctl(rec->enc, OPUS_SET_BITRATE_REQUEST, rec->bitrate);
				} else {
					rec->pFile = wavfile_open(rec->filename, rec->channels, rec->srate);
					if (!rec->pFile) {
						DEBUG_WARNING("recorder: failed to create WAVE file\n");
						lock_rel(rec->rec_lock);
						break;
					}
				}
				notify_state(rec, RECORDER_STATE_ACTIVE);
			}
			lock_rel(rec->rec_lock);
			Sleep(50);
			continue;
		}

		// let's try to handle clock skew between two sound devices
		// (or some crappy hardware)
		if (sizerx > sizetx)
			delta = sizerx - sizetx;
		else
			delta = sizetx - sizerx;
		// typically delta = 0
		if (delta < rec->frame_size * 2) {
			// ignore minor differences
			delta = 0;
		}

		cnt = sizeof(bufrx);
		if (sizerx < cnt)
			cnt = sizerx;
		if (sizetx < cnt)
			cnt = sizetx;
		// this would effectively insert some silence into one of the directions
		cnt += delta/2;

		if (cnt + (delta/2) <= sizeof(bufrx))
		{
			// this would effectively insert some silence into one of the directions
			cnt += delta/2;
		}		

		aubuf_read(rec->abrx, bufrx, cnt);
		aubuf_read(rec->abtx, buftx, cnt);
	#if 0
		sizerx = aubuf_cur_size(rec->abrx);
		sizetx = aubuf_cur_size(rec->abrx);
		DEBUG_WARNING("AFTER: sizerx=%d, sizetx=%d\n", (int)sizerx, (int)sizetx);
	#endif
		if (rec->paused != rec->pause_request) {
			rec->paused = rec->pause_request;
			if (rec->paused) {
				notify_state(rec, RECORDER_STATE_PAUSED);
			} else {
            	notify_state(rec, RECORDER_STATE_ACTIVE);
			}
		}

		if (rec->paused == false) {
			if (rec->channels == 1) {
				//DEBUG_WARNING("write %d\n", cnt);
				// single channel: either one of the sides or both sides mixed (sum)
				short *dst = (short*)bufrx;
				short *src = (short*)buftx;
				if (rec->side == RECORDER_SIDE_LOCAL) {
					if (rec->pFile) {
						fwrite(bufrx, cnt, 1, rec->pFile);
					} else if (rec->enc) {
						ope_encoder_write(rec->enc, (const opus_int16*)bufrx, cnt/sizeof(short));
					}
				} else if (rec->side == RECORDER_SIDE_REMOTE) {
					if (rec->pFile) {
						fwrite(buftx, cnt, 1, rec->pFile);
					} else  if (rec->enc) {
						ope_encoder_write(rec->enc, (const opus_int16*)buftx, cnt/sizeof(short));
					}
				} else {
					// default: both parties mixed
					for (i=0; i<cnt/sizeof(short); i++) {
						// saturate to prevent roll-off
						int val = dst[i] + src[i];
						if (val < SHRT_MIN) {
							val = SHRT_MIN;
						} else if (val > SHRT_MAX) {
							val = SHRT_MAX;
						}
						dst[i] = val;
					}
					if (rec->pFile) {
						fwrite(bufrx, cnt, 1, rec->pFile);
					} else if (rec->enc) {
						ope_encoder_write(rec->enc, (const opus_int16*)bufrx, cnt/sizeof(short));
					}
				}
			} else if (rec->channels == 2) {
				short bufstereo[8000];
				short *srcA = (short*)bufrx;
				short *srcB = (short*)buftx;
				for (i=0; i<cnt/sizeof(short)*rec->channels; i += rec->channels) {
					bufstereo[i]     = *srcA++;
					bufstereo[i + 1] = *srcB++;
				}
				if (rec->pFile) {
					fwrite(bufstereo, cnt * sizeof(short), 1, rec->pFile);
				} else if (rec->enc) {
					ope_encoder_write(rec->enc, (const opus_int16*)bufstereo, cnt/rec->channels);
				}
			} else {
				assert(!"Unhandled channel count");
			}
		}
		Sleep(50);
	}
	rec->terminated = true;
	rec->filename_set = false;
	return 0;
}

