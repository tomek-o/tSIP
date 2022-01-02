/**
 * @file portaudio.c  Portaudio sound driver
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <string.h>
#include <portaudio.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>


#define DEBUG_MODULE "portaudio"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


/*
 * portaudio v19 is required
 */

struct ausrc_st {
	struct ausrc *as;      /* inheritance */
	PaStream *stream_rd;
	ausrc_read_h *rh;
	void *arg;
	volatile bool ready;
};

struct auplay_st {
	struct auplay *ap;      /* inheritance */
	PaStream *stream_wr;
	auplay_write_h *wh;
	void *arg;
	volatile bool ready;
};


static struct ausrc *ausrc;
static struct auplay *auplay;


/**
 * This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
 */
static int read_callback(const void *inputBuffer, void *outputBuffer,
			 unsigned long frameCount,
			 const PaStreamCallbackTimeInfo *timeInfo,
			 PaStreamCallbackFlags statusFlags, void *userData)
{
	struct ausrc_st *st = userData;

	(void)outputBuffer;
	(void)timeInfo;
	(void)statusFlags;

	if (st->ready)
		st->rh(inputBuffer, 2*frameCount, st->arg);

	return paContinue;
}


static int write_callback(const void *inputBuffer, void *outputBuffer,
			  unsigned long frameCount,
			  const PaStreamCallbackTimeInfo *timeInfo,
			  PaStreamCallbackFlags statusFlags, void *userData)
{
	struct auplay_st *st = userData;

	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;

	if (st->ready)
		st->wh(outputBuffer, 2*frameCount, st->arg);

	return paContinue;
}


static int read_stream_open(struct ausrc_st *st, const struct ausrc_prm *prm,
			    uint32_t dev)
{
	PaStreamParameters prm_in;
	PaError err;
	const PaStreamInfo* info;

	memset(&prm_in, 0, sizeof(prm_in));
	prm_in.device           = dev;
	prm_in.channelCount     = prm->ch;
	prm_in.sampleFormat     = paInt16;
	prm_in.suggestedLatency = conf_config()->audio.portaudioInSuggestedLatency;

	st->stream_rd = NULL;
	err = Pa_OpenStream(&st->stream_rd, &prm_in, NULL, prm->srate,
			    prm->frame_size, paNoFlag, read_callback, st);
	if (paNoError != err) {
		DEBUG_WARNING("read: Pa_OpenStream: %s\n",
			      Pa_GetErrorText(err));
		return E_AUDIO_SOURCE_DEV_OPEN_ERROR;
	}

	err = Pa_StartStream(st->stream_rd);
	if (paNoError != err) {
		DEBUG_WARNING("read: Pa_StartStream: %s\n",
			      Pa_GetErrorText(err));
		return E_AUDIO_SOURCE_DEV_OPEN_ERROR;
	}

	info = Pa_GetStreamInfo(st->stream_rd);
	if (info) {
		DEBUG_NOTICE("read stream latency = %d ms\n", (int)(info->inputLatency*1000));
	}

	return 0;
}


static int write_stream_open(struct auplay_st *st,
			     const struct auplay_prm *prm, uint32_t dev)
{
	PaStreamParameters prm_out;
	PaError err;
	const PaStreamInfo* info;

	memset(&prm_out, 0, sizeof(prm_out));
	prm_out.device           = dev;
	prm_out.channelCount     = prm->ch;
	prm_out.sampleFormat     = paInt16;
	prm_out.suggestedLatency = conf_config()->audio.portaudioOutSuggestedLatency;

	st->stream_wr = NULL;
	err = Pa_OpenStream(&st->stream_wr, NULL, &prm_out, prm->srate,
			    prm->frame_size, paNoFlag, write_callback, st);
	if (paNoError != err) {
		DEBUG_WARNING("write: Pa_OpenStream: %s\n",
			      Pa_GetErrorText(err));
		return E_AUDIO_OUTPUT_DEV_OPEN_ERROR;
	}

	err = Pa_StartStream(st->stream_wr);
	if (paNoError != err) {
		DEBUG_WARNING("write: Pa_StartStream: %s\n",
			      Pa_GetErrorText(err));
		return E_AUDIO_OUTPUT_DEV_OPEN_ERROR;
	}

	info = Pa_GetStreamInfo(st->stream_wr);
	if (info) {
		DEBUG_NOTICE("write stream latency = %d ms\n", (int)(info->outputLatency*1000));
	}

	return 0;
}


static void ausrc_destructor(void *arg)
{
	struct ausrc_st *st = arg;

	st->ready = false;

	if (st->stream_rd) {
		Pa_AbortStream(st->stream_rd);
		Pa_CloseStream(st->stream_rd);
	}

	mem_deref(st->as);
}


static void auplay_destructor(void *arg)
{
	struct auplay_st *st = arg;

	st->ready = false;

	if (st->stream_wr) {
		Pa_AbortStream(st->stream_wr);
		Pa_CloseStream(st->stream_wr);
	}

	mem_deref(st->ap);
}

static PaDeviceIndex find_dev(const char* name, bool out) {
	int i;
	int n = Pa_GetDeviceCount();

	for (i=0; i<n; i++)
	{
		const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
		if (out == false) {
			if (info->maxInputChannels > 0)
			{
				if (!strcmp(info->name, name))
					return i;
			}
		} else {
			if (info->maxOutputChannels > 0)
			{
				if (!strcmp(info->name, name))
					return i;
			}
		}
	}
	if (out)
		return Pa_GetDefaultOutputDevice();
	else
		return Pa_GetDefaultInputDevice();
}

static int src_alloc(struct ausrc_st **stp, struct ausrc *as,
		     struct media_ctx **ctx,
		     struct ausrc_prm *prm, const char *device,
		     ausrc_read_h *rh, ausrc_error_h *errh, void *arg)
{
	struct ausrc_st *st;
	int err;

	(void)ctx;
	(void)device;
	(void)errh;

	prm->fmt = AUFMT_S16LE;

	st = mem_zalloc(sizeof(*st), ausrc_destructor);
	if (!st)
		return ENOMEM;

	st->as  = mem_ref(as);
	st->rh  = rh;
	st->arg = arg;

	err = read_stream_open(st, prm, find_dev(device, false));
	if (err)
		goto out;

	st->ready = true;

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}


static int play_alloc(struct auplay_st **stp, struct auplay *ap,
		      struct auplay_prm *prm, const char *device,
		      auplay_write_h *wh, void *arg)
{
	struct auplay_st *st;
	int err;

	(void)device;

	prm->fmt = AUFMT_S16LE;

	st = mem_zalloc(sizeof(*st), auplay_destructor);
	if (!st)
		return ENOMEM;

	st->ap  = mem_ref(ap);
	st->wh  = wh;
	st->arg = arg;

	err = write_stream_open(st, prm, find_dev(device, true));
	if (err)
		goto out;

	st->ready = true;

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}


static int pa_init(void)
{
	PaError error;
	int i, n, err = 0;

	error = Pa_Initialize();
	if (paNoError != error) {
		DEBUG_WARNING("PortAudio init: %s\n", Pa_GetErrorText(error));
		return ENODEV;
	}

	n = Pa_GetDeviceCount();

	DEBUG_NOTICE("Portaudio driver: Device count %d\n", n);

	for (i=0; i<n; i++) {
		const PaDeviceInfo *info;

		info = Pa_GetDeviceInfo(i);

		DEBUG_NOTICE(" device %d: %s (channels in: %d, out: %d)\n",
			i, info->name, info->maxInputChannels, info->maxOutputChannels);
		(void)info;
	}

	if (paNoDevice != Pa_GetDefaultInputDevice())
		err |= ausrc_register(&ausrc, "portaudio", src_alloc);

	if (paNoDevice != Pa_GetDefaultOutputDevice())
		err |= auplay_register(&auplay, "portaudio", play_alloc);

	return err;
}


static int pa_close(void)
{
	ausrc  = mem_deref(ausrc);
	auplay = mem_deref(auplay);

	Pa_Terminate();

	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(portaudio) = {
	"portaudio",
	"sound",
	pa_init,
	pa_close
};
