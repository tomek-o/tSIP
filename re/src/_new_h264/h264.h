/**
 * @file h264/h264.h Internal interface
 *
 * Copyright (C) 2010 Creytiv.com
 */


struct getbit {
	const uint8_t *buf;
	size_t pos;
	size_t end;
};


void     getbit_init(struct getbit *gb, const uint8_t *buf, size_t size);
size_t   getbit_get_left(const struct getbit *gb);
unsigned get_bit(struct getbit *gb);
int      get_ue_golomb(struct getbit *gb, unsigned *valp);
