/**
 * @file av1/depack.c AV1 De-packetizer
 *
 * Copyright (C) 2010 - 2022 Alfred E. Heggestad
 */

#include <string.h>
#include <re_types.h>
#include <re_mbuf.h>
#include <re_av1.h>


/**
 * Decode an AV1 Aggregation header from mbuffer
 *
 * @param hdr Decoded aggregation header
 * @param mb  Mbuffer to decode from
 *
 * @return 0 if success, otherwise errorcode
 */
int av1_aggr_hdr_decode(struct av1_aggr_hdr *hdr, struct mbuf *mb)
{
	uint8_t v;

	if (!hdr || !mb)
		return EINVAL;

	memset(hdr, 0, sizeof(*hdr));

	if (mbuf_get_left(mb) < 1)
		return EBADMSG;

	v = mbuf_read_u8(mb);

	hdr->z = v>>7 & 0x1;
	hdr->y = v>>6 & 0x1;
	hdr->w = v>>4 & 0x3;
	hdr->n = v>>3 & 0x1;

	return 0;
}
