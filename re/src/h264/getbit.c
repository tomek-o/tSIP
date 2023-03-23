/**
 * @file h264/getbit.c Generic bit parser
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <re_types.h>
#include <re_fmt.h>
#include "h264.h"

#define DEBUG_MODULE "h264_getbit"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


void getbit_init(struct getbit *gb, const uint8_t *buf, size_t size)
{
	if (!gb)
		return;

	gb->buf = buf;
	gb->pos = 0;
	gb->end = size;
}


size_t getbit_get_left(const struct getbit *gb)
{
	if (!gb)
		return 0;

	if (gb->end > gb->pos)
		return gb->end - gb->pos;
	else
		return 0;
}


unsigned get_bit(struct getbit *gb)
{
	const uint8_t *p;
	register unsigned tmp;

	if (!gb)
		return 0;

	if (gb->pos >= gb->end) {
		DEBUG_WARNING("get_bit: read past end"
			   " (%zu >= %zu)\n", gb->pos, gb->end);
		return 0;
	}

	p = gb->buf;
	tmp = ((*(p + (gb->pos >> 0x3))) >> (0x7 - (gb->pos & 0x7))) & 0x1;

	++gb->pos;

	return tmp;
}


int get_ue_golomb(struct getbit *gb, unsigned *valp)
{
	unsigned zeros = 0;
	unsigned info;
	int i;

	if (!gb)
		return EINVAL;

	while (1) {

		if (getbit_get_left(gb) < 1)
			return EBADMSG;

		if (0 == get_bit(gb))
			++zeros;
		else
			break;
	}

	info = 1 << zeros;

	for (i = zeros - 1; i >= 0; i--) {

		if (getbit_get_left(gb) < 1)
			return EBADMSG;

		info |= get_bit(gb) << i;
	}

	if (valp)
		*valp = info - 1;

	return 0;
}
