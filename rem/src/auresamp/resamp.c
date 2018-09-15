/**
 * @file resamp.c Audio Resampler
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <string.h>
#include <stdlib.h>
#include <rem_fir.h>
#include <rem_auresamp.h>


typedef void (resample_h)(struct auresamp *ar, int16_t *dst,
			  const int16_t *src, size_t nsamp_dst);


/** Defines an Audio resampler */
struct auresamp {
	struct fir fir;
	const int16_t *coeffv;
	int16_t *sampv;
	size_t sampc;
	int coeffn;
	double ratio;
	uint8_t ch_in;
	uint8_t ch_out;
	resample_h *resample;
};


/*
 * FIR filter with cutoff 8000Hz, samplerate 16000Hz
 */
static const int16_t fir_lowpass[31] = {
   -55,      0,     96,      0,   -220,      0,    461,      0,
  -877,      0,   1608,      0,  -3176,      0,  10342,  16410,
 10342,      0,  -3176,      0,   1608,      0,   -877,      0,
   461,      0,   -220,      0,     96,      0,    -55,
};


static inline void resample(struct auresamp *ar, int16_t *dst,
			    const int16_t *src, size_t nsamp_dst)
{
	double p = 0;

	while (nsamp_dst--) {

		*dst++ = src[(int)p];

		p += 1/ar->ratio;
	}
}


static inline void resample_mono2stereo(struct auresamp *ar, int16_t *dst,
					const int16_t *src, size_t nsamp_dst)
{
	double p = 0;

	while (nsamp_dst--) {

		const int i = (int)p;

		*dst++ = src[i];  /* Left channel */
		*dst++ = src[i];  /* Right channel */

		p += 1/ar->ratio;
	}
}


static inline void resample_stereo2mono(struct auresamp *ar, int16_t *dst,
					const int16_t *src, size_t nsamp_dst)
{
	double p = 0;

	while (nsamp_dst--) {

		const int i = ((int)p) & ~1;

		*dst++ = (src[i] + src[i+1]) / 2;

		p += 1/ar->ratio * 2;
	}
}


static inline void resample_stereo(struct auresamp *ar, int16_t *dst,
				   const int16_t *src, size_t nsamp_dst)
{
	double p = 0;

	while (nsamp_dst--) {

		const int i = ((int)p) & ~1;

		*dst++ = src[i];    /* Left channel */
		*dst++ = src[i+1];  /* Right channel */

		p += 1/ar->ratio * 2;
	}
}


static void auresamp_lowpass(struct auresamp *ar, int16_t *buf, size_t nsamp,
			     int channels)
{
	while (nsamp > 0) {

		size_t len = min(nsamp, FIR_MAX_INPUT_LEN);

		fir_process(&ar->fir, ar->coeffv, buf, buf, len, ar->coeffn,
			    channels);

		buf   += (len*channels);
		nsamp -= len;
	}
}


static void destructor(void *arg)
{
	struct auresamp *ar = arg;

	mem_deref(ar->sampv);
}


/**
 * Allocate a new Audio resampler
 *
 * @param arp       Pointer to allocated audio resampler
 * @param sampc_max Maximum number of source samples when downsampling
 * @param srate_in  Sample rate for the input in [Hz]
 * @param ch_in     Number of channels for the input
 * @param srate_out Sample rate for the output in [Hz]
 * @param ch_out    Number of channels for the output
 *
 * @return 0 for success, otherwise error code
 */
int auresamp_alloc(struct auresamp **arp, size_t sampc_max,
		   uint32_t srate_in, uint8_t ch_in,
		   uint32_t srate_out, uint8_t ch_out)
{
	struct auresamp *ar;
	int err = 0;

	if (!arp || !sampc_max || !srate_in || !srate_out)
		return EINVAL;

	ar = mem_zalloc(sizeof(*ar), destructor);
	if (!ar)
		return ENOMEM;

	ar->sampv = mem_zalloc(sampc_max * 2, NULL);
	if (!ar->sampv) {
		err = ENOMEM;
		goto out;
	}

	ar->sampc = sampc_max;
	ar->ratio = 1.0 * srate_out / srate_in;
	ar->ch_in = ch_in;
	ar->ch_out = ch_out;

	fir_init(&ar->fir);

	if (ch_in == 1 && ch_out == 1)
		ar->resample = resample;
	else if (ch_in == 1 && ch_out == 2)
		ar->resample = resample_mono2stereo;
	else if (ch_in == 2 && ch_out == 1)
		ar->resample = resample_stereo2mono;
	else if (ch_in == 2 && ch_out == 2)
		ar->resample = resample_stereo;
	else {
		err = EINVAL;
		goto out;
	}

	ar->coeffv = fir_lowpass;
	ar->coeffn = (int)ARRAY_SIZE(fir_lowpass);

 out:
	if (err)
		mem_deref(ar);
	else
		*arp = ar;

	return err;
}


/**
 * Resample PCM data
 *
 * @param ar        Audio resampler
 * @param dst_sampv Destination buffer for PCM data
 * @param dst_sampc Size of destination buffer/number of destination samples
 * @param src_sampv Source buffer with PCM data
 * @param src_sampc Number of source samples
 *
 * @return 0 for success, otherwise error code
 */
int auresamp_process(struct auresamp *ar,
		     int16_t *dst_sampv, size_t *dst_sampc,
		     const int16_t *src_sampv, size_t src_sampc)
{
	size_t ns, nd;

	if (!ar || !dst_sampv || !dst_sampc || !src_sampv)
		return EINVAL;

	ns = src_sampc / ar->ch_in;
	nd = (size_t)(ns * ar->ratio);

	if (*dst_sampc < nd * ar->ch_out)
		return ENOMEM;

	if (ar->ratio > 1) {

		ar->resample(ar, dst_sampv, src_sampv, nd);
		auresamp_lowpass(ar, dst_sampv, nd, ar->ch_out);
	}
	else if (ar->ratio < 1) {

		/* decimation: low-pass filter, then downsample */

		if (src_sampc > ar->sampc)
			return ENOMEM;

		memcpy(ar->sampv, src_sampv, src_sampc * 2);

		auresamp_lowpass(ar, ar->sampv, ns, ar->ch_in);
		ar->resample(ar, dst_sampv, ar->sampv, nd);
	}
	else {
		ar->resample(ar, dst_sampv, src_sampv, nd);
	}

	*dst_sampc = nd * ar->ch_out;

	return 0;
}
