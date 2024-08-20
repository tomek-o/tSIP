/**
 * @file avcodec/decode.c  Video codecs using FFmpeg libavcodec -- decoder
 *
 * Copyright (C) 2010 - 2013 Creytiv.com
 */
#include <re.h>
#include <re_h264.h>
#include <stdint.h>
#include <rem.h>
#include <baresip.h>
#ifdef __BORLANDC__
#pragma warn -8056	// disable "Integer arithmetic overflow" warning
#endif
#include <libavcodec/avcodec.h>
#ifdef __BORLANDC__
#pragma warn .8056
#endif
#include "h26x.h"
#include "avcodec.h"


#define DEBUG_MODULE "avcodec"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

#ifndef AV_INPUT_BUFFER_PADDING_SIZE
#define AV_INPUT_BUFFER_PADDING_SIZE 64
#endif


enum {
	DECODE_MAXSZ = 524288,
};

static char *av_err_str(char *errbuf, size_t errbuf_size, int errnum)
{
	av_strerror(errnum, errbuf, errbuf_size);
	return errbuf;
}

struct viddec_state {
	const AVCodec *codec;
	AVCodecContext *ctx;
	AVFrame *pict;
	struct mbuf *mb;
	bool got_keyframe;
	size_t frag_start;
	bool frag;
	uint16_t frag_seq;

	struct {
		unsigned n_key;
		unsigned n_lost;
	} stats;
};


static void destructor(void *arg)
{
	struct viddec_state *st = arg;

	DEBUG_WARNING("avcodec: decoder stats"
	      " (keyframes:%u, lost_fragments:%u)\n",
	      st->stats.n_key, st->stats.n_lost);

	mem_deref(st->mb);

	if (st->ctx) {
		if (st->ctx->codec)
			avcodec_close(st->ctx);
		av_free(st->ctx);
	}

	if (st->pict)
		av_free(st->pict);
}


static void fragment_rewind(struct viddec_state *vds)
{
	vds->mb->pos = vds->frag_start;
	vds->mb->end = vds->frag_start;
}

static int init_decoder(struct viddec_state *st, const char *name)
{
	enum AVCodecID codec_id;

	codec_id = avcodec_resolve_codecid(name);
	if (codec_id == AV_CODEC_ID_NONE)
		return EINVAL;

	st->codec = avcodec_find_decoder(codec_id);
	if (!st->codec)
		return ENOENT;

	st->ctx = avcodec_alloc_context3(st->codec);

	st->pict = av_frame_alloc();

	if (!st->ctx || !st->pict)
		return ENOMEM;

	if (avcodec_open2(st->ctx, st->codec, NULL) < 0)
		return ENOENT;

	return 0;
}


int decode_update(struct viddec_state **vdsp, const struct vidcodec *vc,
		  const char *fmtp)
{
	struct viddec_state *st;
	int err = 0;

	if (!vdsp || !vc)
		return EINVAL;

	if (*vdsp)
		return 0;

	(void)fmtp;

	st = mem_zalloc(sizeof(*st), destructor);
	if (!st)
		return ENOMEM;

	st->mb = mbuf_alloc(1024);
	if (!st->mb) {
		err = ENOMEM;
		goto out;
	}

	err = init_decoder(st, vc->name);
	if (err) {
		DEBUG_WARNING("%s: could not init decoder\n", vc->name);
		goto out;
	}

	re_printf("video decoder %s (%s)\n", vc->name, fmtp);

 out:
	if (err)
		mem_deref(st);
	else
		*vdsp = st;

	return err;
}


static int ffdecode(struct viddec_state *st, struct vidframe *frame,
		    bool *intra)
{
	AVPacket *avpkt;
	int i, got_picture, ret;
	int err = 0;


	err = mbuf_fill(st->mb, 0x00, AV_INPUT_BUFFER_PADDING_SIZE);
	if (err)
		return err;
	st->mb->end -= AV_INPUT_BUFFER_PADDING_SIZE;

	avpkt = av_packet_alloc();
	if (!avpkt) {
		err = ENOMEM;
		goto out;
	}

	avpkt->data = st->mb->buf;
	avpkt->size = (int)st->mb->end;

	ret = avcodec_send_packet(st->ctx, avpkt);
	if (ret < 0) {
		char err_buf[AV_ERROR_MAX_STRING_SIZE] = {0};
		DEBUG_WARNING("avcodec: decode: avcodec_send_packet error,"
			" packet=%zu bytes, ret=%d (%s)\n",
			st->mb->end, ret, av_err_str(err_buf, AV_ERROR_MAX_STRING_SIZE, ret));
		err = EBADMSG;
		goto out;
	}

	ret = avcodec_receive_frame(st->ctx, st->pict);
	if (ret == AVERROR(EAGAIN)) {
		goto out;
	}
	else if (ret < 0) {
		DEBUG_WARNING("avcodec: avcodec_receive_frame error ret=%d\n", ret);
		err = EBADMSG;
		goto out;
	}

	got_picture = true;

	if (got_picture) {
		for (i=0; i<4; i++) {
			frame->data[i]     = st->pict->data[i];
			frame->linesize[i] = st->pict->linesize[i];
		}
		frame->size.w = st->ctx->width;
		frame->size.h = st->ctx->height;
		frame->fmt    = VID_FMT_YUV420P;

		if (st->pict->key_frame) {

			*intra = true;
			st->got_keyframe = true;
			++st->stats.n_key;
		}
	}

 out:
	av_packet_free(&avpkt);
	return err;
}

int avcodec_decode_h264(struct viddec_state *st, struct vidframe *frame,
			bool *intra, bool marker, uint16_t seq,
			struct mbuf *src)
{
	struct h264_nal_header h264_hdr;
	const uint8_t nal_seq[3] = {0, 0, 1};
	int err;

	if (!st || !frame || !intra || !src)
		return EINVAL;

	*intra = false;

	err = h264_nal_header_decode(&h264_hdr, src);
	if (err)
		return err;

#if 0
	re_printf("avcodec: decode: %s %s type=%2d %s  \n",
		  marker ? "[M]" : "   ",
		  h264_is_keyframe(h264_hdr.type) ? "<KEY>" : "     ",
		  h264_hdr.type,
		  h264_nal_unit_name(h264_hdr.type));
#endif

	if (h264_hdr.type == H264_NALU_SLICE && !st->got_keyframe) {
		DEBUG_WARNING("avcodec: decoder waiting for keyframe\n");
		return EPROTO;
	}

	if (h264_hdr.f) {
		DEBUG_WARNING("avcodec: H264 forbidden bit set!\n");
		return EBADMSG;
	}

	if (st->frag && h264_hdr.type != H264_NALU_FU_A) {
		DEBUG_WARNING("avcodec: lost fragments; discarding previous NAL\n");
		fragment_rewind(st);
		st->frag = false;
		++st->stats.n_lost;
	}

	/* handle NAL types */
	if (1 <= h264_hdr.type && h264_hdr.type <= 23) {

		--src->pos;

		/* prepend H.264 NAL start sequence */
		err  = mbuf_write_mem(st->mb, nal_seq, 3);

		err |= mbuf_write_mem(st->mb, mbuf_buf(src),
				      mbuf_get_left(src));
		if (err)
			goto out;
	}
	else if (H264_NALU_FU_A == h264_hdr.type) {
		struct h264_fu fu;

		err = h264_fu_hdr_decode(&fu, src);
		if (err)
			return err;
		h264_hdr.type = fu.type;

		if (fu.s) {
			if (st->frag) {
				DEBUG_WARNING("avcodec: start: lost fragments;"
				      " ignoring previous NAL\n");
				fragment_rewind(st);
				++st->stats.n_lost;
			}

			st->frag_start = st->mb->pos;
			st->frag = true;

			/* prepend H.264 NAL start sequence */
			mbuf_write_mem(st->mb, nal_seq, 3);

			/* encode NAL header back to buffer */
			err = h264_nal_header_encode(st->mb, &h264_hdr);
			if (err)
				goto out;
		}
		else {
			int diff;
			if (!st->frag) {
				DEBUG_WARNING("avcodec: ignoring fragment"
					  " (nal=%u)\n", fu.type);
				++st->stats.n_lost;
				return 0;
			}

			diff = rtp_seq_diff(st->frag_seq, seq);
			if (diff != 1) {
				DEBUG_WARNING("avcodec: lost fragments detected (seq diff = %d, frag_seq = %d, seq = %d)\n", diff, st->frag_seq, seq);
				fragment_rewind(st);
				st->frag = false;
				++st->stats.n_lost;
				return 0;
			}
		}

		err = mbuf_write_mem(st->mb, mbuf_buf(src),
				     mbuf_get_left(src));
		if (err)
			goto out;

		if (fu.e)
			st->frag = false;

		st->frag_seq = seq;
	}
	else if (H264_NALU_STAP_A == h264_hdr.type) {

		while (mbuf_get_left(src) >= 2) {

			const uint16_t len = ntohs(mbuf_read_u16(src));
			struct h264_nal_header lhdr;

			if (mbuf_get_left(src) < len)
				return EBADMSG;

			err = h264_nal_header_decode(&lhdr, src);
			if (err)
				return err;

			--src->pos;

			err  = mbuf_write_mem(st->mb, nal_seq, 3);
			err |= mbuf_write_mem(st->mb, mbuf_buf(src), len);
			if (err)
				goto out;

			src->pos += len;
		}
	}
	else {
		DEBUG_WARNING("avcodec: decode: unknown NAL type %u\n",
			h264_hdr.type);
		return EBADMSG;
	}

	if (!marker) {

		if (st->mb->end > DECODE_MAXSZ) {
			DEBUG_WARNING("avcodec: decode buffer size exceeded\n");
			err = ENOMEM;
			goto out;
		}

		return 0;
	}

	if (st->frag) {
		err = EPROTO;
		goto out;
	}

	err = ffdecode(st, frame, intra);
	if (err)
		goto out;

 out:
	mbuf_rewind(st->mb);
	st->frag = false;

	return err;
} 


int avcodec_decode_mpeg4(struct viddec_state *st, struct vidframe *frame,
		 bool *intra, bool marker, uint16_t seq, struct mbuf *src)
{
	int err;

	if (!src)
		return 0;

	(void)seq;

	*intra = false;

	err = mbuf_write_mem(st->mb, mbuf_buf(src),
			     mbuf_get_left(src));
	if (err)
		goto out;

	if (!marker) {

		if (st->mb->end > DECODE_MAXSZ) {
			DEBUG_WARNING("avcodec: decode buffer size exceeded\n");
			err = ENOMEM;
			goto out;
		}

		return 0;
	}

	err = ffdecode(st, frame, intra);
	if (err)
		goto out;

 out:
	mbuf_rewind(st->mb);

	return err;
}


int avcodec_decode_h263(struct viddec_state *st, struct vidframe *frame,
		bool *intra, bool marker, uint16_t seq, struct mbuf *src)
{
	struct h263_hdr hdr;
	int err;

	if (!st || !frame || !intra)
		return EINVAL;

	*intra = false;

	if (!src)
		return 0;

	(void)seq;

	err = h263_hdr_decode(&hdr, src);
	if (err)
		return err;

	if (hdr.i && !st->got_keyframe)
		return EPROTO;

#if 0
	debug(".....[%s seq=%5u ] MODE %s -"
	      " SBIT=%u EBIT=%u I=%s"
	      " (%5u/%5u bytes)\n",
	      marker ? "M" : " ", seq,
	      h263_hdr_mode(&hdr) == H263_MODE_A ? "A" : "B",
	      hdr.sbit, hdr.ebit, hdr.i ? "Inter" : "Intra",
	      mbuf_get_left(src), st->mb->end);
#endif

#if 0
	if (st->mb->pos == 0) {
		uint8_t *p = mbuf_buf(src);

		if (p[0] != 0x00 || p[1] != 0x00) {
			warning("invalid PSC detected (%02x %02x)\n",
				p[0], p[1]);
			return EPROTO;
		}
	}
#endif

	/*
	 * The H.263 Bit-stream can be fragmented on bit-level,
	 * indicated by SBIT and EBIT. Example:
	 *
	 *               8 bit  2 bit
	 *            .--------.--.
	 * Packet 1   |        |  |
	 * SBIT=0     '--------'--'
	 * EBIT=6
	 *                        .------.--------.--------.
	 * Packet 2               |      |        |        |
	 * SBIT=2                 '------'--------'--------'
	 * EBIT=0                   6bit    8bit     8bit
	 *
	 */

	if (hdr.sbit > 0) {
		const uint8_t mask  = (1 << (8 - hdr.sbit)) - 1;
		const uint8_t sbyte = mbuf_read_u8(src) & mask;

		st->mb->buf[st->mb->end - 1] |= sbyte;
	}

	err = mbuf_write_mem(st->mb, mbuf_buf(src),
			     mbuf_get_left(src));
	if (err)
		goto out;

	if (!marker) {

		if (st->mb->end > DECODE_MAXSZ) {
			DEBUG_WARNING("avcodec: decode buffer size exceeded\n");
			err = ENOMEM;
			goto out;
		}

		return 0;
	}

	err = ffdecode(st, frame, intra);
	if (err)
		goto out;

 out:
	mbuf_rewind(st->mb);

	return err;
}
