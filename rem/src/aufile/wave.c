/**
 * @file wave.c  WAVE format encoding and decoding
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <string.h>
#include <re.h>
#include <rem_au.h>
#include <rem_aufile.h>
#include "aufile.h"


enum {
	WAVE_FMT_SIZE = 16
};


/** WAV-file chunk */
struct wav_chunk {
	uint8_t id[4];
	uint32_t size;
};


static int write_u16(FILE *f, uint16_t v)
{
	v = sys_htols(v);

	if (1 != fwrite(&v, sizeof(v), 1, f))
		return ferror(f);

	return 0;
}


static int write_u32(FILE *f, uint32_t v)
{
	v = sys_htoll(v);

	if (1 != fwrite(&v, sizeof(v), 1, f))
		return ferror(f);

	return 0;
}


static int read_u16(FILE *f, uint16_t *v)
{
	uint16_t vle;

	if (1 != fread(&vle, sizeof(vle), 1, f))
		return ferror(f);

	*v = sys_ltohs(vle);

	return 0;
}


static int read_u32(FILE *f, uint32_t *v)
{
	uint32_t vle;

	if (1 != fread(&vle, sizeof(vle), 1, f))
		return ferror(f);

	*v = sys_ltohl(vle);

	return 0;

}


static int chunk_encode(FILE *f, const char *id, size_t sz)
{
	if (1 != fwrite(id, 4, 1, f))
		return ferror(f);

	return write_u32(f, (uint32_t)sz);
}


static int chunk_decode(struct wav_chunk *chunk, FILE *f)
{
	if (1 != fread(chunk->id, sizeof(chunk->id), 1, f))
		return ferror(f);

	return read_u32(f, &chunk->size);
}


int wav_header_encode(FILE *f, uint16_t format, uint16_t channels,
		      uint32_t srate, uint16_t bps, size_t bytes)
{
	int err;

	err = chunk_encode(f, "RIFF", 36 + bytes);
	if (err)
		return err;

	if (1 != fwrite("WAVE", 4, 1, f))
		return ferror(f);

	err = chunk_encode(f, "fmt ", WAVE_FMT_SIZE);
	if (err)
		return err;

	err  = write_u16(f, format);
	err |= write_u16(f, channels);
	err |= write_u32(f, srate);
	err |= write_u32(f, srate * channels * bps / 8);
	err |= write_u16(f, channels * bps / 8);
	err |= write_u16(f, bps);
	if (err)
		return err;

	return chunk_encode(f, "data", bytes);
}


int wav_header_decode(struct wav_fmt *fmt, size_t *datasize, FILE *f)
{
	struct wav_chunk header, format, chunk;
	uint8_t rifftype[4];        /* "WAVE" */
	int err = 0;

	err = chunk_decode(&header, f);
	if (err)
		return err;

	if (memcmp(header.id, "RIFF", 4)) {
		(void)re_printf("aufile: expected RIFF (%b)\n",
				 header.id, sizeof(header.id));
		return EBADMSG;
	}

	if (1 != fread(rifftype, sizeof(rifftype), 1, f))
		return ferror(f);

	if (memcmp(rifftype, "WAVE", 4)) {
		(void)re_printf("aufile: expected WAVE (%b)\n",
				 rifftype, sizeof(rifftype));
		return EBADMSG;
	}

	err = chunk_decode(&format, f);
	if (err)
		return err;

	if (memcmp(format.id, "fmt ", 4)) {
		(void)re_printf("aufile: expected fmt (%b)\n",
				 format.id, sizeof(format.id));
		return EBADMSG;
	}

	if (format.size < WAVE_FMT_SIZE)
		return EBADMSG;

	err  = read_u16(f, &fmt->format);
	err |= read_u16(f, &fmt->channels);
	err |= read_u32(f, &fmt->srate);
	err |= read_u32(f, &fmt->byterate);
	err |= read_u16(f, &fmt->block_align);
	err |= read_u16(f, &fmt->bps);
	if (err)
		return err;

	/* skip any extra bytes */
	if (format.size >= (WAVE_FMT_SIZE + 2)) {

		err = read_u16(f, &fmt->extra);
		if (err)
			return err;

		if (fmt->extra > 0) {
			if (fseek(f, fmt->extra, SEEK_CUR))
				return errno;
		}
	}

	/* fast forward to "data" chunk */
	for (;;) {

		err = chunk_decode(&chunk, f);
		if (err)
			return err;

		if (chunk.size > header.size) {
			(void)re_printf("chunk size too large"
					 " (%u > %u)\n",
					 chunk.size, header.size);
			return EBADMSG;
		}

		if (0 == memcmp(chunk.id, "data", 4)) {
			*datasize = chunk.size;
			break;
		}

		if (fseek(f, chunk.size, SEEK_CUR) < 0)
			return errno;
	}

	return 0;
}
