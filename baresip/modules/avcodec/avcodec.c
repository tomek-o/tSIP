/**
 * @file avcodec.c  Video codecs using FFmpeg libavcodec
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
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


const AVCodec *avcodec_h264enc;      /* optional; specified H.264 encoder */
const AVCodec *avcodec_h264dec;      /* optional; specified H.264 decoder */
const AVCodec *avcodec_h265enc;
const AVCodec *avcodec_h265dec;

int avcodec_resolve_codecid(const char *s)
{
	if (0 == str_casecmp(s, "H263"))
		return AV_CODEC_ID_H263;
	else if (0 == str_casecmp(s, "H264"))
		return AV_CODEC_ID_H264;
#ifdef AV_CODEC_ID_H265
	else if (0 == str_casecmp(s, "H265"))
		return AV_CODEC_ID_H265;
#endif
	else if (0 == str_casecmp(s, "MP4V-ES"))
		return AV_CODEC_ID_MPEG4;
	else
		return AV_CODEC_ID_NONE;
}




static int h263_fmtp_enc(struct mbuf *mb, const struct sdp_format *fmt,
			 bool offer, void *arg)
{
	(void)offer;
	(void)arg;

	if (!mb || !fmt)
		return 0;

	return mbuf_printf(mb, "a=fmtp:%s CIF=1;CIF4=1\r\n", fmt->id);
}


static int mpg4_fmtp_enc(struct mbuf *mb, const struct sdp_format *fmt,
			 bool offer, void *arg)
{
	(void)offer;
	(void)arg;

	if (!mb || !fmt)
		return 0;

	return mbuf_printf(mb, "a=fmtp:%s profile-level-id=3\r\n", fmt->id);
}


static struct vidcodec h264_0 = {
#if defined(__BORLANDC__)
	LE_INIT,
	NULL,
	"H264",
	"packetization-mode=0",
	NULL,
	encode_update,
	encode,
	decode_update,
	decode_h264,
	avcodec_h264_fmtp_enc,
	avcodec_h264_fmtp_cmp,
#else
	.name      = "H264",
	.variant   = "packetization-mode=0",
	.encupdh   = encode_update,
	.ench      = encode,
	.decupdh   = decode_update,
	.dech      = decode_h264,
	.fmtp_ench = avcodec_h264_fmtp_enc,
	.fmtp_cmph = avcodec_h264_fmtp_cmp,
#endif
};

static struct vidcodec h264_1 = {
	LE_INIT,
	NULL,
	"H264",
	"packetization-mode=1",
	NULL,
	encode_update,
	encode,
	decode_update,
	decode_h264,
	avcodec_h264_fmtp_enc,
	avcodec_h264_fmtp_cmp
};

#if 0
static struct vidcodec h265 = {
	LE_INIT,
	NULL,
	.name      = "H265",
	.fmtp      = "profile-id=1",
	.encupdh   = avcodec_encode_update,
	.ench      = avcodec_encode,
	.decupdh   = avcodec_decode_update,
	.dech      = avcodec_decode_h265,
	.packetizeh= avcodec_packetize,
};
#endif

static struct vidcodec h263 = {
#if defined(__BORLANDC__)
	LE_INIT,		/* le */
	"34",			/* pt */
	"H263",			/* name */
	NULL,			/* variant */
	NULL,			/* fmtp */
	encode_update,	/* encupdh */
	encode,			/* ench */
	decode_update,	/* decupdh */
	decode_h263,	/* dech */
	h263_fmtp_enc,	/* fmtp_ench */
	NULL			/* fmtp_cmph */
#else
	.pt        = "34",
	.name      = "H263",
	.encupdh   = encode_update,
	.ench      = encode,
	.decupdh   = decode_update,
	.dech      = decode_h263,
	.fmtp_ench = h263_fmtp_enc,
#endif
};

static struct vidcodec mpg4 = {
#if defined(__BORLANDC__)
	LE_INIT,		/* le */
	NULL,			/* pt */
	"MP4V-ES",		/* name */
	NULL,			/* variant */
	NULL,			/* fmtp */
	encode_update,	/* encupdh */
	encode,			/* ench */
	decode_update,	/* decupdh */
	decode_mpeg4,	/* dech */
	mpg4_fmtp_enc,	/* fmtp_ench */
	NULL			/* fmtp_cmph */
#else
	.name      = "MP4V-ES",
	.encupdh   = encode_update,
	.ench      = encode,
	.decupdh   = decode_update,
	.dech      = decode_mpeg4,
	.fmtp_ench = mpg4_fmtp_enc,
#endif
};


static int module_init(void)
{
#if 0
	char h264enc[64] = "libx264";
#else
	char h264enc[64] = "libopenh264";
#endif
	char h264dec[64] = "h264";
	char h265enc[64] = "libx265";
	char h265dec[64] = "hevc";

	avcodec_h264enc = avcodec_find_encoder_by_name(h264enc);
	if (!avcodec_h264enc) {
		DEBUG_WARNING("avcodec: h264 encoder not found (%s)\n", h264enc);
	}

	avcodec_h264dec = avcodec_find_decoder_by_name(h264dec);
	if (!avcodec_h264dec) {
		DEBUG_WARNING("avcodec: h264 decoder not found (%s)\n", h264dec);
	}

	avcodec_h265enc = avcodec_find_encoder_by_name(h265enc);
	avcodec_h265dec = avcodec_find_decoder_by_name(h265dec);

	if (avcodec_h264enc || avcodec_h264dec) {
		vidcodec_register(&h264_0);
		vidcodec_register(&h264_1);
	}
	if (avcodec_h264enc) {
		DEBUG_WARNING("avcodec: using H.264 encoder '%s' -- %s\n",
			 avcodec_h264enc->name, avcodec_h264enc->long_name);
	}
	if (avcodec_h264dec) {
		DEBUG_WARNING("avcodec: using H.264 decoder '%s' -- %s\n",
		     avcodec_h264dec->name, avcodec_h264dec->long_name);
	}

	if (avcodec_find_decoder(AV_CODEC_ID_H263))
		vidcodec_register(&h263);

	if (avcodec_find_decoder(AV_CODEC_ID_MPEG4))
		vidcodec_register(&mpg4);

	return 0;
}


static int module_close(void)
{
	vidcodec_unregister(&mpg4);
	vidcodec_unregister(&h263);
	vidcodec_unregister(&h264_0);
	vidcodec_unregister(&h264_1);

	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(avcodec) = {
	"avcodec",
	"codec",
	module_init,
	module_close
};
