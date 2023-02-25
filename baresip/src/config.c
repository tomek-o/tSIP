/**
 * @file config.c  Core Configuration
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include "core.h"


#define DEBUG_MODULE "config"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


#undef MOD_PRE
#define MOD_PRE ""  /**< Module prefix */


/** Core Run-time Configuration - populated from config file */
/* todo: move parsing/decoding to a module */
static struct config core_config = {
	/** SIP User-Agent */
	{
		16,
		"",
		"",
		"",
		"",
		"",
		0,
		SIP_TRANSP_UDP,
		true,
		false
	},

	/** Audio */
	{
		"","",
		"","",
		"","",
		"","",
		{8000, 48000},
		{1, 2},
		0,
		0,
		false,
		AUDIO_MODE_POLL,
		128, 128,					// softvol tx, rx, \sa SOFTVOL_BASE
		{ false, 12000, 6.0f, 0.01f, 0.01f },	// agc_rx: enabled, target, max gain, attack rate, release rate
		{ false, 300, 1000, 50, 1000 },		// gate_tx: enabled, threshold_close, hold_ms, attack time, release time 
		0.1, 0.1,					// portaudio suggested latency: in, out
        false						// loop_ring_without_silence
	},

#ifdef USE_VIDEO
	/** Video */
	{
		"dshow", "",						// src dev/mod
		"sdl", "",							// disp dev/mod
		352, 288,
		512000,
		25,
		false,
		VID_FMT_UYVY422,
		/* selfview */
		{
			true,	/* enabled */
			true,	/* pip */
		},
		/* dshow */
		{
        	false	/* skip reading back media type */
		}
	},
#endif

	/** Audio/Video Transport */
	{
		0xb8,
		{1024, 49152},
		{512000, 1024000},
		true,
		false,
		{5, 10},
		0
	},

	/* recording */
	{
		false
	},

	/* tx preprocessor */
	{
		false,
		true,
		false,
		false,
		false,
		8000
	},

	AEC_WEBRTC,

	/* WebRTC AEC */
	{
		120,
		0
	},

	/* Opus */
	{
		true,
		true,
		false, 64000,	/* bitrate */
		false, 48000,	/* samplerate */
		false, false,	/* cbr */
		false, false,	/* inband_fec */
		false, false,	/* dtx */
		false,	/* mirror */
		10,		/* complexity */
		false, OPUS_APP_AUDIO,
		false, 10		/* packet_loss */
	},

	/* ZRTP */
	{
		false,
		false,
		{0}				/* zid_filename */
	},

	/* Network */
	{
		""
	},

#ifdef USE_VIDEO
	/* BFCP */
	{
		""
	},
#endif

	/* messages */
	{
		200, "OK",
		0 /* do_not_reply */
	},
};

static int dns_server_handler(const struct pl *pl, void *arg)
{
	struct sa sa;
	int err;

	(void)arg;

	err = sa_decode(&sa, pl->p, pl->l);
	if (err) {
		DEBUG_WARNING("dns_server: could not decode `%r'\n", pl);
		return err;
	}

	err = net_dnssrv_add(&sa);
	if (err) {
		DEBUG_WARNING("failed to add nameserver %r: %m\n", pl, err);
	}

	return err;
}

/**
 * Get the baresip core config
 *
 * @return Core config
 */
struct config *conf_config(void)
{
	return &core_config;
}

void u32mask_enable(uint32_t *mask, uint8_t bit, bool enable)
{
	if (!mask)
		return;

	if (enable)
		*mask |=  (1u << bit);
	else
		*mask &= ~(1u << bit);
}


bool u32mask_enabled(uint32_t mask, uint8_t bit)
{
	return 0 != (mask & (1u << bit));
}

