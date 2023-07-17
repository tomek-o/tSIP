/**
 * @file fir.c FIR -- Finite Impulse Response
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <string.h>
#include <re.h>
#include <rem_fir.h>


/**
 * Reset the FIR-filter
 *
 * @param fir FIR-filter state
 */
void fir_reset(struct fir *fir)
{
	if (!fir)
		return;

	memset(fir, 0, sizeof(*fir));
}


/**
 * Process samples with the FIR filter
 *
 * @note product of channel and tap-count must be power of two
 *
 * @param fir  FIR filter
 * @param outv Output samples
 * @param inv  Input samples
 * @param inc  Number of samples
 * @param ch   Number of channels
 * @param tapv Filter taps
 * @param tapc Number of taps
 */
void fir_filter(struct fir *fir, int16_t *outv, const int16_t *inv, size_t inc,
		unsigned ch, const int16_t *tapv, size_t tapc)
{
	const unsigned hmask = (ch * (unsigned)tapc) - 1;

	if (!fir || !outv || !inv || !ch || !tapv || !tapc)
		return;

	if (hmask >= RE_ARRAY_SIZE(fir->history) || hmask & (hmask+1))
		return;

	while (inc--) {

		int64_t acc = 0;
		unsigned i, j;

		fir->history[fir->index & hmask] = *inv++;

		for (i=0, j=fir->index++; i<tapc; ++i, j-=ch)
			acc += (int64_t)fir->history[j & hmask] * tapv[i];

		if (acc > 0x3fffffff)
			acc = 0x3fffffff;
		else if (acc < -0x40000000)
			acc = -0x40000000;

		*outv++ = (int16_t)(acc>>15);
	}
}
