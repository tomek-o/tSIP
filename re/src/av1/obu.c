/**
 * @file av1/obu.c AV1 Open Bitstream Unit (OBU)
 *
 * Copyright (C) 2010 - 2022 Alfred E. Heggestad
 */

#include <string.h>
#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_av1.h>


#define DEBUG_MODULE "av1"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


/**
 * Encode a number into LEB128 format, which is an unsigned integer
 * represented by a variable number of little-endian bytes.
 *
 * @param mb    Mbuffer to encode into
 * @param value Value to encode
 *
 * @return 0 if success, otherwise errorcode
 */
int av1_leb128_encode(struct mbuf *mb, uint64_t value)
{
	int err = 0;

	if (!mb)
		return EINVAL;

	while (value >= 0x80) {

		uint8_t u8 = 0x80 | (value & 0x7f);

		err |= mbuf_write_u8(mb, u8);

		value >>= 7;
	}

	err |= mbuf_write_u8(mb, (uint8_t)value);

	return err;
}


/**
 * Decode a number in LEB128 format, which is an unsigned integer
 * represented by a variable number of little-endian bytes.
 *
 * @param mb    Mbuffer to decode from
 * @param value Decoded value, set on return
 *
 * @return 0 if success, otherwise errorcode
 */
int av1_leb128_decode(struct mbuf *mb, uint64_t *value)
{
	uint64_t ret = 0;
	unsigned i;

	if (!mb || !value)
		return EINVAL;

	for (i = 0; i < 8; i++) {

		size_t byte;

		if (mbuf_get_left(mb) < 1)
			return EBADMSG;

		byte = mbuf_read_u8(mb);

		ret |= (uint64_t)(byte & 0x7f) << (i * 7);

		if (!(byte & 0x80))
			break;
	}

	*value = ret;

	return 0;
}


/**
 * Encode an OBU into an mbuffer
 *
 * @param mb       Mbuffer to encode into
 * @param type     OBU type
 * @param has_size True to use the 'has_size' field
 * @param len      Number of bytes
 * @param payload  Optional OBU payload
 *
 * @return 0 if success, otherwise errorcode
 */
int av1_obu_encode(struct mbuf *mb, uint8_t type, bool has_size,
		   size_t len, const uint8_t *payload)
{
	uint8_t val;
	int err;

	if (!mb || type==0)
		return EINVAL;

	val  = (type&0xf) << 3;
	val |= (unsigned)has_size << 1;

	err  = mbuf_write_u8(mb, val);

	if (has_size)
		err |= av1_leb128_encode(mb, len);

	if (payload && len)
		err |= mbuf_write_mem(mb, payload, len);

	return err;
}


/**
 * Decode an OBU header from mbuffer
 *
 * @param hdr Decoded OBU header
 * @param mb  Mbuffer to decode from
 *
 * @return 0 if success, otherwise errorcode
 */
int av1_obu_decode(struct av1_obu_hdr *hdr, struct mbuf *mb)
{
	uint8_t val;
	bool f;
	int err;

	if (!hdr || !mb)
		return EINVAL;

	if (mbuf_get_left(mb) < 1)
		return EBADMSG;

	memset(hdr, 0, sizeof(*hdr));

	val = mbuf_read_u8(mb);

	f         = (val >> 7) & 0x1;
	hdr->type = (val >> 3) & 0xf;
	hdr->x    = (val >> 2) & 0x1;
	hdr->s    = (val >> 1) & 0x1;

	if (f) {
		DEBUG_WARNING("av1: header: obu forbidden bit!"
			" [type=%u, x=%d, s=%d, left=%zu bytes]\n",
			hdr->type, hdr->x, hdr->s, mbuf_get_left(mb));
		return EBADMSG;
	}

	if (hdr->type == 0) {
		DEBUG_WARNING("av1: header: obu type 0 is reserved [%H]\n",
			      av1_obu_print, hdr);
		return EBADMSG;
	}

	if (hdr->x) {
		DEBUG_WARNING("av1: header: extension not supported (%u)\n",
			      hdr->type);
		return ENOTSUP;
	}

	if (hdr->s) {
		uint64_t size;

		err = av1_leb128_decode(mb, &size);
		if (err)
			return err;

		if (size > mbuf_get_left(mb)) {
			DEBUG_WARNING("av1: obu decode: short packet:"
				      " %llu > %zu\n",
				      size, mbuf_get_left(mb));
			return EBADMSG;
		}

		hdr->size = (size_t)size;
	}
	else {
		hdr->size = mbuf_get_left(mb);
	}

	return 0;
}


int av1_obu_print(struct re_printf *pf, const struct av1_obu_hdr *hdr)
{
	if (!hdr)
		return 0;

	return re_hprintf(pf, "type=%u,%-24s x=%d s=%d size=%zu",
			  hdr->type, av1_obu_name(hdr->type),
			  hdr->x, hdr->s, hdr->size);
}


unsigned av1_obu_count(const uint8_t *buf, size_t size)
{
	struct mbuf wrap;
	unsigned count = 0;

	wrap.buf = (uint8_t *)buf;
	wrap.size = size;
	wrap.pos = 0;
	wrap.end = size;

	while (mbuf_get_left(&wrap) > 1) {

		struct av1_obu_hdr hdr;

		int err = av1_obu_decode(&hdr, &wrap);
		if (err) {
			DEBUG_WARNING("count: could not decode OBU"
				      " [%zu bytes]: %m\n", size, err);
			return 0;
		}

		mbuf_advance(&wrap, hdr.size);

		++count;
	}

	return count;
}


unsigned av1_obu_count_rtp(const uint8_t *buf, size_t size)
{
	struct mbuf wrap;
	unsigned count = 0;

	wrap.buf = (uint8_t *)buf;
	wrap.size = size;
	wrap.pos = 0;
	wrap.end = size;

	while (mbuf_get_left(&wrap) > 1) {

		struct av1_obu_hdr hdr;

		int err = av1_obu_decode(&hdr, &wrap);
		if (err) {
			DEBUG_WARNING("count: could not decode OBU"
				      " [%zu bytes]: %m\n", size, err);
			return 0;
		}

		switch (hdr.type) {

		case AV1_OBU_SEQUENCE_HEADER:
		case AV1_OBU_FRAME_HEADER:
		case AV1_OBU_METADATA:
		case AV1_OBU_FRAME:
		case AV1_OBU_REDUNDANT_FRAME_HEADER:
		case AV1_OBU_TILE_GROUP:
			++count;
			break;

		default:
			break;
		}

		mbuf_advance(&wrap, hdr.size);
	}

	return count;
}


const char *av1_obu_name(enum obu_type type)
{
	switch (type) {

	case AV1_OBU_SEQUENCE_HEADER:        return "OBU_SEQUENCE_HEADER";
	case AV1_OBU_TEMPORAL_DELIMITER:     return "OBU_TEMPORAL_DELIMITER";
	case AV1_OBU_FRAME_HEADER:           return "OBU_FRAME_HEADER";
	case AV1_OBU_REDUNDANT_FRAME_HEADER:
		return "OBU_REDUNDANT_FRAME_HEADER";
	case AV1_OBU_FRAME:                  return "OBU_FRAME";
	case AV1_OBU_TILE_GROUP:             return "OBU_TILE_GROUP";
	case AV1_OBU_METADATA:               return "OBU_METADATA";
	case AV1_OBU_TILE_LIST:              return "OBU_TILE_LIST";
	case AV1_OBU_PADDING:                return "OBU_PADDING";
	default: return "???";
	}
}
