/**
 * @file util.c  Audio utility functions
 *
 * Copyright (C) 2022 Commend.com - c.spielberger@commend.com
 */

#include <re.h>
#include <rem_au.h>


/**
 * Calculate number of samples from sample rate, channels and packet time
 *
 * @param srate    Sample rate in [Hz]
 * @param channels Number of channels
 * @param ptime    Packet time in [ms]
 *
 * @return Number of samples
 */
uint32_t calc_nsamp(uint32_t srate, uint8_t channels, uint16_t ptime)
{
	return srate * channels * ptime / 1000;
}
