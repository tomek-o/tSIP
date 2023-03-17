/**
 * @file au/fmt.c  Audio formats
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <rem_au.h>


/* Number of bytes per sample */
size_t aufmt_sample_size(enum aufmt fmt)
{
	switch (fmt) {

	case AUFMT_S16LE:   return sizeof(int16_t);
	case AUFMT_PCMA:    return 1;
	case AUFMT_PCMU:    return 1;
	default:            return 0;
	}
}


const char *aufmt_name(enum aufmt fmt)
{
	switch (fmt) {

	case AUFMT_S16LE:   return "S16LE";
	case AUFMT_PCMA:    return "PCMA";
	case AUFMT_PCMU:    return "PCMU";
	default:            return "???";
	}
}
