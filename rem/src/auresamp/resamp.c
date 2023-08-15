/**
 * @file resamp.c Audio Resampler
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <string.h>
#include <re.h>
#include <rem_fir.h>
#include <rem_auresamp.h>


/* 48kHz sample-rate, 4kHz cutoff (pass 0-3kHz, stop 5-24kHz) */
static const int16_t fir_48_4[] = {
	 62,   -176,   -329,   -556,   -802,  -1005,  -1090,   -985,
       -636,    -23,    826,   1837,   2894,   3859,   4595,   4994,
       4994,   4595,   3859,   2894,   1837,    826,    -23,   -636,
       -985,  -1090,  -1005,   -802,   -556,   -329,   -176,     62
};

/* 48kHz sample-rate, 8kHz cutoff (pass 0-7kHz, stop 9-24kHz) */
static const int16_t fir_48_8[] = {
	238,    198,   -123,   -738,  -1268,  -1204,   -380,    714,
       1164,    376,  -1220,  -2206,  -1105,   2395,   6909,  10069,
      10069,   6909,   2395,  -1105,  -2206,  -1220,    376,   1164,
	714,   -380,  -1204,  -1268,   -738,   -123,    198,    238
};

/* 16kHz sample-rate, 4kHz cutoff and 32kHz sample-rate, 8 kHz cutoff */
static const int16_t fir_16_4[] = {
		22, 60, -41, -157, -9, 322, 195, -490, -613, 539, 1362, -229,
		-2657, -1101, 6031, 13167, 13167, 6031, -1101, -2657, -229,
		1362, 539, -613, -490, 195, 322, -9, -157, -41, 60, 22
};

static void upsample_mono2mono(int16_t *outv, const int16_t *inv,
			       size_t inc, unsigned ratio)
{
	unsigned i;

	while (inc >= 1) {

		for (i=0; i<ratio; i++)
			*outv++ = *inv;

		++inv;
		--inc;
	}
}


static void upsample_mono2stereo(int16_t *outv, const int16_t *inv,
				 size_t inc, unsigned ratio)
{
	unsigned i;

	ratio *= 2;

	while (inc >= 1) {

		for (i=0; i<ratio; i++)
			*outv++ = *inv;

		++inv;
		--inc;
	}
}


static void upsample_stereo2mono(int16_t *outv, const int16_t *inv,
				 size_t inc, unsigned ratio)
{
	unsigned i;

	while (inc >= 2) {

		const int16_t s = inv[0]/2 + inv[1]/2;

		for (i=0; i<ratio; i++)
			*outv++ = s;

		inv += 2;
		inc -= 2;
	}
}


static void upsample_stereo2stereo(int16_t *outv, const int16_t *inv,
				   size_t inc, unsigned ratio)
{
	unsigned i;

	while (inc >= 2) {

		for (i=0; i<ratio; i++) {
			*outv++ = inv[0];
			*outv++ = inv[1];
		}

		inv += 2;
		inc -= 2;
	}
}


static void downsample_mono2mono(int16_t *outv, const int16_t *inv,
				 size_t inc, unsigned ratio)
{
	while (inc >= ratio) {

		*outv++ = *inv;

		inv += ratio;
		inc -= ratio;
	}
}


static void downsample_mono2stereo(int16_t *outv, const int16_t *inv,
				   size_t inc, unsigned ratio)
{
	while (inc >= ratio) {

		*outv++ = *inv;
		*outv++ = *inv;

		inv += ratio;
		inc -= ratio;
	}
}


static void downsample_stereo2mono(int16_t *outv, const int16_t *inv,
				   size_t inc, unsigned ratio)
{
	ratio *= 2;

	while (inc >= ratio) {

		*outv++ = inv[0]/2 + inv[1]/2;

		inv += ratio;
		inc -= ratio;
	}
}


static void downsample_stereo2stereo(int16_t *outv, const int16_t *inv,
				     size_t inc, unsigned ratio)
{
	ratio *= 2;

	while (inc >= ratio) {

		*outv++ = inv[0];
		*outv++ = inv[1];

		inv += ratio;
		inc -= ratio;
	}
}


/**
 * Initialize a resampler object
 *
 * @param rs Resampler to initialize
 */
void auresamp_init(struct auresamp *rs)
{
	if (!rs)
		return;

	memset(rs, 0, sizeof(*rs));
	fir_reset(&rs->fir);
}


/**
 * Configure a resampler object
 *
 * @note The sample rate ratio must be an integer
 *
 * @param rs    Resampler
 * @param irate Input sample rate
 * @param ich   Input channel count
 * @param orate Output sample rate
 * @param och   Output channel count
 *
 * @return 0 if success, otherwise error code
 */
int auresamp_setup(struct auresamp *rs, uint32_t irate, unsigned ich,
		   uint32_t orate, unsigned och)
{
	if (!rs || !irate || !ich || !orate || !och)
		return EINVAL;

	if (orate == irate && och == ich) {
		auresamp_init(rs);
		return 0;
	}

	if (orate >= irate) {

		if (orate % irate)
			return ENOTSUP;

		if (ich == 1 && och == 1)
			rs->resample = upsample_mono2mono;
		else if (ich == 1 && och == 2)
			rs->resample = upsample_mono2stereo;
		else if (ich == 2 && och == 1)
			rs->resample = upsample_stereo2mono;
		else if (ich == 2 && och == 2)
			rs->resample = upsample_stereo2stereo;
		else
			return ENOTSUP;

		if (!rs->up || orate != rs->orate || och != rs->och)
			fir_reset(&rs->fir);

		rs->ratio = orate / irate;
		rs->up    = true;

		if (orate == irate) {
			rs->tapv = NULL;
			rs->tapc = 0;
		}
		else if (orate == 48000 && irate == 16000) {
			rs->tapv = fir_48_8;
			rs->tapc = RE_ARRAY_SIZE(fir_48_8);
		}
		else if ((orate == 16000 && irate == 8000) ||
                         (orate == 32000 && irate == 16000)) {
			rs->tapv = fir_16_4;
			rs->tapc = RE_ARRAY_SIZE(fir_16_4);
		}
		else {
			rs->tapv = fir_48_4;
			rs->tapc = RE_ARRAY_SIZE(fir_48_4);
		}
	}
	else {
		if (irate % orate)
			return ENOTSUP;

		if (ich == 1 && och == 1)
			rs->resample = downsample_mono2mono;
		else if (ich == 1 && och == 2)
			rs->resample = downsample_mono2stereo;
		else if (ich == 2 && och == 1)
			rs->resample = downsample_stereo2mono;
		else if (ich == 2 && och == 2)
			rs->resample = downsample_stereo2stereo;
		else
			return ENOTSUP;

		if (rs->up || irate != rs->irate || ich != rs->ich)
			fir_reset(&rs->fir);

		rs->ratio = irate / orate;
		rs->up    = false;

		if (irate == 48000 && orate == 16000) {
			rs->tapv = fir_48_8;
			rs->tapc = RE_ARRAY_SIZE(fir_48_8);
		}
		else if ((irate == 16000 && orate == 8000) ||
                         (irate == 32000 && orate == 16000)) {
			rs->tapv = fir_16_4;
			rs->tapc = RE_ARRAY_SIZE(fir_16_4);
		}
		else {
			rs->tapv = fir_48_4;
			rs->tapc = RE_ARRAY_SIZE(fir_48_4);
		}
	}

	rs->orate = orate;
	rs->och   = och;
	rs->irate = irate;
	rs->ich   = ich;

	return 0;
}


/**
 * Resample
 *
 * @note When downsampling, the input count must be divisible by rate ratio
 *
 * @param rs   Resampler
 * @param outv Output samples
 * @param outc Output sample count (in/out)
 * @param inv  Input samples
 * @param inc  Input sample count
 *
 * @return 0 if success, otherwise error code
 */
int auresamp(struct auresamp *rs, int16_t *outv, size_t *outc,
	     const int16_t *inv, size_t inc)
{
	size_t incc, outcc;

	if (!rs || !rs->resample || !outv || !outc || !inv)
		return EINVAL;

	incc = inc / rs->ich;

	if (rs->up) {
		outcc = incc * rs->ratio;

		if (*outc < outcc * rs->och)
			return ENOMEM;

		rs->resample(outv, inv, inc, rs->ratio);

		*outc = outcc * rs->och;

		if (rs->tapv)
			fir_filter(&rs->fir, outv, outv, *outc, rs->och,
				   rs->tapv, rs->tapc);
	}
	else {
		outcc = incc / rs->ratio;

		if (*outc < outcc * rs->och || *outc < inc)
			return ENOMEM;

		fir_filter(&rs->fir, outv, inv, inc, rs->ich,
			   rs->tapv, rs->tapc);

		rs->resample(outv, outv, inc, rs->ratio);

		*outc = outcc * rs->och;
	}

	return 0;
}
