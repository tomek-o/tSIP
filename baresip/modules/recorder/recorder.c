/** \file
	\brief Audio recorder
*/

#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <stdio.h>
#include <assert.h>
#include "recorder.h"
#include "wavfile.h"

#define DEBUG_MODULE "recorder"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

static DWORD WINAPI ThreadRecWrite(LPVOID data);

static struct lock* rec_lock = NULL;
static bool filename_set = false;
static char filename[512];
static unsigned int channels = 1;

int recorder_start(const char* const file, unsigned int rec_channels) {
	if (rec_lock == NULL)
		return -1;
	lock_write_get(rec_lock);
	filename_set = false;
	channels = rec_channels;
	strncpy(filename, file, sizeof(filename));
	filename[sizeof(filename)-1] = '\0';
	filename_set = true;
	lock_rel(rec_lock);
	return 0;
}

struct recorder_st {
	bool active;
	bool terminated;
	FILE *pFile;
	HANDLE thread;

	unsigned frame_size;
	unsigned int srate;
	struct aubuf *abrx;
	struct aubuf *abtx;
};

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
   	Sleep(10);	
	CloseHandle(st->thread);

	mem_deref(st->abrx);
	mem_deref(st->abtx);

	if (st->pFile) {
		wavfile_close(st->pFile);
		st->pFile = NULL;
	}
}


static int recorder_alloc(struct recorder_st **stp, void **ctx, struct aufilt_prm *prm)
{
	DWORD dwtid;
	struct recorder_st *st;
	int err = 0, tmp, fl;

    filename_set = false;

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

static struct aufilt recorder = {
	LE_INIT, "recorder", encode_update, encode, decode_update, decode
};


static int module_init(void)
{
	int err = lock_alloc(&rec_lock);
	if (err)
		return err;
	aufilt_register(&recorder);
	return 0;
}


static int module_close(void)
{
    mem_deref(rec_lock);
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

		if (rec->pFile == NULL) {
			lock_write_get(rec_lock);
			if (filename_set) {
				rec->pFile = wavfile_open(filename, channels, rec->srate);
				if (!rec->pFile) {
					DEBUG_WARNING("recorder: failed to create file\n");
					lock_rel(rec_lock);
					break;
				}
			}
			lock_rel(rec_lock);
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

		aubuf_read(rec->abrx, bufrx, cnt);
		aubuf_read(rec->abtx, buftx, cnt);

		sizerx = aubuf_cur_size(rec->abrx);
		sizetx = aubuf_cur_size(rec->abrx);
		//DEBUG_WARNING("AFTER: sizerx=%d, sizetx=%d\n", (int)sizerx, (int)sizetx);

		if (channels == 1) {
			//DEBUG_WARNING("write %d\n", cnt);
			short *dst = (short*)bufrx;
			short *src = (short*)buftx;
			for (i=0; i<cnt/sizeof(short); i++) {
				dst[i] += src[i];
			}
			fwrite(bufrx, cnt, 1, rec->pFile);
		} else if (channels == 2) {
			short bufstereo[8000];
			short *srcA = (short*)bufrx;
			short *srcB = (short*)buftx;
			for (i=0; i<cnt/sizeof(short)*channels; i += channels) {
				bufstereo[i]     = *srcA++;
				bufstereo[i + 1] = *srcB++;
			}
			fwrite(bufstereo, cnt * sizeof(short), 1, rec->pFile);
		} else {
			assert(!"Unhandled channel count");
		}

		Sleep(50);
	}
	rec->terminated = true;
	return 0;
}

