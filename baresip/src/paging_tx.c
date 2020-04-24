/**
 * @file
 *
 * Transmitter for RTP paging (multicast, unicast)
 */
#include <re.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <baresip.h>
#include "core.h"


#define DEBUG_MODULE "paging_tx"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

/** Magic number */
#define MAGIC 0x008AABBC
#include "magic.h"


paging_tx_h *handler = NULL;

struct paging_tx {
	MAGIC_DECL                /**< Magic number for debugging           */
	struct le le;             /**< Linked list element                  */
	struct sa target;
	char *codec_name;
	uint32_t srate;
	uint8_t ch;
	const char *fmtp;
	const char *params;
	const struct aucodec *codec;
	struct sdp_media *media;
	struct stream *str;
	struct audio *audio;      /**< Audio stream                         */
	time_t time_start;        /**< Time when call started               */
	time_t time_stop;         /**< Time when call stopped               */
	int stopping;             /**< Flag to avoid double paging_tx_stream_stop() */
	void *arg;                /**< Handler argument                     */
};


static void paging_tx_stream_stop(struct paging_tx *tx)
{
	if (!tx || tx->stopping)
		return;

	tx->stopping = 1;
	tx->time_stop = time(NULL);

	/* Audio */
	audio_stop(tx->audio);

	if (handler) {
    	handler(PAGING_TX_STOPPED);
	}
}


static void paging_tx_destructor(void *arg)
{
	struct paging_tx *tx = arg;

	re_printf("Terminating paging TX...\n");

	paging_tx_stream_stop(tx);
	mem_deref(tx->str);
	mem_deref(tx->audio);
	mem_deref(tx->media);
	mem_deref(tx->codec_name);
	(void)re_printf("Paging TX terminated\n");
}


static void paging_tx_error_handler(int err, const char *str, void *arg)
{
	struct paging_tx *tx = arg;
	MAGIC_CHECK(tx);

	if (err) {
		DEBUG_WARNING("Paging TX error: %m (%s)\n", err, str);
	}

	paging_tx_stream_stop(tx);
	// TODO: app event handler
}

static void audio_event_handler(int key, bool end, void *arg)
{
	struct paging_tx *tx = arg;
	MAGIC_CHECK(tx);

	(void)re_printf("received event: '%c' (end=%d)\n", key, end);
}


static void audio_error_handler(int err, const char *str, void *arg)
{
	struct paging_tx *tx = arg;
	MAGIC_CHECK(tx);

	if (err) {
		DEBUG_WARNING("Audio error: %m (%s)\n", err, str);
	}

	paging_tx_stream_stop(tx);
}


/**
 * Allocate a new object
 * \param addr destination address to transmit to
 * @return 0 if success, otherwise errorcode
 */
int paging_tx_alloc(struct paging_tx **txp, const struct config *cfg, struct sa *addr, const char* forced_src_mod, const char* forced_src_dev, const char* codec, unsigned int ptime)
{
	struct paging_tx *tx;
	int err = 0;
	struct pl pl_cname = PL_INIT, pl_srate = PL_INIT, pl_ch = PL_INIT;
	char cname[64];

	tx = mem_zalloc(sizeof(*tx), paging_tx_destructor);
	if (!tx)
		return ENOMEM;

	MAGIC_INIT(tx);

	tx->target = *addr;

	tx->srate = 8000;	// default; G722 also uses 8000 value for historical reasons
	tx->ch = 1;

	str_ncpy(cname, codec, sizeof(cname)); 

	/* Format: "codec/srate/ch" */
	if (0 == re_regex(cname, str_len(cname),
			  "[^/]+/[0-9]+[/]*[0-9]*",
			  &pl_cname, &pl_srate,
			  NULL, &pl_ch)) {
		(void)pl_strcpy(&pl_cname, cname,
				sizeof(cname));
		tx->srate = pl_u32(&pl_srate);
		if (pl_isset(&pl_ch))
			tx->ch = pl_u32(&pl_ch);
	}
	
	err = str_dup(&tx->codec_name, cname);
	if (err)
		goto out;

	err = sdp_media_alloc(&tx->media, NULL, addr);
	if (err)
		goto out;

	/* Audio stream */
	err = audio_alloc2(&tx->audio, cfg,
		forced_src_mod, forced_src_dev,
		tx->media, ptime,
		audio_event_handler, audio_error_handler, tx);
	if (err)
		goto out;

 out:
	if (err)
		mem_deref(tx);
	else if (txp) {
		*txp = tx;
		re_printf("PAGING_TX allocated, src_mod %s, src_dev %s\n", forced_src_mod?forced_src_mod:"null", forced_src_dev?forced_src_dev:"null");
	}

	return err;
}

int paging_tx_start(struct paging_tx *tx)
{
	int err;
	tx->codec = aucodec_find(tx->codec_name, tx->srate, tx->ch);
	if (tx->codec == NULL) {
		DEBUG_WARNING("Could not find codec %s/%d\n", tx->codec_name, tx->srate);
		return EINVAL;
    }
	if (tx->codec->pt) {
		int pt = atoi(tx->codec->pt);
		err  = audio_encoder_set(tx->audio, tx->codec, pt, tx->params);
		if (!err) {
			err = audio_start(tx->audio);
		}
		if (err) {
			DEBUG_WARNING("audio stream: %m\n", err);
		}
    } else {
		DEBUG_WARNING("Could not use dynamic PT codec for paging!\n");
		err = EINVAL;
    }
	tx->time_start = time(NULL);
	stream_reset(tx->str);
	return err;
}

int paging_tx_hangup(struct paging_tx *tx)
{
	int err = 0;

	if (!tx)
		return EINVAL;

	paging_tx_stream_stop(tx);

	return err;
}

int paging_tx_register_handler(paging_tx_h *pagingh)
{
	handler = pagingh;
	return 0;
}

struct audio *paging_audio(const struct paging_tx *paging)
{
	return paging ? paging->audio : NULL;
}




