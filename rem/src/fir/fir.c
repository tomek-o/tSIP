/**
 * @file fir.c FIR -- Finite Impulse Response
 *
 * Copyright (C) 2010 Creytiv.com
 */
 
#include <re.h>
#include <string.h>
#include <stdlib.h>
#include <rem_fir.h>


/*
 * FIR -- Finite Impulse Response
 *
 * Inspiration:
 *
 *     http://sestevenson.files.wordpress.com/2009/10/firfixed.pdf
 */


/**
 * Initialize the FIR-filter
 *
 * @param fir FIR-filter state
 */
void fir_init(struct fir *fir)
{
	memset(fir->insamp, 0, sizeof(fir->insamp));
}


/**
 * Process PCM samples with the FIR filter
 *
 * @param fir          FIR filter
 * @param coeffs       FIR Coefficients to use
 * @param input        Input PCM samples
 * @param output       Output PCM samples
 * @param length       Number of samples
 * @param filterLength Number of coefficients
 * @param channels     Number of channels
 */
void fir_process(struct fir *fir, const int16_t *coeffs,
		 const int16_t *input, int16_t *output,
		 size_t length, int filterLength, uint8_t channels)
{
	const int16_t *inputx = input;
	int32_t acc;
	const int16_t *coeffp;
	int16_t *inputp;
	size_t n;
	int k;
	int ch;

	/* put the new samples at the high end of the buffer */
	for (n = 0; n < length; n++) {

		for (ch = 0; ch < channels; ch++) {

			fir->insamp[ch][filterLength - 1 + n] = *inputx++;
		}
	}

	for (ch = 0; ch < channels; ch++) {

		/* apply the filter to each input sample */
		for ( n = 0; n < length; n++ ) {

			coeffp = coeffs;
			inputp = &fir->insamp[ch][filterLength - 1 + n];

			/* load rounding constant */
			acc = 1 << 14;

			/* perform the multiply-accumulate */
			for ( k = 0; k < filterLength; k++ ) {
				acc += (int32_t)(*coeffp++) *
					(int32_t)(*inputp--);
			}

			/* saturate the result */
			if ( acc > 0x3fffffff ) {
				acc = 0x3fffffff;
			}
			else if ( acc < -0x40000000 ) {
				acc = -0x40000000;
			}

			/* convert from Q30 to Q15 */
			output[channels*n + ch] = (int16_t)(acc >> 15);
		}
	}

	/* shift input samples back in time for next time */
	for (ch = 0; ch < channels; ch++) {
		memmove( &fir->insamp[ch][0], &fir->insamp[ch][length],
			 (filterLength - 1) * sizeof(int16_t) );
	}
}
