/**
 * @file avformat/audio.c  libavformat media-source -- audio
 *
 * Copyright (C) 2010 - 2020 Alfred E. Heggestad
 */

#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include "mod_avformat.h"

#define DEBUG_MODULE "avformat"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

struct ausrc_st {
	struct ausrc *as;  /* base class */
	struct shared *shared;
	struct ausrc_prm prm;
	SwrContext *swr;
	ausrc_read_h *readh;
	ausrc_error_h *errh;
	void *arg;
};


static void audio_destructor(void *arg)
{
	struct ausrc_st *st = arg;

	avformat_shared_set_audio(st->shared, NULL);
	mem_deref(st->shared);

	if (st->swr)
		swr_free(&st->swr);

	mem_deref(st->as);		
}


static enum AVSampleFormat aufmt_to_avsampleformat(enum aufmt fmt)
{
	switch (fmt) {

	case AUFMT_S16LE: return AV_SAMPLE_FMT_S16;
	//case AUFMT_FLOAT: return AV_SAMPLE_FMT_FLT;
	default:          return AV_SAMPLE_FMT_NONE;
	}
}


int avformat_audio_alloc(struct ausrc_st **stp, struct ausrc *as,
			 struct media_ctx **ctx,
			 struct ausrc_prm *prm, const char *device,
			 ausrc_read_h *readh, ausrc_error_h *errh, void *arg)
{
	struct ausrc_st *st;
	struct shared *sh;
	int err = 0;
	int channels;

	if (!stp || !as || !prm || !readh)
		return EINVAL;

	DEBUG_INFO("avformat: audio: loading input file '%s'\n", device);

	st = mem_zalloc(sizeof(*st), audio_destructor);
	if (!st)
		return ENOMEM;

	st->as   = mem_ref(as);		
	st->readh = readh;
	st->errh  = errh;
	st->arg   = arg;
	st->prm   = *prm;

	sh = avformat_shared_lookup(device);
	if (sh) {
		st->shared = mem_ref(sh);
	}
	else {
		err = avformat_shared_alloc(&st->shared, device,
					    0.0, NULL, false);
		if (err)
			goto out;
	}

	sh = st->shared;

	if (st->shared->au.idx < 0 || !st->shared->au.ctx) {
		DEBUG_INFO("avformat: audio: media file has no audio stream\n");
		err = ENOENT;
		goto out;
	}

	st->swr = swr_alloc();
	if (!st->swr) {
		err = ENOMEM;
		goto out;
	}

	avformat_shared_set_audio(st->shared, st);

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 37, 100)
	channels = sh->au.ctx->ch_layout.nb_channels;
#else
	channels = sh->au.ctx->channels;
#endif

	DEBUG_INFO("avformat: audio: converting %u/%u %s -> %u/%u %s\n",
	     sh->au.ctx->sample_rate, channels,
	     av_get_sample_fmt_name(sh->au.ctx->sample_fmt),
	     prm->srate, prm->ch, aufmt_name(prm->fmt));

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}


void avformat_audio_decode(struct shared *st, AVPacket *pkt)
{
	AVFrame frame;
	AVFrame frame2;
	int ret;

	if (!st || !st->au.ctx)
		return;

	memset(&frame, 0, sizeof(frame));
	memset(&frame2, 0, sizeof(frame2));

	ret = avcodec_send_packet(st->au.ctx, pkt);
	if (ret < 0)
		return;

	ret = avcodec_receive_frame(st->au.ctx, &frame);
	if (ret < 0)
		return;

	/* NOTE: pass timestamp to application */

	mtx_lock(&st->lock);

	if (st->ausrc_st && st->ausrc_st->readh) {

		const AVRational tb = st->au.time_base;
		int channels = st->ausrc_st->prm.ch;

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 37, 100)
		av_channel_layout_default(&frame2.ch_layout, channels);
#else
		frame.channel_layout =
			av_get_default_channel_layout(frame.channels);

		frame2.channels       = channels;
		frame2.channel_layout =
			av_get_default_channel_layout(st->ausrc_st->prm.ch);
#endif
		frame2.sample_rate    = st->ausrc_st->prm.srate;
		frame2.format         =
			aufmt_to_avsampleformat(st->ausrc_st->prm.fmt);

		ret = swr_convert_frame(st->ausrc_st->swr, &frame2, &frame);
		if (ret) {
			DEBUG_WARNING("avformat: swr_convert_frame failed (%d)\n",
				ret);
			goto unlock;
		}

	#if 0
		auframe_init(&af, st->ausrc_st->prm.fmt, frame2.data[0],
				 frame2.nb_samples * channels,
				 st->ausrc_st->prm.srate, st->ausrc_st->prm.ch);
		af.timestamp = frame.pts * AUDIO_TIMEBASE * tb.num / tb.den;

		st->ausrc_st->readh(&af, st->ausrc_st->arg);
	#else
		st->ausrc_st->readh(frame2.data[0], frame2.nb_samples * channels * sizeof(int16_t), st->ausrc_st->arg);
	#endif
	}

 unlock:
	mtx_unlock(&st->lock);

	av_frame_unref(&frame2);
	av_frame_unref(&frame);
}
