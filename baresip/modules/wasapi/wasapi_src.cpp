/**
 * @file wasapi_src.cpp WASAPI sound driver -- source (capture and loopback)
 */
#include <string.h>
#include <stdlib.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include "wasapi.h"


#define DEBUG_MODULE "wasapi_src"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


enum {
	POLL_INTERVAL_MS = 5,
	BUFFER_MS = 100,             /**< shared mode capture buffer */
	SILENCE_GAP_US = 60000,      /**< no data for that long -> generate silence */
	REOPEN_INTERVAL_US = 1000000,
	START_TIMEOUT_MS = 5000
};


struct ausrc_st {
	struct ausrc *as;      /* inheritance */
	ausrc_read_h *rh;
	void *arg;
	struct ausrc_prm prm;
	char device[300];
	HANDLE thread;
	HANDLE ready_event;
	volatile bool run;
	volatile int init_err;
};


/** WASAPI objects of one opened device; reopened if device fails */
struct src_stream {
	IMMDeviceEnumerator *enumerator;
	IMMDevice *dev;
	IAudioClient *client;
	IAudioCaptureClient *capture;
	struct wasapi_fmt want;     /**< what baresip gets */
	struct wasapi_fmt dev_fmt;
	bool converted;
	bool loopback;
	struct wasapi_conv *conv;
	uint8_t *outbuf;
	size_t outbuf_sz;
	bool got_data;
};


static void stream_close(struct src_stream *s)
{
	if (s->client)
		s->client->Stop();
	if (s->capture)
		s->capture->Release();
	if (s->client)
		s->client->Release();
	if (s->dev)
		s->dev->Release();
	if (s->enumerator)
		s->enumerator->Release();

	s->capture = NULL;
	s->client = NULL;
	s->dev = NULL;
	s->enumerator = NULL;
	s->conv = (struct wasapi_conv *)mem_deref(s->conv);
}


static int stream_open(struct ausrc_st *st, struct src_stream *s)
{
	HRESULT hr;
	int err;

	wasapi_fmt_s16(&s->want, st->prm.srate, st->prm.ch);

	hr = CoCreateInstance(WASAPI_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL,
			      WASAPI_IID_IMMDeviceEnumerator, (void**)&s->enumerator);
	if (FAILED(hr)) {
		DEBUG_WARNING("CoCreateInstance(MMDeviceEnumerator) failed: 0x%08x\n", hr);
		goto fail;
	}

	hr = wasapi_open_device(s->enumerator, st->device, true, &s->dev, &s->loopback);
	if (FAILED(hr)) {
		DEBUG_WARNING("no device for [%s]: 0x%08x\n", st->device, hr);
		goto fail;
	}

	hr = wasapi_client_init(s->dev, s->loopback ? AUDCLNT_STREAMFLAGS_LOOPBACK : 0,
				(REFERENCE_TIME)BUFFER_MS * WASAPI_REF_PER_MS,
				&s->want, &s->client, &s->dev_fmt, &s->converted);
	if (FAILED(hr)) {
		DEBUG_WARNING("IAudioClient initialization failed: 0x%08x\n", hr);
		goto fail;
	}

	hr = s->client->GetService(WASAPI_IID_IAudioCaptureClient, (void**)&s->capture);
	if (FAILED(hr)) {
		DEBUG_WARNING("GetService(IAudioCaptureClient) failed: 0x%08x\n", hr);
		goto fail;
	}

	if (s->converted) {
		err = wasapi_conv_alloc(&s->conv, &s->dev_fmt, &s->want);
		if (err)
			goto fail;
	}

	hr = s->client->Start();
	if (FAILED(hr)) {
		DEBUG_WARNING("Start failed: 0x%08x\n", hr);
		goto fail;
	}

	DEBUG_NOTICE("%s [%s] started, device format: %u Hz, %u ch, %u bits%s%s\n",
		     s->loopback ? "loopback" : "capture", st->device,
		     s->dev_fmt.srate, s->dev_fmt.ch, s->dev_fmt.bits,
		     s->dev_fmt.is_float ? " float" : "",
		     s->converted ? " (converted by module)" : "");

	return 0;

 fail:
	stream_close(s);
	return E_AUDIO_SOURCE_DEV_OPEN_ERROR;
}


static bool ensure_outbuf(struct src_stream *s, size_t sz)
{
	uint8_t *p;

	if (s->outbuf_sz >= sz)
		return true;

	p = (uint8_t *)realloc(s->outbuf, sz);
	if (!p)
		return false;

	s->outbuf = p;
	s->outbuf_sz = sz;

	return true;
}


/** \brief Read all pending packets and pass them to baresip
    \return 0 or error if stream should be reopened
*/
static int stream_poll(struct ausrc_st *st, struct src_stream *s)
{
	for (;;) {
		UINT32 packet = 0;
		UINT32 frames = 0;
		DWORD flags = 0;
		BYTE *data = NULL;
		HRESULT hr;

		hr = s->capture->GetNextPacketSize(&packet);
		if (FAILED(hr)) {
			DEBUG_WARNING("GetNextPacketSize failed: 0x%08x\n", hr);
			return EIO;
		}
		if (packet == 0)
			return 0;

		hr = s->capture->GetBuffer(&data, &frames, &flags, NULL, NULL);
		if (FAILED(hr)) {
			DEBUG_WARNING("GetBuffer failed: 0x%08x\n", hr);
			return EIO;
		}

		if (frames > 0) {
			const BYTE *in = (flags & AUDCLNT_BUFFERFLAGS_SILENT) ? NULL : data;
			size_t n;

			/* always copied: baresip may modify the buffer (mute) */
			if (s->converted) {
				n = wasapi_conv_max_out(s->conv, frames);
				if (ensure_outbuf(s, n * s->want.block_align))
					n = wasapi_conv_process(s->conv, in, frames, s->outbuf);
				else
					n = 0;
			}
			else {
				n = frames;
				if (!ensure_outbuf(s, n * s->want.block_align))
					n = 0;
				else if (in)
					memcpy(s->outbuf, in, n * s->want.block_align);
				else
					memset(s->outbuf, 0, n * s->want.block_align);
			}

			if (n > 0 && st->rh)
				st->rh(s->outbuf, n * s->want.block_align, st->arg);

			s->got_data = true;
		}

		hr = s->capture->ReleaseBuffer(frames);
		if (FAILED(hr)) {
			DEBUG_WARNING("ReleaseBuffer failed: 0x%08x\n", hr);
			return EIO;
		}
	}
}


static DWORD WINAPI src_thread(LPVOID arg)
{
	struct ausrc_st *st = (struct ausrc_st *)arg;
	struct src_stream s;
	const size_t frame_bytes = 2 * st->prm.frame_size;
	const uint64_t ptime_us = (uint64_t)(st->prm.frame_size / st->prm.ch) * 1000000 / st->prm.srate;
	uint8_t *silence;
	uint64_t now, last_data, silence_clock, last_retry;
	HRESULT com_hr;

	memset(&s, 0, sizeof(s));

	com_hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	timeBeginPeriod(1);

	silence = (uint8_t *)calloc(1, frame_bytes);

	if (FAILED(com_hr) || silence == NULL)
		st->init_err = ENOMEM;
	else
		st->init_err = stream_open(st, &s);

	SetEvent(st->ready_event);

	if (st->init_err)
		goto out;

	now = wasapi_now_us();
	last_data = now;
	silence_clock = now;
	last_retry = now;

	while (st->run) {
		Sleep(POLL_INTERVAL_MS);
		now = wasapi_now_us();

		if (s.client == NULL) {
			if (now - last_retry >= REOPEN_INTERVAL_US) {
				last_retry = now;
				if (stream_open(st, &s) == 0)
					DEBUG_NOTICE("device reopened\n");
			}
		}
		else if (stream_poll(st, &s) != 0) {
			/* e.g. device unplugged: AUDCLNT_E_DEVICE_INVALIDATED */
			DEBUG_WARNING("capture failed, closing device and retrying\n");
			stream_close(&s);
			last_retry = now;
		}

		if (s.got_data) {
			s.got_data = false;
			last_data = now;
			silence_clock = now;
		}
		else if (now - last_data >= SILENCE_GAP_US) {
			/* loopback delivers no packets at all while nothing
			   is playing (and device may be gone): keep the
			   audio clock running with silence */
			while (now - silence_clock >= ptime_us) {
				memset(silence, 0, frame_bytes);
				if (st->rh)
					st->rh(silence, frame_bytes, st->arg);
				silence_clock += ptime_us;
			}
		}
	}

 out:
	stream_close(&s);
	free(s.outbuf);
	free(silence);

	timeEndPeriod(1);
	if (SUCCEEDED(com_hr))
		CoUninitialize();

	return 0;
}


static void ausrc_destructor(void *arg)
{
	struct ausrc_st *st = (struct ausrc_st *)arg;

	if (st->thread) {
		st->run = false;
		WaitForSingleObject(st->thread, INFINITE);
		CloseHandle(st->thread);
	}

	if (st->ready_event)
		CloseHandle(st->ready_event);

	mem_deref(st->as);
}


int wasapi_src_alloc(struct ausrc_st **stp, struct ausrc *as,
		     struct media_ctx **ctx,
		     struct ausrc_prm *prm, const char *device,
		     ausrc_read_h *rh, ausrc_error_h *errh, void *arg)
{
	struct ausrc_st *st;
	int err = 0;

	(void)ctx;
	(void)errh;

	if (!stp || !as || !prm || prm->ch == 0 || prm->ch > WASAPI_MAX_CH ||
	    prm->srate == 0 || prm->frame_size < (uint32_t)prm->ch)
		return EINVAL;

	prm->fmt = AUFMT_S16LE;

	st = (struct ausrc_st *)mem_zalloc(sizeof(*st), ausrc_destructor);
	if (!st)
		return ENOMEM;

	st->as  = (struct ausrc *)mem_ref(as);
	st->rh  = rh;
	st->arg = arg;
	st->prm = *prm;
	str_ncpy(st->device, device ? device : "", sizeof(st->device));

	st->ready_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!st->ready_event) {
		err = ENOMEM;
		goto out;
	}

	st->run = true;
	st->thread = CreateThread(NULL, 0, src_thread, st, 0, NULL);
	if (!st->thread) {
		st->run = false;
		err = ENOMEM;
		goto out;
	}
	SetThreadPriority(st->thread, THREAD_PRIORITY_HIGHEST);

	if (WaitForSingleObject(st->ready_event, START_TIMEOUT_MS) != WAIT_OBJECT_0) {
		DEBUG_WARNING("timeout starting [%s]\n", st->device);
		err = E_AUDIO_SOURCE_DEV_OPEN_ERROR;
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
