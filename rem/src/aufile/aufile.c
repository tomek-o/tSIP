/**
 * @file aufile.c  Audio File interface
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <string.h>
#include <re.h>
#include <rem_au.h>
#include <rem_aufile.h>
#include "aufile.h"


/** Audio file state */
struct aufile {
	struct aufile_prm prm;
	enum aufile_mode mode;
	size_t datasize;
	size_t nread;
	size_t nwritten;
	FILE *f;
};


static int wavfmt_to_aufmt(enum wavfmt fmt, uint16_t bps)
{
	switch (fmt) {

	case WAVE_FMT_PCM:
		if (bps != 16)
			return -1;

		return AUFMT_S16LE;

	case WAVE_FMT_ALAW:
		if (bps != 8)
			return -1;

		return AUFMT_PCMA;

	case WAVE_FMT_ULAW:
		if (bps != 8)
			return -1;

		return AUFMT_PCMU;

	default:
		return -1;
	}
}


static enum wavfmt aufmt_to_wavfmt(enum aufmt fmt)
{
	switch (fmt) {

	case AUFMT_S16LE:  return WAVE_FMT_PCM;
	case AUFMT_PCMA:   return WAVE_FMT_ALAW;
	case AUFMT_PCMU:   return WAVE_FMT_ULAW;
	default:           return -1;
	}
}


static uint16_t aufmt_to_bps(enum aufmt fmt)
{
	switch (fmt) {

	case AUFMT_S16LE: return 16;
	case AUFMT_PCMA:  return 8;
	case AUFMT_PCMU:  return 8;
	default:          return 0;
	}
}


static void destructor(void *arg)
{
	struct aufile *af = arg;

	if (!af->f)
		return;

	/* Update WAV header in write-mode */
	if (af->mode == AUFILE_WRITE && af->nwritten > 0) {

		rewind(af->f);

		(void)wav_header_encode(af->f, aufmt_to_wavfmt(af->prm.fmt),
					af->prm.channels, af->prm.srate,
					aufmt_to_bps(af->prm.fmt),
					af->nwritten);
	}

	(void)fclose(af->f);
}


/**
 * Open a WAVE file for reading or writing
 *
 * Supported formats:  16-bit PCM, A-law, U-law
 *
 * @param afp       Pointer to allocated Audio file
 * @param prm       Audio format of the file
 * @param filename  Filename of the WAV-file to load
 * @param mode      Read or write mode
 *
 * @return 0 if success, otherwise errorcode
 */
int aufile_open(struct aufile **afp, struct aufile_prm *prm,
		const char *filename, enum aufile_mode mode)
{
	struct wav_fmt fmt;
	struct aufile *af;
	int aufmt;
	int err;

	if (!afp || !filename || (mode == AUFILE_WRITE && !prm))
		return EINVAL;

	af = mem_zalloc(sizeof(*af), destructor);
	if (!af)
		return ENOMEM;

	af->mode = mode;

	af->f = fopen(filename, mode == AUFILE_READ ? "rb" : "wb");
	if (!af->f) {
		err = errno;
		goto out;
	}

	switch (mode) {

	case AUFILE_READ:
		err = wav_header_decode(&fmt, &af->datasize, af->f);
		if (err)
			goto out;

		aufmt = wavfmt_to_aufmt(fmt.format, fmt.bps);
		if (aufmt < 0) {
			err = ENOSYS;
			goto out;
		}

		if (prm) {
			prm->srate    = fmt.srate;
			prm->channels = (uint8_t)fmt.channels;
			prm->fmt      = aufmt;
		}
		break;

	case AUFILE_WRITE:
		af->prm = *prm;

		err = wav_header_encode(af->f, aufmt_to_wavfmt(prm->fmt),
					prm->channels, prm->srate,
					aufmt_to_bps(prm->fmt), 0);
		break;

	default:
		err = ENOSYS;
		break;
	}

 out:
	if (err)
		mem_deref(af);
	else
		*afp = af;

	return err;
}


/**
 * Read PCM-samples from a WAV file
 *
 * @param af  Audio-file
 * @param p   Read buffer
 * @param sz  Size of buffer, on return contains actual read
 *
 * @return 0 if success, otherwise errorcode
 */
int aufile_read(struct aufile *af, uint8_t *p, size_t *sz)
{
	size_t n;

	if (!af || !p || !sz || af->mode != AUFILE_READ)
		return EINVAL;

	if (af->nread >= af->datasize) {
		*sz = 0;
		return 0;
	}

	n = min(*sz, af->datasize - af->nread);

	n = fread(p, 1, n, af->f);
	if (ferror(af->f))
		return errno;

	*sz = n;
	af->nread += n;

	return 0;
}


/**
 * Write PCM-samples to a WAV file
 *
 * @param af  Audio-file
 * @param p   Write buffer
 * @param sz  Size of buffer
 *
 * @return 0 if success, otherwise errorcode
 */
int aufile_write(struct aufile *af, const uint8_t *p, size_t sz)
{
	if (!af || !p || !sz || af->mode != AUFILE_WRITE)
		return EINVAL;

	if (1 != fwrite(p, sz, 1, af->f))
		return ferror(af->f);

	af->nwritten += sz;

	return 0;
}
