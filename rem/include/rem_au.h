/**
 * @file rem_au.h Basic audio types
 *
 * Copyright (C) 2010 Creytiv.com
 */

#ifndef REM_AU_H
#define REM_AU_H

/** Audio formats */
enum aufmt {
	AUFMT_S16LE,  /**< Signed 16-bit PCM */
	AUFMT_PCMA,   /**< G.711 A-law       */
	AUFMT_PCMU,   /**< G.711 U-law       */
};

size_t      aufmt_sample_size(enum aufmt fmt);
const char *aufmt_name(enum aufmt fmt);

uint32_t calc_nsamp(uint32_t srate, uint8_t channels, uint16_t ptime);


#endif

