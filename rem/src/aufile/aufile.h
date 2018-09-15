/**
 * @file aufile.h  Audio File -- internal API
 *
 * Copyright (C) 2010 Creytiv.com
 */


enum wavfmt {
	WAVE_FMT_PCM    = 0x0001,
	WAVE_FMT_ALAW   = 0x0006,
	WAVE_FMT_ULAW   = 0x0007,
};

/** WAVE format sub-chunk */
struct wav_fmt {
	uint16_t format;
	uint16_t channels;
	uint32_t srate;
	uint32_t byterate;
	uint16_t block_align;
	uint16_t bps;
	uint16_t extra;
};

int wav_header_encode(FILE *f, uint16_t format, uint16_t channels,
		      uint32_t srate, uint16_t bps, size_t bytes);
int wav_header_decode(struct wav_fmt *fmt, size_t *datasize, FILE *f);
