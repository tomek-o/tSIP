/**
 * @file rem_auresamp.h Audio Resampling
 *
 * Copyright (C) 2010 Creytiv.com
 */

struct auresamp;

int auresamp_alloc(struct auresamp **arp, size_t sampc_max,
		   uint32_t srate_in, uint8_t ch_in,
		   uint32_t srate_out, uint8_t ch_out);
int auresamp_process(struct auresamp *ar,
		     int16_t *dst_sampv, size_t *dst_sampc,
		     const int16_t *src_sampv, size_t src_sampc);
