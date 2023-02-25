/**
 * @file h265/nal.c H.265 header parser
 *
 * Copyright (C) 2010 - 2022 Alfred E. Heggestad
 */

#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_h265.h>

#define DEBUG_MODULE "h265_nal"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


void h265_nal_encode(uint8_t buf[2], unsigned nal_unit_type,
		    unsigned nuh_temporal_id_plus1)
{
	if (!buf)
		return;

	buf[0] = (nal_unit_type & 0x3f) << 1;
	buf[1] = nuh_temporal_id_plus1 & 0x07;
}


int h265_nal_encode_mbuf(struct mbuf *mb, const struct h265_nal *nal)
{
	uint8_t buf[2];

	h265_nal_encode(buf, nal->nal_unit_type, nal->nuh_temporal_id_plus1);

	return mbuf_write_mem(mb, buf, sizeof(buf));
}


int h265_nal_decode(struct h265_nal *nal, const uint8_t *p)
{
	bool forbidden_zero_bit;
	unsigned nuh_layer_id;

	if (!nal || !p)
		return EINVAL;

	forbidden_zero_bit         = p[0] >> 7;
	nal->nal_unit_type         = (p[0] >> 1) & 0x3f;
	nuh_layer_id               = (p[0]&1)<<5 | p[1] >> 3;
	nal->nuh_temporal_id_plus1 = p[1] & 0x07;

	if (forbidden_zero_bit) {
		DEBUG_WARNING("h265: nal_decode: FORBIDDEN bit set\n");
		return EBADMSG;
	}
	if (nuh_layer_id != 0) {
		DEBUG_WARNING("h265: nal_decode: LayerId MUST be zero\n");
		return EBADMSG;
	}

	return 0;
}


void h265_nal_print(const struct h265_nal *nal)
{
	if (!nal)
		return;

	re_printf("type=%u(%s), TID=%u\n",
		  nal->nal_unit_type,
		  h265_nalunit_name(nal->nal_unit_type),
		  nal->nuh_temporal_id_plus1);
}


const char *h265_nalunit_name(enum h265_naltype type)
{
	switch (type) {

	/* VCL class */
	case H265_NAL_TRAIL_N:         return "TRAIL_N";
	case H265_NAL_TRAIL_R:         return "TRAIL_R";

	case H265_NAL_TSA_N:           return "TSA_N";
	case H265_NAL_TSA_R:           return "TSA_R";

	case H265_NAL_STSA_N:          return "STSA_N";
	case H265_NAL_STSA_R:          return "STSA_R";
	case H265_NAL_RADL_N:          return "RADL_N";
	case H265_NAL_RADL_R:          return "RADL_R";

	case H265_NAL_RASL_N:          return "RASL_N";
	case H265_NAL_RASL_R:          return "RASL_R";

	case H265_NAL_BLA_W_LP:        return "BLA_W_LP";
	case H265_NAL_BLA_W_RADL:      return "BLA_W_RADL";
	case H265_NAL_BLA_N_LP:        return "BLA_N_LP";
	case H265_NAL_IDR_W_RADL:      return "IDR_W_RADL";
	case H265_NAL_IDR_N_LP:        return "IDR_N_LP";
	case H265_NAL_CRA_NUT:         return "CRA_NUT";

	/* non-VCL class */
	case H265_NAL_VPS_NUT:         return "VPS_NUT";
	case H265_NAL_SPS_NUT:         return "SPS_NUT";
	case H265_NAL_PPS_NUT:         return "PPS_NUT";
	case H265_NAL_AUD:             return "AUD";
	case H265_NAL_EOS_NUT:         return "EOS_NUT";
	case H265_NAL_EOB_NUT:         return "EOB_NUT";
	case H265_NAL_FD_NUT:          return "FD_NUT";
	case H265_NAL_PREFIX_SEI_NUT:  return "PREFIX_SEI_NUT";
	case H265_NAL_SUFFIX_SEI_NUT:  return "SUFFIX_SEI_NUT";

	/* RFC 7798 */
	case H265_NAL_AP:              return "H265_NAL_AP";
	case H265_NAL_FU:              return "H265_NAL_FU";
	}

	return "???";
}


bool h265_is_keyframe(enum h265_naltype type)
{
	/* between 16 and 21 (inclusive) */
	switch (type) {

	case H265_NAL_BLA_W_LP:
	case H265_NAL_BLA_W_RADL:
	case H265_NAL_BLA_N_LP:
	case H265_NAL_IDR_W_RADL:
	case H265_NAL_IDR_N_LP:
	case H265_NAL_CRA_NUT:
		return true;
	default:
		return false;
	}
}


static const uint8_t *h265_find_startcode(const uint8_t *p, const uint8_t *end)
{
	const uint8_t *a = p + 4 - ((size_t)p & 3);

	for (end -= 3; p < a && p < end; p++ ) {
		if (p[0] == 0 && p[1] == 0 && p[2] == 1)
			return p;
	}

	for (end -= 3; p < end; p += 4) {
		uint32_t x = *(const uint32_t*)(void *)p;
		if ( (x - 0x01010101) & (~x) & 0x80808080 ) {
			if (p[1] == 0 ) {
				if ( p[0] == 0 && p[2] == 1 )
					return p;
				if ( p[2] == 0 && p[3] == 1 )
					return p+1;
			}
			if ( p[3] == 0 ) {
				if ( p[2] == 0 && p[4] == 1 )
					return p+2;
				if ( p[4] == 0 && p[5] == 1 )
					return p+3;
			}
		}
	}

	for (end += 3; p < end; p++) {
		if (p[0] == 0 && p[1] == 0 && p[2] == 1)
			return p;
	}

	return end + 3;
}


static inline int packetize(bool marker, const uint8_t *buf, size_t len,
			    size_t maxlen, uint64_t rtp_ts,
			    h265_packet_h *pkth, void *arg)
{
	int err = 0;

	if (len <= maxlen) {
		err = pkth(marker, rtp_ts, NULL, 0, buf, len, arg);
	}
	else {
		struct h265_nal nal;
		uint8_t fu_hdr[3];
		const size_t flen = maxlen - sizeof(fu_hdr);

		err = h265_nal_decode(&nal, buf);
		if (err) {
			DEBUG_WARNING("h265: encode: could not decode"
				" NAL of %zu bytes (%m)\n", len, err);
			return err;
		}

		h265_nal_encode(fu_hdr, H265_NAL_FU,
				nal.nuh_temporal_id_plus1);

		fu_hdr[2] = 1<<7 | nal.nal_unit_type;

		buf+=2;
		len-=2;

		while (len > flen) {
			err |= pkth(false, rtp_ts, fu_hdr, 3, buf, flen,
				    arg);

			buf += flen;
			len -= flen;
			fu_hdr[2] &= ~(1 << 7); /* clear Start bit */
		}

		fu_hdr[2] |= 1<<6;  /* set END bit */

		err |= pkth(marker, rtp_ts, fu_hdr, 3, buf, len,
			    arg);
	}

	return err;
}


int h265_packetize(uint64_t rtp_ts, const uint8_t *buf, size_t len,
		   size_t pktsize, h265_packet_h *pkth, void *arg)
{
	const uint8_t *start = buf;
	const uint8_t *end   = buf + len;
	const uint8_t *r;
	int err = 0;

	r = h265_find_startcode(start, end);

	while (r < end) {
		const uint8_t *r1;
		bool marker;

		/* skip zeros */
		while (!*(r++))
			;

		r1 = h265_find_startcode(r, end);

		marker = (r1 >= end);

		err |= packetize(marker, r, r1-r, pktsize, rtp_ts, pkth, arg);

		r = r1;
	}

	return err;
}
