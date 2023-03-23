/**
 * @file avcodec.h  Video codecs using FFmpeg libavcodec -- internal API
 *
 * Copyright (C) 2010 Creytiv.com
 */


extern const uint8_t h264_level_idc;
extern const AVCodec *avcodec_h264enc;
extern const AVCodec *avcodec_h264dec;

extern const AVCodec *avcodec_h265enc;
extern const AVCodec *avcodec_h265dec;


/*
 * Encode
 */

struct videnc_state;

int encode_update(struct videnc_state **vesp, const struct vidcodec *vc,
		  struct videnc_param *prm, const char *fmtp);
int encode(struct videnc_state *st, bool update, const struct vidframe *frame,
	   videnc_packet_h *pkth, void *arg);


/*
 * Decode
 */

struct viddec_state;

int decode_update(struct viddec_state **vdsp, const struct vidcodec *vc,
		  const char *fmtp);
int avcodec_decode_h263(struct viddec_state *st, struct vidframe *frame,
		bool *intra, bool marker, uint16_t seq, struct mbuf *src);
int avcodec_decode_h264(struct viddec_state *st, struct vidframe *frame,
		bool *intra, bool marker, uint16_t seq, struct mbuf *src);
int avcodec_decode_mpeg4(struct viddec_state *st, struct vidframe *frame,
		 bool *intra, bool marker, uint16_t seq, struct mbuf *src);
int decode_h263_test(struct viddec_state *st, struct vidframe *frame,
		     bool marker, uint16_t seq, struct mbuf *src);


int decode_sdpparam_h264(struct videnc_state *st, const struct pl *name,
			 const struct pl *val);
int h264_decode(struct viddec_state *st, struct mbuf *src);


int avcodec_resolve_codecid(const char *s);


/*
 * SDP
 */

uint32_t h264_packetization_mode(const char *fmtp);
int avcodec_h264_fmtp_enc(struct mbuf *mb, const struct sdp_format *fmt,
		  bool offer, void *arg);
bool avcodec_h264_fmtp_cmp(const char *lfmtp, const char *rfmtp, void *data);
