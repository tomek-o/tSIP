/**
 * @file opus.c Opus Audio Codec
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <re.h>
#include <baresip.h>
#include <opus.h>
#include "opus_priv.h"


/**
 * @defgroup opus opus
 *
 * The OPUS audio codec
 *
 * Supported version: libopus 1.0.0 or later
 *
 * Configuration options:
 *
 \verbatim
  opus_stereo        yes     # Request peer to send stereo
  opus_sprop_stereo  yes     # Sending stereo
  opus_bitrate    128000     # Average bitrate in [bps]
  opus_cbr        {yes,no}   # Constant Bitrate (inverse of VBR)
  opus_inbandfec  {yes,no}   # Enable inband Forward Error Correction (FEC)
  opus_dtx        {yes,no}   # Enable Discontinuous Transmission (DTX)
  opus_complexity {0-10}     # Encoder's computational complexity (10 max)
  opus_application {audio, voip} # Encoder's intended application
  opus_packet_loss {0-100}   # Expected packet loss for FEC
 \endverbatim
 *
 * References:
 *
 *    RFC 6716  Definition of the Opus Audio Codec
 *    RFC 7587  RTP Payload Format for the Opus Speech and Audio Codec
 *
 *    http://opus-codec.org/downloads/
 */

#define DEBUG_MODULE "opus"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

static bool opus_mirror;
static char fmtp[256] = "";
static char fmtp_mirror[256];

uint32_t opus_complexity = 10;
opus_int32 opus_application = OPUS_APPLICATION_AUDIO;
opus_int32 opus_packet_loss = 0;


static int opus_fmtp_enc(struct mbuf *mb, const struct sdp_format *fmt,
			 bool offer, void *arg)
{
	bool mirror;

	(void)arg;
	(void)offer;

	if (!mb || !fmt)
		return 0;

	mirror = !offer && str_isset(fmtp_mirror);

	return mbuf_printf(mb, "a=fmtp:%s %s\r\n",
			   fmt->id, mirror ? fmtp_mirror : fmtp);
}

static struct aucodec opus = {
#if 0
	.name      = "opus",
	.srate     = 48000,
	.crate     = 48000,
	.ch        = 2,
	.fmtp      = fmtp,
	.encupdh   = opus_encode_update,
	.ench      = opus_encode_frm,
	.decupdh   = opus_decode_update,
	.dech      = opus_decode_frm,
	.plch      = opus_decode_pkloss,
	.fmtp_ench = NULL, //opus_fmtp_enc,
#else
	// BC does not accept initialization form as above
	LE_INIT,
	0,
	"opus", 48000, 2,
	fmtp,
	opus_encode_update, opus_encode_frm,
	opus_decode_update, opus_decode_frm,
	opus_decode_pkloss,
	NULL, //opus_fmtp_enc,
	NULL
#endif
};


void opus_mirror_params(const char *x)
{
	if (!opus_mirror)
		return;

	DEBUG_INFO("opus: mirror parameters: \"%s\"\n", x);

	str_ncpy(fmtp_mirror, x, sizeof(fmtp_mirror));
}


static int module_init(void)
{
	//struct conf *conf = conf_cur();
	const struct config *cfg = conf_config();
	uint32_t value;

	char *p;
	bool b;
	struct pl pl;
	int n = 0;

	memset(fmtp, 0, sizeof(fmtp));
	memset(fmtp_mirror, 0, sizeof(fmtp_mirror));	
	p = fmtp + str_len(fmtp);

	if (!cfg->opus.stereo || !cfg->opus.sprop_stereo)
		opus.ch = 1;

	/* always set stereo parameter first */
	n = re_snprintf(p, sizeof(fmtp) - str_len(p),
			"stereo=%d;sprop-stereo=%d", cfg->opus.stereo, cfg->opus.sprop_stereo);
	if (n <= 0)
		return ENOMEM;

	p += n;

	if (cfg->opus.set_bitrate) {
		n = re_snprintf(p, sizeof(fmtp) - str_len(p), ";maxaveragebitrate=%d", cfg->opus.bitrate);
		if (n <= 0)
			return ENOMEM;
		p += n;
	}
	if (cfg->opus.set_samplerate) {
        value = cfg->opus.samplerate;
		if ((value != 8000) && (value != 12000) && (value != 16000) &&
		    (value != 24000) && (value != 48000)) {
			DEBUG_WARNING("opus: invalid samplerate: %d\n", value);
			return EINVAL;
		}
		opus.srate = value;
	}

	if (cfg->opus.set_cbr) {

		n = re_snprintf(p, sizeof(fmtp) - str_len(p), ";cbr=%d", cfg->opus.cbr);
		if (n <= 0)
			return ENOMEM;

		p += n;
	}

	if (cfg->opus.set_inband_fec) {

		n = re_snprintf(p, sizeof(fmtp) - str_len(p),
				";useinbandfec=%d", (int)cfg->opus.inband_fec);
		if (n <= 0)
			return ENOMEM;

		p += n;
	}

	if (cfg->opus.set_dtx) {

		n = re_snprintf(p, sizeof(fmtp) - str_len(p),
				";usedtx=%d", (int)cfg->opus.dtx);
		if (n <= 0)
			return ENOMEM;

		p += n;
	}

	opus_mirror = cfg->opus.mirror;

	if (opus_mirror) {
		opus.fmtp = NULL;
		opus.fmtp_ench = opus_fmtp_enc;
	}

	opus_complexity = cfg->opus.complexity;

	if (opus_complexity > 10)
		opus_complexity = 10;

	if (cfg->opus.set_application) {
		if (cfg->opus.application == OPUS_APP_AUDIO)
			opus_application = OPUS_APPLICATION_AUDIO;
		else if (cfg->opus.application == OPUS_APP_VOIP)
			opus_application = OPUS_APPLICATION_VOIP;
		else {
			DEBUG_WARNING("opus: unknown encoder application: %r\n",
					&pl);
			return EINVAL;
		}
	}

	if (cfg->opus.set_packet_loss) {
        value = cfg->opus.packet_loss;
		if (value > 100)
			opus_packet_loss = 100;
		else
			opus_packet_loss = value;
	}

	DEBUG_INFO("opus: fmtp=\"%s\"\n", fmtp);

	aucodec_register(&opus);

	return 0;
}


static int module_close(void)
{
	aucodec_unregister(&opus);

	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(opus) = {
	"opus",
	"audio codec",
	module_init,
	module_close,
};
