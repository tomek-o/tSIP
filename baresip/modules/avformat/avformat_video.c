/**
 * @file avformat/video.c  libavformat media-source -- video
 *
 * Copyright (C) 2010 - 2020 Alfred E. Heggestad
 * Copyright (C) 2021 by:
 *     Media Magic Technologies <developer@mediamagictechnologies.com>
 *     and Divus GmbH <developer@divus.eu>
 */

#include <re.h>
#include <rem.h>
#include <baresip.h>
#ifdef __BORLANDC__
#pragma warn -8056	// disable "Integer arithmetic overflow" warning
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#ifdef __BORLANDC__
#pragma warn .8056
#endif
#include "mod_avformat.h"

#define DEBUG_MODULE "avformat"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

struct vidsrc_st {
	struct vidsrc *vs;  /* inheritance */
	struct shared *shared;
	vidsrc_frame_h *frameh;
	void *arg;
};


static void video_destructor(void *arg)
{
	struct vidsrc_st *st = arg;

	avformat_shared_set_video(st->shared, NULL);
	mem_deref(st->shared);
	mem_deref(st->vs);	
}


static enum vidfmt avpixfmt_to_vidfmt(enum AVPixelFormat pix_fmt)
{
	switch (pix_fmt) {

	case AV_PIX_FMT_YUV420P:  return VID_FMT_YUV420P;
	case AV_PIX_FMT_YUVJ420P: return VID_FMT_YUV420P;
	case AV_PIX_FMT_YUV444P:  return VID_FMT_YUV444P;
	case AV_PIX_FMT_NV12:     return VID_FMT_NV12;
	case AV_PIX_FMT_NV21:     return VID_FMT_NV21;
	case AV_PIX_FMT_UYVY422:  return VID_FMT_UYVY422;
	case AV_PIX_FMT_YUYV422:  return VID_FMT_YUYV422;
	case AV_PIX_FMT_RGBA:     return VID_FMT_RGB32;
	case AV_PIX_FMT_YUV422P:  return VID_FMT_YUV422P;
	case AV_PIX_FMT_YUVJ422P: return VID_FMT_YUV422P;
	default:                  return (enum vidfmt)-1;
	}
}


int  avformat_video_alloc(struct vidsrc_st **stp, struct vidsrc *vs,
		 struct media_ctx **ctx, struct vidsrc_prm *prm,
		 const struct vidsz *size,
		 const char *fmt, const char *dev,
		 vidsrc_frame_h *frameh,
		 vidsrc_error_h *errorh, void *arg)
{
	struct vidsrc_st *st;
	struct shared *sh;
	int err = 0;

	(void)fmt;
	(void)errorh;

	if (!stp || !vs || !prm || !size || !frameh)
		return EINVAL;

	DEBUG_INFO("avformat: video: alloc dev='%s'\n", dev);

	st = mem_zalloc(sizeof(*st), video_destructor);
	if (!st)
		return ENOMEM;

	st->vs = (struct vidsrc *)mem_ref(vs);		
	st->frameh = frameh;
	st->arg    = arg;

	sh = avformat_shared_lookup(dev);
	if (sh) {
		st->shared = mem_ref(sh);
	}
	else {
		err = avformat_shared_alloc(&st->shared, dev,
					    prm->fps, size, true);
		if (err)
			goto out;
	}

	if (st->shared->vid.idx < 0 || !st->shared->vid.ctx) {
		DEBUG_INFO("avformat: video: media file has no video stream\n");
		err = ENOENT;
		goto out;
	}

	avformat_shared_set_video(st->shared, st);

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}

#if 0
void avformat_video_copy(struct shared *st, AVPacket *pkt)
{
	struct vidpacket vp;
	AVRational tb;

	if (!st || !pkt)
		return;

	tb = st->vid.time_base;

	vp.buf = pkt->data;
	vp.size = pkt->size;
	vp.timestamp = pkt->pts * VIDEO_TIMEBASE * tb.num / tb.den;
	vp.keyframe = !!(pkt->flags & AV_PKT_FLAG_KEY);

	mtx_lock(&st->lock);

	if (st->vidsrc_st && st->vidsrc_st->packeth) {
		st->vidsrc_st->packeth(&vp, st->vidsrc_st->arg);
	}

	mtx_unlock(&st->lock);
}
#endif


void avformat_video_decode(struct shared *st, AVPacket *pkt)
{
	AVRational tb;
	struct vidframe vf;
	AVFrame *frame = 0;
	/* uint64_t timestamp; */
	unsigned i;
	int ret;

	if (!st || !st->vid.ctx)
		return;

	tb = st->vid.time_base;

	frame = av_frame_alloc();
	if (!frame)
		return;

	ret = avcodec_send_packet(st->vid.ctx, pkt);
	if (ret < 0)
		goto out;

	ret = avcodec_receive_frame(st->vid.ctx, frame);
	if (ret < 0)
		goto out;

	if (st->vid.ctx->hw_device_ctx) {
		AVFrame *frame2;

		frame2 = av_frame_alloc();
		if (!frame2)
			goto out;

		ret = av_hwframe_transfer_data(frame2, frame, 0);
		if (ret < 0) {
			av_frame_free(&frame2);
			goto out;
		}

		ret = av_frame_copy_props(frame2, frame);
		if (ret < 0) {
			av_frame_free(&frame2);
			goto out;
		}

		av_frame_unref(frame);
		av_frame_move_ref(frame, frame2);
		av_frame_free(&frame2);
	}

	vf.fmt = avpixfmt_to_vidfmt(frame->format);
	if (vf.fmt == (enum vidfmt)-1) {
		DEBUG_WARNING("avformat: decode: bad pixel format"
			" (%i) (%s)\n",
			frame->format,
			av_get_pix_fmt_name(frame->format));
		goto out;
	}

	vf.size.w = st->vid.ctx->width;
	vf.size.h = st->vid.ctx->height;

	for (i=0; i<4; i++) {
		vf.data[i]     = frame->data[i];
		vf.linesize[i] = frame->linesize[i];
	}

	/* convert timestamp */
	/* timestamp = frame->pts * VIDEO_TIMEBASE * tb.num / tb.den; */

	mtx_lock(&st->lock);

	if (st->vidsrc_st && st->vidsrc_st->frameh)
		st->vidsrc_st->frameh(&vf, /* timestamp, */ st->vidsrc_st->arg);

	mtx_unlock(&st->lock);

 out:
	if (frame)
		av_frame_free(&frame);
}


/*
 * Raw H.264 access units (Annex-B, from the decoder hook) to a fragmented MP4 file.
 * Used by tSIP video recording. Timestamps are the arrival times of the access units.
 */

struct h264_mp4 {
	AVFormatContext *fmt;
	AVStream *st;
	bool header;		/**< header written, samples can be written */
	int64_t t0;		/**< arrival time of first sample [us] */
	int64_t last_pts;	/**< last sample time [ms], pts is strictly increasing */
};

/* next NAL unit of Annex-B buffer, without start code and trailing zero bytes */
static bool annexb_next(const uint8_t *buf, size_t len, size_t *pos,
			const uint8_t **nal, size_t *nlen)
{
	size_t i = *pos, s, e;

	while (i + 3 <= len && !(buf[i] == 0 && buf[i+1] == 0 && buf[i+2] == 1))
		i++;
	if (i + 3 > len)
		return false;

	s = i + 3;
	e = s;
	while (e + 3 <= len && !(buf[e] == 0 && buf[e+1] == 0 && buf[e+2] == 1))
		e++;
	if (e + 3 > len)
		e = len;
	while (e > s && buf[e-1] == 0)
		e--;

	*nal = buf + s;
	*nlen = e - s;
	*pos = e;
	return true;
}

/* MP4 sample NAL type: keep slices and SEI, parameter sets are in avcC */
static bool mp4_sample_nal(uint8_t type)
{
	return type != 7 && type != 8 && type != 9;
}

static bool h264_mp4_dims(const uint8_t *buf, size_t len, int *w, int *h)
{
	const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	AVCodecParserContext *parser = av_parser_init(AV_CODEC_ID_H264);
	AVCodecContext *ctx = codec ? avcodec_alloc_context3(codec) : NULL;
	uint8_t *out = NULL;
	int outlen = 0;
	bool ok = false;

	if (parser && ctx) {
		(void)av_parser_parse2(parser, ctx, &out, &outlen, buf, (int)len,
				       AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
		if (parser->width > 0 && parser->height > 0) {
			*w = parser->width;
			*h = parser->height;
			ok = true;
		}
	}

	if (parser)
		av_parser_close(parser);
	if (ctx)
		avcodec_free_context(&ctx);
	return ok;
}

/* write MP4 header once SPS, PPS and dimensions are known (first keyframe) */
static bool h264_mp4_start(struct h264_mp4 *w, const uint8_t *buf, size_t len)
{
	const uint8_t *nal, *sps = NULL, *pps = NULL;
	size_t nlen, pos = 0, spslen = 0, ppslen = 0, extlen;
	int width = 0, height = 0, ret;
	uint8_t *p;
	AVDictionary *opts = NULL;
	AVCodecParameters *par = w->st->codecpar;

	while (annexb_next(buf, len, &pos, &nal, &nlen)) {
		if (nlen == 0)
			continue;
		if ((nal[0] & 0x1f) == 7 && !sps) {
			sps = nal;
			spslen = nlen;
		}
		else if ((nal[0] & 0x1f) == 8 && !pps) {
			pps = nal;
			ppslen = nlen;
		}
	}
	if (!sps || !pps || spslen < 4)
		return false;
	if (!h264_mp4_dims(buf, len, &width, &height))
		return false;

	extlen = 11 + spslen + ppslen;
	par->extradata = av_mallocz(extlen + AV_INPUT_BUFFER_PADDING_SIZE);
	if (!par->extradata)
		return false;
	p = par->extradata;
	*p++ = 1;		/* configurationVersion */
	*p++ = sps[1];		/* profile_idc */
	*p++ = sps[2];		/* profile_compatibility */
	*p++ = sps[3];		/* level_idc */
	*p++ = 0xff;		/* 4-byte NAL unit lengths */
	*p++ = 0xe1;		/* one SPS */
	*p++ = (uint8_t)(spslen >> 8);
	*p++ = (uint8_t)spslen;
	memcpy(p, sps, spslen);
	p += spslen;
	*p++ = 1;		/* one PPS */
	*p++ = (uint8_t)(ppslen >> 8);
	*p++ = (uint8_t)ppslen;
	memcpy(p, pps, ppslen);
	par->extradata_size = (int)extlen;

	par->codec_type = AVMEDIA_TYPE_VIDEO;
	par->codec_id = AV_CODEC_ID_H264;
	par->width = width;
	par->height = height;

	/* fragmented MP4: playable even if recording is interrupted */
	av_dict_set(&opts, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
	ret = avformat_write_header(w->fmt, &opts);
	av_dict_free(&opts);
	if (ret < 0)
		return false;

	w->t0 = (int64_t)tmr_jiffies_usec();
	w->last_pts = -1;
	w->header = true;
	return true;
}

bool h264_mp4_supported(void)
{
	return av_guess_format("mp4", NULL, NULL) != NULL;
}

int h264_mp4_open(void **wp, const char *filename)
{
	struct h264_mp4 *w;
	int ret;

	if (!wp || !filename)
		return EINVAL;

	w = mem_zalloc(sizeof(*w), NULL);
	if (!w)
		return ENOMEM;

	ret = avformat_alloc_output_context2(&w->fmt, NULL, "mp4", filename);
	if (ret < 0 || !w->fmt)
		goto err;

	ret = avio_open(&w->fmt->pb, filename, AVIO_FLAG_WRITE);
	if (ret < 0)
		goto err;

	w->st = avformat_new_stream(w->fmt, NULL);
	if (!w->st)
		goto err;
	w->st->time_base.num = 1;
	w->st->time_base.den = 1000;

	*wp = w;
	return 0;

 err:
	{
		char errbuf[128];
		av_strerror(ret, errbuf, sizeof(errbuf));
		DEBUG_WARNING("mp4 open %s: %s (%d)\n", filename, errbuf, ret);
	}
	if (w->fmt) {
		if (w->fmt->pb)
			avio_closep(&w->fmt->pb);
		avformat_free_context(w->fmt);
	}
	mem_deref(w);
	return EIO;
}

int h264_mp4_write(void *wp, const uint8_t *buf, size_t len)
{
	struct h264_mp4 *w = wp;
	const uint8_t *nal;
	size_t nlen, pos = 0, total = 0, off = 0, n_nal = 0;
	bool key = false;
	AVPacket *pkt;
	int64_t now, pts;
	int ret;

	if (!w || !buf || !len)
		return EINVAL;

	if (!w->header && !h264_mp4_start(w, buf, len))
		return 0;	/* wait for keyframe with parameter sets */

	/* convert to length-prefixed NAL units */
	while (annexb_next(buf, len, &pos, &nal, &nlen)) {
		if (nlen == 0 || !mp4_sample_nal(nal[0] & 0x1f))
			continue;
		if ((nal[0] & 0x1f) == 5)
			key = true;
		total += 4 + nlen;
		++n_nal;
	}
	if (!n_nal)
		return 0;

	pkt = av_packet_alloc();
	if (!pkt)
		return ENOMEM;
	if (av_new_packet(pkt, (int)total) < 0) {
		av_packet_free(&pkt);
		return ENOMEM;
	}

	pos = 0;
	while (annexb_next(buf, len, &pos, &nal, &nlen)) {
		if (nlen == 0 || !mp4_sample_nal(nal[0] & 0x1f))
			continue;
		pkt->data[off]   = (uint8_t)(nlen >> 24);
		pkt->data[off+1] = (uint8_t)(nlen >> 16);
		pkt->data[off+2] = (uint8_t)(nlen >> 8);
		pkt->data[off+3] = (uint8_t)nlen;
		memcpy(pkt->data + off + 4, nal, nlen);
		off += 4 + nlen;
	}

	now = (int64_t)tmr_jiffies_usec();
	pts = (now - w->t0) / 1000;
	if (pts <= w->last_pts)
		pts = w->last_pts + 1;
	w->last_pts = pts;

	pkt->stream_index = w->st->index;
	pkt->pts = pts;
	pkt->dts = pts;
	if (key)
		pkt->flags |= AV_PKT_FLAG_KEY;

	ret = av_interleaved_write_frame(w->fmt, pkt);	/* takes ownership of data */
	av_packet_free(&pkt);
	return ret < 0 ? EIO : 0;
}

void h264_mp4_close(void *wp)
{
	struct h264_mp4 *w = wp;

	if (!w)
		return;

	if (w->header)
		(void)av_write_trailer(w->fmt);
	if (w->fmt) {
		if (w->fmt->pb)
			avio_closep(&w->fmt->pb);
		avformat_free_context(w->fmt);
	}
	mem_deref(w);
}
