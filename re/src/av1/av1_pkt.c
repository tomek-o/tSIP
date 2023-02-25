/**
 * @file av1/pkt.c AV1 Packetizer
 *
 * Copyright (C) 2010 - 2022 Alfred E. Heggestad
 */

#include <string.h>
#include <re_types.h>
#include <re_fmt.h>
#include <re_mem.h>
#include <re_mbuf.h>
#include <re_av1.h>


#define DEBUG_MODULE "av1"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


static void hdr_encode(uint8_t hdr[AV1_AGGR_HDR_SIZE],
		       bool z, bool y, uint8_t w, bool n)
{
	hdr[0] = z<<7 | y<<6 | w<<4 | n<<3;
}


/**
 * Packetize an AV1 bitstream with one or more OBUs
 *
 * @param newp    Pointer to new stream flag
 * @param marker  Set marker bit
 * @param rtp_ts  RTP timestamp
 * @param buf     Input buffer
 * @param len     Buffer length
 * @param maxlen  Maximum RTP packet size
 * @param pkth    Packet handler
 * @param arg     Handler argument
 *
 * @return 0 if success, otherwise errorcode
 */
int av1_packetize(bool *newp, bool marker, uint64_t rtp_ts,
		  const uint8_t *buf, size_t len, size_t maxlen,
		  av1_packet_h *pkth, void *arg)
{
	uint8_t hdr[AV1_AGGR_HDR_SIZE];
	bool cont = false;
	uint8_t w = 0;  /* variable OBU count */
	int err = 0;

	if (!newp || !buf || !len || maxlen < (AV1_AGGR_HDR_SIZE + 1) || !pkth)
		return EINVAL;

	maxlen -= sizeof(hdr);

	while (len > maxlen) {

		hdr_encode(hdr, cont, true, w, *newp);
		*newp = false;

		err |= pkth(false, rtp_ts, hdr, sizeof(hdr), buf, maxlen, arg);

		buf  += maxlen;
		len  -= maxlen;
		cont = true;
	}

	hdr_encode(hdr, cont, false, w, *newp);
	*newp = false;

	err |= pkth(marker, rtp_ts, hdr, sizeof(hdr), buf, len, arg);

	return err;
}


static struct mbuf *encode_obu(uint8_t type, const uint8_t *p, size_t len)
{
	struct mbuf *mb = mbuf_alloc(len);
	const bool has_size = false;  /* NOTE */
	int err;

	if (!mb)
		return NULL;

	err = av1_obu_encode(mb, type, has_size, len, p);
	if (err) {
		mem_deref(mb);
		return NULL;
	}

	mb->pos = 0;

	return mb;
}


static int copy_obus(struct mbuf *mb_pkt, const uint8_t *buf, size_t size,
		     bool w0)
{
	struct mbuf wrap;
	struct mbuf *mb_obu = NULL;
	int err = 0;

	wrap.buf  = (uint8_t *)buf;
	wrap.size = size;
	wrap.pos  = 0;
	wrap.end  = size;

	while (mbuf_get_left(&wrap) >= 2) {

		struct av1_obu_hdr hdr;
		bool last;

		err = av1_obu_decode(&hdr, &wrap);
		if (err) {
			DEBUG_WARNING("av1: encode: hdr dec error (%m)\n",
				      err);
			return err;
		}

		last = (hdr.size == mbuf_get_left(&wrap));

		switch (hdr.type) {

		case AV1_OBU_SEQUENCE_HEADER:
		case AV1_OBU_FRAME_HEADER:
		case AV1_OBU_METADATA:
		case AV1_OBU_FRAME:
		case AV1_OBU_REDUNDANT_FRAME_HEADER:
		case AV1_OBU_TILE_GROUP:
			mb_obu = encode_obu(hdr.type, mbuf_buf(&wrap),
					    hdr.size);
			if (!mb_obu) {
				err = ENOMEM;
				goto out;
			}

			if (last) {
				if (w0)
					err = av1_leb128_encode(mb_pkt,
								mb_obu->end);
			}
			else {
				err = av1_leb128_encode(mb_pkt, mb_obu->end);
			}

			if (err)
				goto out;

			err = mbuf_write_mem(mb_pkt, mb_obu->buf, mb_obu->end);
			if (err)
				goto out;

			break;

		case AV1_OBU_TEMPORAL_DELIMITER:
		case AV1_OBU_TILE_LIST:
		case AV1_OBU_PADDING:
			/* skip */
			break;

		default:
			DEBUG_WARNING("av1: unknown obu type %u\n", hdr.type);
			break;
		}

		mbuf_advance(&wrap, hdr.size);

		mb_obu = mem_deref(mb_obu);
	}

 out:
	mem_deref(mb_obu);
	return err;
}


static int av1_packetize_internal(bool *newp, bool marker, uint64_t rtp_ts,
				  const uint8_t *buf, size_t len,
				  size_t maxlen, uint8_t w,
				  av1_packet_h *pkth, void *arg)
{
	uint8_t hdr[AV1_AGGR_HDR_SIZE];
	bool cont = false;
	int err = 0;


	if (w > 3) {
		DEBUG_WARNING("w too large\n");
		return EPROTO;
	}

	maxlen -= sizeof(hdr);

	while (len > maxlen) {

		hdr_encode(hdr, cont, true, w, *newp);
		*newp = false;

		err |= pkth(false, rtp_ts, hdr, sizeof(hdr), buf, maxlen, arg);

		buf  += maxlen;
		len  -= maxlen;
		cont = true;
	}

	hdr_encode(hdr, cont, false, w, *newp);
	*newp = false;

	err |= pkth(marker, rtp_ts, hdr, sizeof(hdr), buf, len, arg);

	return err;
}


int av1_packetize_high(bool *newp, bool marker, uint64_t rtp_ts,
		       const uint8_t *buf, size_t len, size_t maxlen,
		       av1_packet_h *pkth, void *arg)
{
	struct mbuf *mb_pkt;
	unsigned count = 0;
	uint8_t w;
	int err;

	if (!newp || !buf || !len || maxlen < (AV1_AGGR_HDR_SIZE + 1) || !pkth)
		return EINVAL;

	mb_pkt = mbuf_alloc(len);
	if (!mb_pkt)
		return ENOMEM;

	count = av1_obu_count_rtp(buf, len);

	if (count > 3) {
		w = 0;
	}
	else {
		w = count;
	}

	err = copy_obus(mb_pkt, buf, len, count > 3);
	if (err)
		goto out;

	err = av1_packetize_internal(newp, marker, rtp_ts,
				     mb_pkt->buf, mb_pkt->end, maxlen,
				     w,
				     pkth, arg);
	if (err)
		goto out;

 out:
	mem_deref(mb_pkt);
	return err;
}
