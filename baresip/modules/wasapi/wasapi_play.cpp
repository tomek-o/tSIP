/**
 * @file wasapi_play.cpp WASAPI sound driver -- player
 */
#include <string.h>
#include <stdlib.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include "wasapi.h"


#define DEBUG_MODULE "wasapi_play"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


enum {
	POLL_INTERVAL_MS = 5,
	BUFFER_PTIMES = 4,          /**< shared mode buffer size, in ptime units */
	MIN_BUFFER_MS = 60,
	TARGET_PTIMES = 2,          /**< keep that much audio queued in device buffer */
	REOPEN_INTERVAL_US = 1000000,
	START_TIMEOUT_MS = 5000
};


struct auplay_st {
	struct auplay *ap;      /* inheritance */
	auplay_write_h *wh;
	void *arg;
	struct auplay_prm prm;
	char device[300];
	HANDLE thread;
	HANDLE ready_event;
	volatile bool run;
	volatile int init_err;
};


/** WASAPI objects of one opened device; reopened if device fails */
struct play_stream {
	IMMDeviceEnumerator *enumerator;
	IMMDevice *dev;
	IAudioClient *client;
	IAudioRenderClient *render;
	struct wasapi_fmt want;     /**< what baresip delivers */
	struct wasapi_fmt dev_fmt;
	bool converted;
	struct wasapi_conv *conv;
	UINT32 buffer_frames;       /**< device buffer size */
	UINT32 target_frames;       /**< queued frames to maintain */
	uint8_t *pcm;               /**< one baresip frame */
	uint8_t *staging;           /**< device format, waiting for room in device buffer */
	size_t staging_frames;
};


static void stream_close(struct play_stream *s)
{
	if (s->client)
		s->client->Stop();
	if (s->render)
		s->render->Release();
	if (s->client)
		s->client->Release();
	if (s->dev)
		s->dev->Release();
	if (s->enumerator)
		s->enumerator->Release();

	s->render = NULL;
	s->client = NULL;
	s->dev = NULL;
	s->enumerator = NULL;
	s->conv = (struct wasapi_conv *)mem_deref(s->conv);

	free(s->pcm);
	free(s->staging);
	s->pcm = NULL;
	s->staging = NULL;
	s->staging_frames = 0;
}


static int stream_open(struct auplay_st *st, struct play_stream *s)
{
	const uint32_t frame_frames = st->prm.frame_size / st->prm.ch;
	uint32_t ptime_ms = frame_frames * 1000 / st->prm.srate;
	uint32_t buffer_ms;
	size_t chunk_frames;
	bool loopback;
	HRESULT hr;
	int err;

	if (ptime_ms == 0)
		ptime_ms = 1;
	buffer_ms = BUFFER_PTIMES * ptime_ms;
	if (buffer_ms < MIN_BUFFER_MS)
		buffer_ms = MIN_BUFFER_MS;

	wasapi_fmt_s16(&s->want, st->prm.srate, st->prm.ch);

	hr = CoCreateInstance(WASAPI_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
			      WASAPI_IID_IMMDeviceEnumerator, (void**)&s->enumerator);
	if (FAILED(hr)) {
		DEBUG_WARNING("CoCreateInstance(MMDeviceEnumerator) failed: 0x%08x\n", hr);
		goto fail;
	}

	hr = wasapi_open_device(s->enumerator, st->device, false, &s->dev, &loopback);
	if (FAILED(hr)) {
		DEBUG_WARNING("no device for [%s]: 0x%08x\n", st->device, hr);
		goto fail;
	}

	hr = wasapi_client_init(s->dev, 0, (REFERENCE_TIME)buffer_ms * WASAPI_REF_PER_MS,
				&s->want, &s->client, &s->dev_fmt, &s->converted);
	if (FAILED(hr)) {
		DEBUG_WARNING("IAudioClient initialization failed: 0x%08x\n", hr);
		goto fail;
	}

	hr = s->client->GetService(WASAPI_IID_IAudioRenderClient, (void**)&s->render);
	if (FAILED(hr)) {
		DEBUG_WARNING("GetService(IAudioRenderClient) failed: 0x%08x\n", hr);
		goto fail;
	}

	hr = s->client->GetBufferSize(&s->buffer_frames);
	if (FAILED(hr)) {
		DEBUG_WARNING("GetBufferSize failed: 0x%08x\n", hr);
		goto fail;
	}

	s->target_frames = TARGET_PTIMES * ptime_ms * s->dev_fmt.srate / 1000;
	if (s->target_frames > s->buffer_frames)
		s->target_frames = s->buffer_frames;

	if (s->converted) {
		err = wasapi_conv_alloc(&s->conv, &s->want, &s->dev_fmt);
		if (err)
			goto fail;
		chunk_frames = wasapi_conv_max_out(s->conv, frame_frames);
	}
	else {
		chunk_frames = frame_frames;
	}

	s->pcm = (uint8_t *)malloc(frame_frames * s->want.block_align);
	s->staging = (uint8_t *)malloc((s->target_frames + chunk_frames) * s->dev_fmt.block_align);
	s->staging_frames = 0;
	if (!s->pcm || !s->staging)
		goto fail;

	hr = s->client->Start();
	if (FAILED(hr)) {
		DEBUG_WARNING("Start failed: 0x%08x\n", hr);
		goto fail;
	}

	DEBUG_NOTICE("playback [%s] started, device format: %u Hz, %u ch, %u bits%s%s,"
		     " buffer %u frames, target %u frames\n",
		     st->device, s->dev_fmt.srate, s->dev_fmt.ch, s->dev_fmt.bits,
		     s->dev_fmt.is_float ? " float" : "",
		     s->converted ? " (converted by module)" : "",
		     s->buffer_frames, s->target_frames);

	return 0;

 fail:
	stream_close(s);
	return E_AUDIO_OUTPUT_DEV_OPEN_ERROR;
}


/** \brief Top up device buffer to target level
    \return 0 or error if stream should be reopened
*/
static int stream_poll(struct auplay_st *st, struct play_stream *s)
{
	const uint32_t frame_frames = st->prm.frame_size / st->prm.ch;
	const unsigned ba = s->dev_fmt.block_align;
	UINT32 padding = 0;
	UINT32 need;
	BYTE *data = NULL;
	HRESULT hr;

	hr = s->client->GetCurrentPadding(&padding);
	if (FAILED(hr)) {
		DEBUG_WARNING("GetCurrentPadding failed: 0x%08x\n", hr);
		return EIO;
	}

	if (padding >= s->target_frames)
		return 0;

	need = s->target_frames - padding;

	/* pull whole baresip frames until there is enough to write */
	while (s->staging_frames < need) {
		uint8_t *dst = s->staging + s->staging_frames * ba;

		st->wh(s->pcm, frame_frames * s->want.block_align, st->arg);

		if (s->converted) {
			s->staging_frames += wasapi_conv_process(s->conv, s->pcm, frame_frames, dst);
		}
		else {
			memcpy(dst, s->pcm, frame_frames * ba);
			s->staging_frames += frame_frames;
		}
	}

	hr = s->render->GetBuffer(need, &data);
	if (FAILED(hr)) {
		DEBUG_WARNING("GetBuffer failed: 0x%08x\n", hr);
		return EIO;
	}

	memcpy(data, s->staging, need * ba);

	hr = s->render->ReleaseBuffer(need, 0);
	if (FAILED(hr)) {
		DEBUG_WARNING("ReleaseBuffer failed: 0x%08x\n", hr);
		return EIO;
	}

	s->staging_frames -= need;
	memmove(s->staging, s->staging + need * ba, s->staging_frames * ba);

	return 0;
}


static DWORD WINAPI play_thread(LPVOID arg)
{
	struct auplay_st *st = (struct auplay_st *)arg;
	struct play_stream s;
	uint64_t last_retry;
	HRESULT com_hr;

	memset(&s, 0, sizeof(s));

	com_hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	timeBeginPeriod(1);

	if (FAILED(com_hr))
		st->init_err = ENOMEM;
	else
		st->init_err = stream_open(st, &s);

	SetEvent(st->ready_event);

	if (st->init_err)
		goto out;

	last_retry = wasapi_now_us();

	while (st->run) {
		Sleep(POLL_INTERVAL_MS);

		if (s.client == NULL) {
			uint64_t now = wasapi_now_us();
			if (now - last_retry >= REOPEN_INTERVAL_US) {
				last_retry = now;
				if (stream_open(st, &s) == 0)
					DEBUG_NOTICE("device reopened\n");
			}
		}
		else if (stream_poll(st, &s) != 0) {
			/* e.g. device unplugged: AUDCLNT_E_DEVICE_INVALIDATED */
			DEBUG_WARNING("playback failed, closing device and retrying\n");
			stream_close(&s);
			last_retry = wasapi_now_us();
		}
	}

 out:
	stream_close(&s);

	timeEndPeriod(1);
	if (SUCCEEDED(com_hr))
		CoUninitialize();

	return 0;
}


static void auplay_destructor(void *arg)
{
	struct auplay_st *st = (struct auplay_st *)arg;

	if (st->thread) {
		st->run = false;
		WaitForSingleObject(st->thread, INFINITE);
		CloseHandle(st->thread);
	}

	if (st->ready_event)
		CloseHandle(st->ready_event);

	mem_deref(st->ap);
}


int wasapi_play_alloc(struct auplay_st **stp, struct auplay *ap,
		      struct auplay_prm *prm, const char *device,
		      auplay_write_h *wh, void *arg)
{
	struct auplay_st *st;
	int err = 0;

	if (!stp || !ap || !prm || !wh || prm->ch == 0 || prm->ch > WASAPI_MAX_CH ||
	    prm->srate == 0 || prm->frame_size < (uint32_t)prm->ch)
		return EINVAL;

	prm->fmt = AUFMT_S16LE;

	st = (struct auplay_st *)mem_zalloc(sizeof(*st), auplay_destructor);
	if (!st)
		return ENOMEM;

	st->ap  = (struct auplay *)mem_ref(ap);
	st->wh  = wh;
	st->arg = arg;
	st->prm = *prm;
	str_ncpy(st->device, device ? device : "", sizeof(st->device));

	st->ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!st->ready_event) {
		err = ENOMEM;
		goto out;
	}

	st->run = true;
	st->thread = CreateThread(NULL, 0, play_thread, st, 0, NULL);
	if (!st->thread) {
		st->run = false;
		err = ENOMEM;
		goto out;
	}
	SetThreadPriority(st->thread, THREAD_PRIORITY_HIGHEST);

	if (WaitForSingleObject(st->ready_event, START_TIMEOUT_MS) != WAIT_OBJECT_0) {
		DEBUG_WARNING("timeout starting [%s]\n", st->device);
		err = E_AUDIO_OUTPUT_DEV_OPEN_ERROR;
		goto out;
	}

	err = st->init_err;

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}
