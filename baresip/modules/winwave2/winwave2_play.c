/**
 * @file winwave2/winwave2_play.c Windows sound driver -- playback
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


#define WRITE_BUFFERS  4
#define INC_WPOS(a) ((a) = (((a) + 1) % WRITE_BUFFERS))


struct auplay_st {
	struct auplay *ap;      /* inheritance */
	struct dspbuf bufs[WRITE_BUFFERS];
	int pos;
	HWAVEOUT waveout;
	bool rdy;
	bool closed;
	size_t inuse;
	auplay_write_h *wh;
	void *arg;
	HANDLE thread;
	volatile bool run;
	CRITICAL_SECTION crit;
};


static void auplay_destructor(void *arg) {
	struct auplay_st *st = arg;
	int i;

	if (st->run) {
		st->run = false;
		(void)WaitForSingleObject(st->thread, INFINITE);
	}

	st->wh = NULL;

	waveOutReset(st->waveout);

	for (i = 0; i < WRITE_BUFFERS; i++) {
		waveOutUnprepareHeader(st->waveout, &st->bufs[i].wh, sizeof(WAVEHDR));
		mem_deref(st->bufs[i].mb);
	}

	waveOutClose(st->waveout);

	for (i=0; i<100; i++) {
		if (st->closed)
			break;
		Sleep(10);
	}

	DeleteCriticalSection(&st->crit);
	mem_deref(st->ap);
}


static DWORD WINAPI dsp_write(LPVOID arg) {
	MMRESULT res;
	WAVEHDR *wh;
	struct mbuf *mb;
	size_t inuse;

	struct auplay_st *st = (struct auplay_st *)arg;

	while (st->run) {
		Sleep(5);

		EnterCriticalSection(&st->crit);
		inuse = st->inuse;
		LeaveCriticalSection(&st->crit);

		if (!st->rdy)
			continue;

		if (inuse == WRITE_BUFFERS)
			continue;

		wh = &st->bufs[st->pos].wh;
		if (wh->dwFlags & WHDR_PREPARED)
			waveOutUnprepareHeader(st->waveout, wh, sizeof(*wh));

		mb = st->bufs[st->pos].mb;
		wh->lpData = (LPSTR)mb->buf;

		if (st->wh)
			st->wh((void *)mb->buf, mb->size, st->arg);

		wh->dwBufferLength = mb->size;
		wh->dwFlags = 0;
		wh->dwUser = (DWORD_PTR)mb;

		waveOutPrepareHeader(st->waveout, wh, sizeof(*wh));

		INC_WPOS(st->pos);

		res = waveOutWrite(st->waveout, wh, sizeof(*wh));
		if (res != MMSYSERR_NOERROR)
			DEBUG_WARNING("winwave: dsp_write: waveOutWrite: failed: %d\n", res);
		else {
			EnterCriticalSection(&st->crit);
			st->inuse++;
			LeaveCriticalSection(&st->crit);
		}

	}
	return 0;
}


static void CALLBACK waveOutCallback(HWAVEOUT hwo,
                                     UINT uMsg,
                                     DWORD_PTR dwInstance,
                                     DWORD_PTR dwParam1,
                                     DWORD_PTR dwParam2) {
	struct auplay_st *st = (struct auplay_st *)dwInstance;

	(void)hwo;
	(void)dwParam2;

	switch (uMsg) {

	case WOM_OPEN:
		st->rdy = true;
		break;

	case WOM_DONE:
		EnterCriticalSection(&st->crit);
		st->inuse--;
		LeaveCriticalSection(&st->crit);
		break;

	case WOM_CLOSE:
		st->rdy = false;
		st->closed = true;
		break;

	default:
		break;
	}
}

static unsigned int find_dev(const char* dev) {
	WAVEOUTCAPS wic;
	unsigned int i;
	int nInDevices = waveOutGetNumDevs();
	for (i=0; i<nInDevices; i++) {
		if (waveOutGetDevCaps(i, &wic, sizeof(WAVEOUTCAPS))==MMSYSERR_NOERROR) {
			if (!strcmp(dev, wic.szPname)) {
				return i;
			}
		}
	}
	return WAVE_MAPPER;
}

static int write_stream_open(unsigned int dev, struct auplay_st *st,
                             const struct auplay_prm *prm) {
	WAVEFORMATEX wfmt;
	MMRESULT res;
	int i;

	/* Open an audio I/O stream. */
	st->waveout = NULL;
	st->pos = 0;
	st->rdy = false;
	st->closed = false;

	for (i = 0; i < WRITE_BUFFERS; i++) {
		memset(&st->bufs[i].wh, 0, sizeof(WAVEHDR));
		st->bufs[i].mb = mbuf_alloc(2 * prm->frame_size);
	}

	wfmt.wFormatTag      = WAVE_FORMAT_PCM;
	wfmt.nChannels       = prm->ch;
	wfmt.nSamplesPerSec  = prm->srate;
	wfmt.wBitsPerSample  = 16;
	wfmt.nBlockAlign     = (prm->ch * wfmt.wBitsPerSample) / 8;
	wfmt.nAvgBytesPerSec = wfmt.nSamplesPerSec * wfmt.nBlockAlign;
	wfmt.cbSize          = 0;

	res = waveOutOpen(&st->waveout, dev, &wfmt,
	                  (DWORD_PTR) waveOutCallback,
	                  (DWORD_PTR) st,
	                  CALLBACK_FUNCTION | WAVE_FORMAT_DIRECT);
	if (res != MMSYSERR_NOERROR) {
		DEBUG_WARNING("waveOutOpen: failed %d\n", res);
		return E_AUDIO_OUTPUT_DEV_OPEN_ERROR;
	}

	return 0;
}


int winwave2_play_alloc(struct auplay_st **stp, struct auplay *ap,
                        struct auplay_prm *prm, const char *device,
                        auplay_write_h *wh, void *arg) {
	struct auplay_st *st;
	int i, err;
	(void)device;

	if (!stp || !ap || !prm)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), auplay_destructor);
	if (!st)
		return ENOMEM;

	st->ap  = mem_ref(ap);
	st->wh  = wh;
	st->arg = arg;

	InitializeCriticalSection(&st->crit);

	prm->fmt = AUFMT_S16LE;

	err = write_stream_open(find_dev(device), st, prm);
	if (err)
		goto out;

	st->run = true;
	st->thread = CreateThread(NULL, 0, dsp_write, st, 0, NULL);
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

