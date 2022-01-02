/**
 * @file winwave2/winwave2_src.c Windows sound driver -- source
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <rem.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <baresip.h>
#include "winwave2.h"


#define DEBUG_MODULE "winwave2"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


#define READ_BUFFERS   4
#define INC_RPOS(a) ((a) = (((a) + 1) % READ_BUFFERS))


struct ausrc_st {
	struct ausrc *as;      /* inheritance */
	struct dspbuf bufs[READ_BUFFERS];
	int pos;
	HWAVEIN wavein;
	volatile bool rdy;
	size_t inuse;
	ausrc_read_h *rh;
	void *arg;
	HANDLE thread;
	volatile bool run;
	CRITICAL_SECTION crit;
};


static void ausrc_destructor(void *arg) {
	struct ausrc_st *st = arg;
	int i;

	/* Mark the device for closing
	 */
	st->rdy = false;

	if (st->run) {
		st->run = false;
		(void)WaitForSingleObject(st->thread, INFINITE);
	}

	st->rh = NULL;

	waveInStop(st->wavein);
	// WIM_DATA may be sent
	waveInReset(st->wavein);

	for (i = 0; i < READ_BUFFERS; i++) {
		waveInUnprepareHeader(st->wavein, &st->bufs[i].wh,
		                      sizeof(WAVEHDR));
		mem_deref(st->bufs[i].mb);
	}

	waveInClose(st->wavein);
	DeleteCriticalSection(&st->crit);
	mem_deref(st->as);
}


static DWORD WINAPI add_wave_in(LPVOID arg) {
	struct ausrc_st *st = (struct ausrc_st *)arg;
	MMRESULT res;
	struct mbuf *mb;
	WAVEHDR *wh;
	size_t inuse;

	while (st->run) {
		Sleep(2);

		EnterCriticalSection(&st->crit);
		inuse = st->inuse;
		LeaveCriticalSection(&st->crit);

		if (!st->rdy)
			continue;

		if (inuse == READ_BUFFERS)
			continue;

		wh = &st->bufs[st->pos].wh;
		if (wh->dwFlags & WHDR_PREPARED)
			waveInUnprepareHeader(st->wavein, wh, sizeof(*wh));

		mb = st->bufs[st->pos].mb;
		wh->lpData          = (LPSTR)mb->buf;

		if (st->rh)
			st->rh((void *)wh->lpData, wh->dwBytesRecorded, st->arg);

		wh->dwBufferLength  = mb->size;
		wh->dwBytesRecorded = 0;
		wh->dwFlags         = 0;
		wh->dwUser          = (DWORD_PTR)mb;

		waveInPrepareHeader(st->wavein, wh, sizeof(*wh));

		INC_RPOS(st->pos);

		res = waveInAddBuffer(st->wavein, wh, sizeof(*wh));
		if (res != MMSYSERR_NOERROR)
			DEBUG_WARNING("winwave: add_wave_in: waveInAddBuffer failed: %d\n", res);
		else {
			EnterCriticalSection(&st->crit);
			st->inuse++;
			LeaveCriticalSection(&st->crit);
		}

	}

	return 0;
}


static void CALLBACK waveInCallback(HWAVEOUT hwo,
                                    UINT uMsg,
                                    DWORD_PTR dwInstance,
                                    DWORD_PTR dwParam1,
                                    DWORD_PTR dwParam2) {
	struct ausrc_st *st = (struct ausrc_st *)dwInstance;

	(void)hwo;
	(void)dwParam2;

	switch (uMsg) {

	case WIM_CLOSE:
		st->rdy = false;
		break;

	case WIM_OPEN:
		st->rdy = true;
		break;

	case WIM_DATA:
		EnterCriticalSection(&st->crit);
		st->inuse--;
		LeaveCriticalSection(&st->crit);
		break;

	default:
		break;
	}
}

static unsigned int find_dev(const char* name) {
	WAVEINCAPS wic;
	unsigned int i;
	int nInDevices = waveInGetNumDevs();
	for (i=0; i<nInDevices; i++) {
		if (waveInGetDevCaps(i, &wic, sizeof(WAVEINCAPS))==MMSYSERR_NOERROR) {
			if (!strcmp(name, wic.szPname)) {
				return i;
			}
		}
	}
	return WAVE_MAPPER;
}

static int read_stream_open(unsigned int dev, struct ausrc_st *st, const struct ausrc_prm *prm) {
	WAVEFORMATEX wfmt;
	MMRESULT res;
	int i, err = 0;

	/* Open an audio INPUT stream. */
	st->wavein = NULL;
	st->pos = 0;
	st->rdy = false;

	for (i = 0; i < READ_BUFFERS; i++) {
		memset(&st->bufs[i].wh, 0, sizeof(WAVEHDR));
		st->bufs[i].mb = mbuf_alloc(2 * prm->frame_size);
		if (!st->bufs[i].mb)
			return ENOMEM;
	}

	wfmt.wFormatTag      = WAVE_FORMAT_PCM;
	wfmt.nChannels       = prm->ch;
	wfmt.nSamplesPerSec  = prm->srate;
	wfmt.wBitsPerSample  = 16;
	wfmt.nBlockAlign     = (prm->ch * wfmt.wBitsPerSample) / 8;
	wfmt.nAvgBytesPerSec = wfmt.nSamplesPerSec * wfmt.nBlockAlign;
	wfmt.cbSize          = 0;

	res = waveInOpen(&st->wavein, dev, &wfmt,
	                 (DWORD_PTR) waveInCallback,
	                 (DWORD_PTR) st,
	                 CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT);
	if (res != MMSYSERR_NOERROR) {
		DEBUG_WARNING("waveInOpen: failed, status = %u\n", res);
		return E_AUDIO_SOURCE_DEV_OPEN_ERROR;
	}

	waveInStart(st->wavein);

	return err;
}


int winwave2_src_alloc(struct ausrc_st **stp, struct ausrc *as,
                       struct media_ctx **ctx,
                       struct ausrc_prm *prm, const char *device,
                       ausrc_read_h *rh, ausrc_error_h *errh, void *arg) {
	struct ausrc_st *st;
	int err;

	(void)ctx;
	(void)device;
	(void)errh;

	if (!stp || !as || !prm)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), ausrc_destructor);
	if (!st)
		return ENOMEM;

	st->as  = mem_ref(as);
	st->rh  = rh;
	st->arg = arg;

	InitializeCriticalSection(&st->crit);

	prm->fmt = AUFMT_S16LE;

	err = read_stream_open(find_dev(device), st, prm);
	if (err)
		goto out;

	st->run = true;
	st->thread = CreateThread(NULL, 0, add_wave_in, st, 0, NULL);
	if (!st->thread) {
		st->run = false;
		err = ENOMEM;
	}

out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}

