/**
 * @file src/audio.c  Audio stream
 *
 * Copyright (C) 2010 Creytiv.com
 * \ref GenericAudioStream
 */
#define _BSD_SOURCE 1
#include <re.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif
#include <rem.h>
#include <baresip.h>
#include "core.h"
#include "dtmf.h"
#include "tone_generator.h"


#define DEBUG_MODULE "audio"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

/** Magic number */
#define MAGIC 0x000a0d10
#include "magic.h"


/**
 * \page GenericAudioStream Generic Audio Stream
 *
 * Implements a generic audio stream. The application can allocate multiple
 * instances of a audio stream, mapping it to a particular SDP media line.
 * The audio object has a DSP sound card sink and source, and an audio encoder
 * and decoder. A particular audio object is mapped to a generic media
 * stream object. Each audio channel has an optional audio filtering chain.
 *
 *<pre>
 *            write  read
 *              |    /|\
 *             \|/    |
 * .------.   .---------.    .-------.
 * |filter|<--|  audio  |--->|encoder|
 * '------'   |         |    |-------|
 *            | object  |--->|decoder|
 *            '---------'    '-------'
 *              |    /|\
 *              |     |
 *             \|/    |
 *         .------. .-----.
 *         |auplay| |ausrc|
 *         '------' '-----'
 *</pre>
 */

enum {
	AUDIO_SAMPSZ    = 3*1920  /* Max samples, 48000Hz 2ch at 60ms */
};


/**
 * Audio transmit/encoder
 *
 *
 \verbatim

 Processing encoder pipeline:

 .    .-------.   .-------.   .--------.   .------.   .--------.   .--------.
 |    |       |   |       |   |        |   |      |   |        |   |        |
 |O-->| ausrc |-->| aubuf |-->| resamp |-->| DTMF |-->| aufilt |-->| encode |---> RTP
 |    |       |   |       |   |        |   | ins. |   |        |   |        |
 '    '-------'   '-------'   '--------'   '------'   '--------'   '--------'

 	clocked by: ausrc
 \endverbatim
 *
 */
struct autx {
	struct ausrc_st *ausrc;       /**< Audio Source                    */
	struct ausrc_prm ausrc_prm;
	const struct aucodec *ac;     /**< Current audio encoder           */
	struct auenc_state *enc;      /**< Audio encoder state (optional)  */
	struct aubuf *ab;             /**< Packetize outgoing stream       */
	struct auresamp *resamp;      /**< Optional resampler for DSP      */
	struct list filtl;            /**< Audio filters in encoding order */
	struct mbuf *mb;              /**< Buffer for outgoing RTP packets */
    char mod[32];
	char device[128];
	int16_t *sampv;               /**< Sample buffer                   */
	int16_t *sampv_rs;            /**< Sample buffer for resampler     */
	uint32_t ptime;               /**< Packet time for sending         */
	uint32_t ts;                  /**< Timestamp for outgoing RTP      */
	uint32_t ts_tel;              /**< Timestamp for Telephony Events  */
	size_t psize;                 /**< Packet size for sending         */
	bool marker;                  /**< Marker bit for outgoing RTP     */
	bool is_g722;                 /**< Set if encoder is G.722 codec   */
	bool muted;                   /**< Audio source is muted           */
	int cur_key;                  /**< Currently transmitted event     */

	union {
		struct tmr tmr;       /**< Timer for sending RTP packets   */
#ifdef HAVE_PTHREAD
		struct {
			pthread_t tid;/**< Audio transmit thread           */
			bool run;     /**< Audio transmit thread running   */
		} thr;
#endif
	} u;
	struct dtmf_generator dtmfgen;	/**< DTMF generator state	    */
	struct tone_generator tonegen;	/**< tone generator state       */
	struct ausrc_st *ausrc_extra;
};


/**
 * Audio receive/decoder
 *
 \verbatim

 Processing decoder pipeline:

       .--------.   .-------.   .--------.   .--------.   .--------.
 |\    |        |   |       |   |        |   |        |   |        |
 | |<--| auplay |<--| aubuf |<--| resamp |<--| aufilt |<--| decode |<--- RTP
 |/    |        |   |       |   |        |   |        |   |        |
       '--------'   '-------'   '--------'   '--------'   '--------'

 \endverbatim
 */
struct aurx {
	struct auplay_st *auplay;     /**< Audio Player                    */
	struct auplay_prm auplay_prm; /**< Audio Player parameters         */
	const struct aucodec *ac;     /**< Current audio decoder           */
	struct audec_state *dec;      /**< Audio decoder state (optional)  */
	struct aubuf *ab;             /**< Incoming audio buffer           */
	struct auresamp *resamp;      /**< Optional resampler for DSP      */
	struct list filtl;            /**< Audio filters in decoding order */
    char mod[32];
	char device[128];
	int16_t *sampv;               /**< Sample buffer                   */
	int16_t *sampv_rs;            /**< Sample buffer for resampler     */
	uint32_t ptime;               /**< Packet time for receiving       */
	int pt;                       /**< Payload type for incoming RTP   */
	int pt_tel;                   /**< Event payload type - receive    */
};


/** Generic Audio stream */
struct audio {
	MAGIC_DECL                    /**< Magic number for debugging      */
	struct autx tx;               /**< Transmit                        */
	struct aurx rx;               /**< Receive                         */
	struct stream *strm;          /**< Generic media stream            */
	struct telev *telev;          /**< Telephony events                */
	struct config_audio cfg;      /**< Audio configuration             */
	bool started;                 /**< Stream is started flag          */
	audio_event_h *eventh;        /**< Event handler                   */
	audio_err_h *errh;            /**< Audio error handler             */
	void *arg;                    /**< Handler argument                */
};


static void audio_destructor(void *arg)
{
	struct audio *a = arg;

	audio_stop(a);

	mem_deref(a->tx.enc);
	mem_deref(a->rx.dec);
	mem_deref(a->tx.ab);
	mem_deref(a->tx.mb);
	mem_deref(a->tx.sampv);
	mem_deref(a->rx.sampv);
	mem_deref(a->rx.ab);
	mem_deref(a->tx.sampv_rs);
	mem_deref(a->tx.resamp);
	mem_deref(a->rx.sampv_rs);
	mem_deref(a->rx.resamp);

	list_flush(&a->tx.filtl);
	list_flush(&a->rx.filtl);

	mem_deref(a->strm);
	mem_deref(a->telev);
}


/**
 * Calculate number of samples from sample rate, channels and packet time
 *
 * @param srate    Sample rate in [Hz]
 * @param channels Number of channels
 * @param ptime    Packet time in [ms]
 *
 * @return Number of samples
 */
static inline uint32_t calc_nsamp(uint32_t srate, uint8_t channels,
				  uint16_t ptime)
{
	return srate * channels * ptime / 1000;
}


/**
 * Get the DSP samplerate for an audio-codec (exception for G.722)
 */
static inline uint32_t get_srate(const struct aucodec *ac)
{
	if (!ac)
		return 0;

	return !str_casecmp(ac->name, "G722") ? 16000 : ac->srate;
}


static inline uint32_t get_framesize(const struct aucodec *ac,
				     uint32_t ptime)
{
	if (!ac)
		return 0;

	return calc_nsamp(get_srate(ac), ac->ch, ptime);
}


static bool aucodec_equal(const struct aucodec *a, const struct aucodec *b)
{
	if (!a || !b)
		return false;

	return get_srate(a) == get_srate(b) && a->ch == b->ch;
}

static inline bool in_range(const struct range *rng, uint32_t val)
{
	return rng ? (val >= rng->min && val <= rng->max) : false;
}

static int add_audio_codec(struct audio *a, struct sdp_media *m,
			   struct aucodec *ac)
{
	if (!in_range(&a->cfg.srate, ac->srate)) {
		DEBUG_INFO("skip codec with %uHz (audio range %uHz - %uHz)\n",
			   ac->srate, a->cfg.srate.min, a->cfg.srate.max);
		return 0;
	}

	if (!in_range(&a->cfg.channels, ac->ch)) {
		DEBUG_INFO("skip codec with %uch (audio range %uch-%uch)\n",
			   ac->ch, a->cfg.channels.min, a->cfg.channels.max);
		return 0;
	}

	return sdp_format_add(NULL, m, false, ac->pt, ac->name, ac->srate,
			      ac->ch, ac->fmtp_ench, ac->fmtp_cmph, ac, false,
			      "%s", ac->fmtp);
}


/**
 * Encoder audio and send via stream
 *
 * @note This function has REAL-TIME properties
 *
 * @param a     Audio object
 * @param tx    Audio transmit object
 * @param sampv Audio samples
 * @param sampc Number of audio samples
 */
static void encode_rtp_send(struct audio *a, struct autx *tx,
			    int16_t *sampv, size_t sampc)
{
	size_t len;
	int err;

	if (!tx->ac)
		return;

	tx->mb->pos = tx->mb->end = STREAM_PRESZ;
	len = mbuf_get_space(tx->mb);

	err = tx->ac->ench(tx->enc, mbuf_buf(tx->mb), &len, sampv, sampc);
	if (err) {
		DEBUG_WARNING("%s encode error: %d samples (%m)\n",
			      tx->ac->name, sampc, err);
		goto out;
	}

	tx->mb->pos = STREAM_PRESZ;
	tx->mb->end = STREAM_PRESZ + len;

	if (mbuf_get_left(tx->mb)) {

		err = stream_send(a->strm, tx->marker, -1, tx->ts, tx->mb);
		if (err)
			goto out;
	}

	tx->ts += (uint32_t)(tx->is_g722 ? sampc/2 : sampc);

 out:
	tx->marker = false;
}


/*
 * @note This function has REAL-TIME properties
 */
static void poll_aubuf_tx(struct audio *a)
{
	struct autx *tx = &a->tx;
	int16_t *sampv = tx->sampv;
	size_t sampc;
	struct le *le;
	int err = 0;

	sampc = tx->psize / 2;

	/* timed read from audio-buffer */
	if (aubuf_get_samp(tx->ab, tx->ptime, tx->sampv, sampc))
		return;

	/* optional resampler */
	if (tx->resamp) {
		size_t sampc_rs = AUDIO_SAMPSZ;

		err = auresamp_process(tx->resamp,
				       tx->sampv_rs, &sampc_rs,
				       tx->sampv, sampc);
		if (err)
			return;

		sampv = tx->sampv_rs;
		sampc = sampc_rs;
	}

	/* check if we should generate DTMF audio */
	if (!dtmf_is_empty(&tx->dtmfgen)) {
		size_t x;

		for (x = 0; x != sampc; x++) {
			if (dtmf_get_sample(&tx->dtmfgen, (int16_t *)sampv + x))
				break;
		}
	}

	tone_generator_process(&tx->tonegen, tx->ausrc_prm.srate, sampv, sampc);

	/* Process exactly one audio-frame in list order */
	for (le = tx->filtl.head; le; le = le->next) {
		struct aufilt_enc_st *st = le->data;

		if (st->af && st->af->ench)
			err |= st->af->ench(st, sampv, &sampc);
	}

	/* Encode and send */
	encode_rtp_send(a, tx, sampv, sampc);
}


static void check_telev(struct audio *a, struct autx *tx)
{
	const struct sdp_format *fmt;
	bool marker = false;
	int err;

	tx->mb->pos = tx->mb->end = STREAM_PRESZ;

	err = telev_poll(a->telev, &marker, tx->mb);
	if (err)
		return;

	if (marker)
		tx->ts_tel = tx->ts;

	fmt = sdp_media_rformat(stream_sdpmedia(audio_strm(a)), telev_rtpfmt);
	if (!fmt)
		return;

	tx->mb->pos = STREAM_PRESZ;
	err = stream_send(a->strm, marker, fmt->pt, tx->ts_tel, tx->mb);
	if (err) {
		DEBUG_WARNING("telev: stream_send %m\n", err);
	}
}


/**
 * Write samples to Audio Player.
 *
 * @note This function has REAL-TIME properties
 *
 * @note The application is responsible for filling in silence in
 *       the case of underrun
 *
 * @note This function may be called from any thread
 *
 * @return true for valid audio samples, false for silence
 */
static bool auplay_write_handler(uint8_t *buf, size_t sz, void *arg)
{
	struct audio *a = arg;
	struct aurx *rx = &a->rx;

#if 0
	{
		char buf[64];
		/* debugging exact timing of read handler calls */
		(void)re_printf("[%s] auplay %u\n", sys_time(buf, sizeof(buf)), sz);
	}
#endif

	aubuf_read(rx->ab, buf, sz);

	return true;
}


/**
 * Read samples from Audio Source
 *
 * @note This function has REAL-TIME properties
 *
 * @note This function may be called from any thread
 */
static void ausrc_read_handler(const uint8_t *buf, size_t sz, void *arg)
{
	struct audio *a = arg;
	struct autx *tx = &a->tx;

	if (tx->muted)
		memset((void *)buf, 0, sz);

	(void)aubuf_write(tx->ab, buf, sz);

	if (a->cfg.txmode == AUDIO_MODE_POLL)
		poll_aubuf_tx(a);

	/* Exact timing: send Telephony-Events from here */
	check_telev(a, tx);
}


static void ausrc_error_handler(int err, const char *str, void *arg)
{
	struct audio *a = arg;
	MAGIC_CHECK(a);

	if (a->errh)
		a->errh(err, str, a->arg);
}


static int pt_handler(struct audio *a, uint8_t pt_old, uint8_t pt_new)
{
	const struct sdp_format *lc;

	lc = sdp_media_lformat(stream_sdpmedia(a->strm), pt_new);
	if (!lc)
		return ENOENT;

	if (pt_old != (uint8_t)-1) {
		(void)re_printf("Audio decoder changed payload"
				 " %u -> %u\n",
				 pt_old, pt_new);
	}

	a->rx.pt = pt_new;

	return audio_decoder_set(a, lc->data, lc->pt, lc->params);
}


static void handle_telev(struct audio *a, struct mbuf *mb)
{
	int event, digit;
	bool end;

	if (telev_recv(a->telev, mb, &event, &end))
		return;

	digit = telev_code2digit(event);
	if (digit >= 0 && a->eventh)
		a->eventh(digit, end, a->arg);
}


/**
 * Decode incoming packets using the Audio decoder
 *
 * NOTE: mb=NULL if no packet received
 */
static int aurx_stream_decode(struct aurx *rx, struct mbuf *mb)
{
	size_t sampc = AUDIO_SAMPSZ;
	int16_t *sampv;
	struct le *le;
	int err = 0;

	/* No decoder set */
	if (!rx->ac)
		return 0;

	if (mbuf_get_left(mb)) {
		err = rx->ac->dech(rx->dec, rx->sampv, &sampc,
				   mbuf_buf(mb), mbuf_get_left(mb));
	}
	else if (rx->ac->plch) {
		err = rx->ac->plch(rx->dec, rx->sampv, &sampc);
	}
	else {
		/* no PLC in the codec, might be done in filters below */
		sampc = 0;
	}

	if (err) {
		DEBUG_WARNING("%s codec decode %u bytes: %m\n",
			      rx->ac->name, mbuf_get_left(mb), err);
		goto out;
	}

	/* Process exactly one audio-frame in reverse list order */
	for (le = rx->filtl.tail; le; le = le->prev) {
		struct aufilt_dec_st *st = le->data;

		if (st->af && st->af->dech)
			err |= st->af->dech(st, rx->sampv, &sampc);
	}

	if (!rx->ab)
		goto out;

	sampv = rx->sampv;

	/* optional resampler */
	if (rx->resamp) {
		size_t sampc_rs = AUDIO_SAMPSZ;

		err = auresamp_process(rx->resamp,
				       rx->sampv_rs, &sampc_rs,
				       rx->sampv, sampc);
		if (err)
			return err;

		sampv = rx->sampv_rs;
		sampc = sampc_rs;
	}

	err = aubuf_write_samp(rx->ab, sampv, sampc);
	if (err)
		goto out;

 out:
	return err;
}


/* Handle incoming stream data from the network */
static void stream_recv_handler(const struct rtp_header *hdr,
				struct mbuf *mb, void *arg)
{
	struct audio *a = arg;
	struct aurx *rx = &a->rx;
	int err;

	if (!mb)
		goto out;

	/* Telephone event? */
	if (hdr->pt == rx->pt_tel) {
		handle_telev(a, mb);
		return;
	}

	/* Comfort Noise (CN) as of RFC 3389 */
	if (PT_CN == hdr->pt)
		return;

	/* Audio payload-type changed? */
	/* XXX: this logic should be moved to stream.c */
	if (hdr->pt != rx->pt) {

		err = pt_handler(a, rx->pt, hdr->pt);
		if (err)
			return;
	}

 out:
	(void)aurx_stream_decode(&a->rx, mb);
}


static int add_telev_codec(struct audio *a)
{
	struct sdp_media *m = stream_sdpmedia(audio_strm(a));
	struct sdp_format *sf;
	int err;

	/* Use payload-type 101 if available, for CiscoGW interop */
	err = sdp_format_add(&sf, m, false,
			     (!sdp_media_lformat(m, 101)) ? "101" : NULL,
			     telev_rtpfmt, TELEV_SRATE, 1, NULL,
			     NULL, NULL, false, "0-15");
	if (err)
		return err;

	a->rx.pt_tel = sf->pt;

	return err;
}


int audio_alloc(struct audio **ap, const struct config *cfg,
		struct call *call, struct sdp_session *sdp_sess, int label,
		const struct mnat *mnat, struct mnat_sess *mnat_sess,
		const struct menc *menc, struct menc_sess *menc_sess,
		uint32_t ptime, const struct list *aucodecl,
		audio_event_h *eventh, audio_err_h *errh, void *arg)
{
	struct audio *a;
	struct autx *tx;
	struct aurx *rx;
	struct le *le;
	int err;

	if (!ap || !cfg)
		return EINVAL;

	a = mem_zalloc(sizeof(*a), audio_destructor);
	if (!a)
		return ENOMEM;

	MAGIC_INIT(a);

	a->cfg = cfg->audio;
	tx = &a->tx;
	rx = &a->rx;

	err = stream_alloc(&a->strm, &cfg->avt, call, sdp_sess,
			   "audio", label,
			   mnat, mnat_sess, menc, menc_sess,
			   stream_recv_handler, NULL, a);
	if (err)
		goto out;

	if (cfg->avt.rtp_bw.max) {
		stream_set_bw(a->strm, AUDIO_BANDWIDTH);
	}

	err = sdp_media_set_lattr(stream_sdpmedia(a->strm), true,
				  "ptime", "%u", ptime);
	if (err)
		goto out;

	/* Audio codecs */
	for (le = list_head(aucodecl); le; le = le->next) {
		err = add_audio_codec(a, stream_sdpmedia(a->strm), le->data);
		if (err)
			goto out;
	}

	tx->mb = mbuf_alloc(STREAM_PRESZ + 4096);
	tx->sampv = mem_zalloc(AUDIO_SAMPSZ * 2, NULL);
	rx->sampv = mem_zalloc(AUDIO_SAMPSZ * 2, NULL);
	if (!tx->mb || !tx->sampv || !rx->sampv) {
		err = ENOMEM;
		goto out;
	}

	err = telev_alloc(&a->telev, TELEV_PTIME);
	if (err)
		goto out;

	err = add_telev_codec(a);
	if (err)
		goto out;

	str_ncpy(tx->device, a->cfg.src_dev, sizeof(tx->device));
	tx->ptime  = ptime;
	tx->ts     = rand_u16();
	tx->marker = true;

	str_ncpy(rx->device, a->cfg.play_dev, sizeof(rx->device));
	rx->pt     = -1;
	rx->ptime  = ptime;

	a->eventh  = eventh;
	a->errh    = errh;
	a->arg     = arg;

	if (a->cfg.txmode == AUDIO_MODE_TMR)
		tmr_init(&tx->u.tmr);

	tone_generator_init(&a->tx.tonegen);

 out:
	if (err)
		mem_deref(a);
	else
		*ap = a;

	return err;
}

int audio_alloc2(struct audio **ap, const struct config *cfg,
		const char* forced_src_mod, const char* forced_dev_name,
		struct sdp_media * sdp,
		uint32_t ptime,
		audio_event_h *eventh, audio_err_h *errh, void *arg)
{
	struct audio *a;
	struct autx *tx;
	struct aurx *rx;
	struct le *le;
	int err;

	if (!ap || !cfg)
		return EINVAL;

	a = mem_zalloc(sizeof(*a), audio_destructor);
	if (!a)
		return ENOMEM;

	MAGIC_INIT(a);

	a->cfg = cfg->audio;
	if (forced_src_mod) {
		str_ncpy(a->cfg.src_mod, forced_src_mod, sizeof(a->cfg.src_mod));
	}
	if (forced_dev_name) {
    	str_ncpy(a->cfg.src_dev, forced_dev_name, sizeof(a->cfg.src_dev));
	}

	tx = &a->tx;
	rx = &a->rx;

	err = stream_alloc2(&a->strm, &cfg->avt,
				sdp,
			   AF_INET,
			   "audio", 0,
			   stream_recv_handler, NULL, a);
	if (err)
		goto out;

	stream_set_bw(a->strm, AUDIO_BANDWIDTH);

	tx->mb = mbuf_alloc(STREAM_PRESZ + 4096);
	tx->sampv = mem_zalloc(AUDIO_SAMPSZ * 2, NULL);
	rx->sampv = mem_zalloc(AUDIO_SAMPSZ * 2, NULL);
	if (!tx->mb || !tx->sampv || !rx->sampv) {
		err = ENOMEM;
		goto out;
	}

	str_ncpy(tx->device, a->cfg.src_dev, sizeof(tx->device));
	tx->ptime  = ptime;
	tx->ts     = rand_u16();
	tx->marker = true;

	str_ncpy(rx->device, a->cfg.play_dev, sizeof(rx->device));
	rx->pt     = -1;
	rx->ptime  = ptime;

	a->eventh  = eventh;
	a->errh    = errh;
	a->arg     = arg;

	if (a->cfg.txmode == AUDIO_MODE_TMR)
		tmr_init(&tx->u.tmr);

	tone_generator_init(&a->tx.tonegen);

 out:
	if (err)
		mem_deref(a);
	else
		*ap = a;

	return err;
}


#ifdef HAVE_PTHREAD
static void *tx_thread(void *arg)
{
	struct audio *a = arg;

	/* Enable Real-time mode for this thread, if available */
	if (a->cfg.txmode == AUDIO_MODE_THREAD_REALTIME)
		(void)realtime_enable(true, 1);

	while (a->tx.u.thr.run) {

		poll_aubuf_tx(a);

		sys_msleep(5);
	}

	return NULL;
}
#endif


static void timeout_tx(void *arg)
{
	struct audio *a = arg;

	tmr_start(&a->tx.u.tmr, 5, timeout_tx, a);

	poll_aubuf_tx(a);
}


static void aufilt_param_set(struct aufilt_prm *prm,
			     const struct aucodec *ac, uint32_t ptime)
{
	if (!ac) {
		DEBUG_WARNING("aufilt param: NO CODEC!\n");
		memset(prm, 0, sizeof(*prm));
		return;
	}

	prm->srate      = get_srate(ac);
	prm->ch         = ac->ch;
	prm->frame_size = calc_nsamp(get_srate(ac), ac->ch, ptime);
}


static int autx_print_pipeline(struct re_printf *pf, const struct autx *autx)
{
	struct le *le;
	int err;

	if (!autx)
		return 0;

	err = re_hprintf(pf, "audio tx pipeline:  %10s",
			 autx->ausrc ? autx->ausrc->as->name : "src");

	for (le = list_head(&autx->filtl); le; le = le->next) {
		struct aufilt_enc_st *st = le->data;

		if (st->af->ench)
			err |= re_hprintf(pf, " ---> %s", st->af->name);
	}

	err |= re_hprintf(pf, " ---> %s\n",
			  autx->ac ? autx->ac->name : "encoder");

	return err;
}


static int aurx_print_pipeline(struct re_printf *pf, const struct aurx *aurx)
{
	struct le *le;
	int err;

	if (!aurx)
		return 0;

	err = re_hprintf(pf, "audio rx pipeline:  %10s",
			 aurx->auplay ? aurx->auplay->ap->name : "play");

	for (le = list_head(&aurx->filtl); le; le = le->next) {
		struct aufilt_dec_st *st = le->data;

		if (st->af->dech)
			err |= re_hprintf(pf, " <--- %s", st->af->name);
	}

	err |= re_hprintf(pf, " <--- %s\n",
			  aurx->ac ? aurx->ac->name : "decoder");

	return err;
}


/**
 * Setup the audio-filter chain
 *
 * must be called before auplay/ausrc-alloc
 */
static int aufilt_setup(struct audio *a)
{
	struct aufilt_prm encprm, decprm;
	struct autx *tx = &a->tx;
	struct aurx *rx = &a->rx;
	struct le *le;

	/* wait until we have both Encoder and Decoder */
	if (!tx->ac || !rx->ac)
		return 0;

	if (!list_isempty(&tx->filtl) || !list_isempty(&rx->filtl))
		return 0;

	aufilt_param_set(&encprm, tx->ac, tx->ptime);
	aufilt_param_set(&decprm, rx->ac, rx->ptime);

	/* Audio filters */
	for (le = list_head(aufilt_list()); le; le = le->next) {
		struct aufilt *af = le->data;
		struct aufilt_enc_st *encst = NULL;
		struct aufilt_dec_st *decst = NULL;
		void *ctx = NULL;
		int err = 0;		

		if (af->encupdh) {
			err |= af->encupdh(&encst, &ctx, af, &encprm);
			if (err) {
				continue;
			}

			encst->af = af;
			list_append(&tx->filtl, &encst->le, encst);
		}

		if (af->decupdh) {
			err |= af->decupdh(&decst, &ctx, af, &decprm);
			if (err) {
				continue;
			}

			decst->af = af;
			list_append(&rx->filtl, &decst->le, decst);
		}

		if (err) {
			DEBUG_WARNING("audio-filter '%s' update failed (%m)\n",
				      af->name, err);
			break;
		}
	}

	return 0;
}


static int start_player(struct aurx *rx, struct audio *a)
{
	const struct aucodec *ac = rx->ac;
	uint32_t srate_dsp = get_srate(ac);
	int err;

	DEBUG_WARNING("start_player\n");

	if (!ac)
		return 0;

	/* Optional resampler, if configured */
	if (a->cfg.srate_play && a->cfg.srate_play != srate_dsp
	    && !rx->resamp) {

		srate_dsp = a->cfg.srate_play;

		(void)re_printf("enable auplay resampler: %u --> %u Hz\n",
				get_srate(ac), srate_dsp);

		rx->sampv_rs = mem_zalloc(AUDIO_SAMPSZ * 2, NULL);
		if (!rx->sampv_rs)
			return ENOMEM;

		err = auresamp_alloc(&rx->resamp, AUDIO_SAMPSZ,
				     get_srate(ac), ac->ch,
				     srate_dsp, ac->ch);
		if (err)
			return err;
	}

	/* Start Audio Player */
	if (!rx->auplay && auplay_find(NULL)) {

		struct auplay_prm prm;

		prm.fmt        = AUFMT_S16LE;
		prm.srate      = srate_dsp;
		prm.ch         = ac->ch;
		prm.frame_size = calc_nsamp(prm.srate, prm.ch, rx->ptime);

		if (!rx->ab) {
			const size_t psize = 2 * prm.frame_size;

			err = aubuf_alloc(&rx->ab, psize * 1, psize * 8);
			if (err)
				return err;
		}

		DEBUG_WARNING("auplay_alloc\n");
		err = auplay_alloc(&rx->auplay,
					rx->mod[0]?rx->mod:a->cfg.play_mod,
				   &prm, rx->device,
				   auplay_write_handler, a);
		DEBUG_WARNING("auplay_alloc status = %d\n", err);
		if (err) {
			DEBUG_WARNING("start_player failed (%s.%s): %m\n",
				      a->cfg.play_mod, rx->device, err);
			return err;
		}
		rx->auplay_prm = prm;
	}

	DEBUG_WARNING("start_player: end\n");

	return 0;
}


static int start_source(struct autx *tx, struct audio *a)
{
	const struct aucodec *ac = tx->ac;
	uint32_t srate_dsp = get_srate(tx->ac);
	int err;

	DEBUG_WARNING("start_source\n");

	if (!ac)
		return 0;

	/* Optional resampler, if configured */
	if (a->cfg.srate_src && a->cfg.srate_src != srate_dsp &&
	    !tx->resamp) {

		srate_dsp = a->cfg.srate_src;

		(void)re_printf("enable ausrc resampler: %u --> %u Hz\n",
				get_srate(ac), srate_dsp);

		tx->sampv_rs = mem_zalloc(AUDIO_SAMPSZ * 2, NULL);
		if (!tx->sampv_rs)
			return ENOMEM;

		err = auresamp_alloc(&tx->resamp, AUDIO_SAMPSZ,
				     srate_dsp, ac->ch,
				     get_srate(ac), ac->ch);
		if (err)
			return err;
	}

	/* Start Audio Source */
	if (!tx->ausrc && ausrc_find(NULL)) {

		struct ausrc_prm prm;

		prm.fmt        = AUFMT_S16LE;
		prm.srate      = srate_dsp;
		prm.ch         = ac->ch;
		prm.frame_size = calc_nsamp(prm.srate, prm.ch, tx->ptime);

		tx->psize = 2 * prm.frame_size;

		if (!tx->ab) {
			err = aubuf_alloc(&tx->ab, tx->psize * 2,
					  tx->psize * 30);
			if (err)
				return err;
		}

		DEBUG_WARNING("ausrc_alloc\n");
		err = ausrc_alloc(&tx->ausrc, NULL, a->cfg.src_mod,
				  &prm, tx->device,
				  ausrc_read_handler, ausrc_error_handler, a);
		DEBUG_WARNING("ausrc_alloc status = %d\n", err);
		if (err) {
			DEBUG_WARNING("start_source failed: %m\n", err);
			return err;
		}

		switch (a->cfg.txmode) {
#ifdef HAVE_PTHREAD
		case AUDIO_MODE_THREAD:
		case AUDIO_MODE_THREAD_REALTIME:
			if (!tx->u.thr.run) {
				tx->u.thr.run = true;
				err = pthread_create(&tx->u.thr.tid, NULL,
						     tx_thread, a);
				if (err) {
					tx->u.thr.tid = false;
					return err;
				}
			}
			break;
#endif

		case AUDIO_MODE_TMR:
			tmr_start(&tx->u.tmr, 1, timeout_tx, a);
			break;

		default:
			break;
		}

		tx->ausrc_prm = prm;
	}

	DEBUG_WARNING("start_source: end\n");

	return 0;
}

static void ausrc_extra_read_handler(const uint8_t *buf, size_t sz, void *arg)
{
}


static void ausrc_extra_error_handler(int err, const char *str, void *arg)
{
}

/** Extra instance of audio input just to reduce problem with some Bluetooth headset delay
	(switching modes at the beginning of the call?)
*/
static int start_extra_source(struct autx *tx, struct audio *a)
{
    int err;
	/* Start Audio Source */
	if (!tx->ausrc_extra && ausrc_find(NULL)) {

		struct ausrc_prm prm;

		prm.fmt        = AUFMT_S16LE;
		prm.srate      = 8000;
		prm.ch         = 1;
		prm.frame_size = calc_nsamp(prm.srate, prm.ch, 20);

		tx->psize = 2 * prm.frame_size;

		DEBUG_WARNING("ausrc_extra_alloc\n");
		err = ausrc_alloc(&tx->ausrc_extra, NULL, a->cfg.src_mod,
				  &prm, tx->device,
				  ausrc_extra_read_handler, ausrc_extra_error_handler, a);
		DEBUG_WARNING("ausrc_extra_alloc status = %d\n", err);
		if (err) {
			DEBUG_WARNING("start_extra_source failed: %m\n", err);
			return err;
		}

		switch (a->cfg.txmode) {
#ifdef HAVE_PTHREAD
		case AUDIO_MODE_THREAD:
		case AUDIO_MODE_THREAD_REALTIME:
			if (!tx->u.thr.run) {
				tx->u.thr.run = true;
				err = pthread_create(&tx->u.thr.tid, NULL,
						     tx_thread, a);
				if (err) {
					tx->u.thr.tid = false;
					return err;
				}
			}
			break;
#endif

		case AUDIO_MODE_TMR:
			tmr_start(&tx->u.tmr, 1, timeout_tx, a);
			break;

		default:
			break;
		}

		tx->ausrc_prm = prm;
	}

	DEBUG_WARNING("start_extra_source: end\n");

	return 0;
}


int audio_start_extra_source(struct audio *a)
{
	int err;
	DEBUG_WARNING("starting extra source (only)...\n");
	err  = start_extra_source(&a->tx, a);
	if (err) {
		DEBUG_WARNING("start_source failed: %m\n", err);
	}
	return err;
}

/**
 * Start the audio playback and recording
 *
 * @param a Audio object
 *
 * @return 0 if success, otherwise errorcode
 */
int audio_start(struct audio *a)
{
	int err;

	if (!a)
		return EINVAL;

	err = stream_start(a->strm);
	if (err) {
		DEBUG_WARNING("stream_start failed: %m\n", err);
		return err;
	}

	/* Audio filter */
	if (!list_isempty(aufilt_list())) {
		err = aufilt_setup(a);
		if (err) {
			DEBUG_WARNING("aufilt_setup failed: %m\n", err);
			return err;
		}
	}

	/* configurable order of play/src start */
	if (a->cfg.src_first) {
		DEBUG_WARNING("starting source...\n");
		err  = start_source(&a->tx, a);
		if (err) {
			DEBUG_WARNING("start_source failed: %m\n", err);
		} else {
			DEBUG_WARNING("starting player...\n");
			err = start_player(&a->rx, a);
		}
	}
	else {
		DEBUG_WARNING("starting player...\n");
		err  = start_player(&a->rx, a);
		if (err) {
			DEBUG_WARNING("start_player failed: %m\n", err);
		} else {
			DEBUG_WARNING("starting source...\n");
			err = start_source(&a->tx, a);
		}
	}
#if 0
	if (a->tx.ausrc_extra) {
		mem_deref(a->tx.ausrc_extra);
		a->tx.ausrc_extra = NULL;
	}
#endif
	if (err)
		return err;

	if (a->tx.ac && a->rx.ac) {

		if (!a->started) {
			(void)re_printf("%H%H",
					autx_print_pipeline, &a->tx,
					aurx_print_pipeline, &a->rx);
		}

		a->started = true;
	}

	return err;
}


/**
 * Stop the audio playback and recording
 *
 * @param a Audio object
 */
void audio_stop(struct audio *a)
{
	struct autx *tx;
	struct aurx *rx;

	if (!a)
		return;

	tx = &a->tx;
	rx = &a->rx;

	switch (a->cfg.txmode) {

#ifdef HAVE_PTHREAD
	case AUDIO_MODE_THREAD:
	case AUDIO_MODE_THREAD_REALTIME:
		if (tx->u.thr.run) {
			tx->u.thr.run = false;
			pthread_join(tx->u.thr.tid, NULL);
		}
		break;
#endif
	case AUDIO_MODE_TMR:
		tmr_cancel(&tx->u.tmr);
		break;

	default:
		break;
	}

	/* audio device must be stopped first */
	if (tx->ausrc_extra) {
		mem_deref(tx->ausrc_extra);
		tx->ausrc_extra = NULL;
	}
	tx->ausrc  = mem_deref(tx->ausrc);
	rx->auplay = mem_deref(rx->auplay);

	list_flush(&tx->filtl);
	list_flush(&rx->filtl);
	tx->ab = mem_deref(tx->ab);
	rx->ab = mem_deref(rx->ab);
}


int audio_encoder_set(struct audio *a, const struct aucodec *ac,
		      int pt_tx, const char *params)
{
	struct autx *tx;
	int err = 0;
	bool reset;

	if (!a || !ac)
		return EINVAL;

	tx = &a->tx;

	reset = !aucodec_equal(ac, tx->ac);

	if (ac != tx->ac) {
		(void)re_printf("Set audio encoder: %s %uHz %dch\n",
				 ac->name, get_srate(ac), ac->ch);

		/* Audio source must be stopped first */
		if (reset) {
			tx->ausrc = mem_deref(tx->ausrc);
		}

		tx->is_g722 = (0 == str_casecmp(ac->name, "G722"));
		tx->enc = mem_deref(tx->enc);
		tx->ac = ac;
	}

	if (ac->encupdh) {
		struct auenc_param prm;

		prm.ptime = tx->ptime;

		err = ac->encupdh(&tx->enc, ac, &prm, params);
		if (err) {
			DEBUG_WARNING("alloc encoder: %m\n", err);
			return err;
		}
	}

	stream_set_srate(a->strm, get_srate(ac), get_srate(ac));
	stream_update_encoder(a->strm, pt_tx);

	if (!tx->ausrc) {
		err |= audio_start(a);
	}

	return err;
}


int audio_decoder_set(struct audio *a, const struct aucodec *ac,
		      int pt_rx, const char *params)
{
	struct aurx *rx;
	bool reset = false;
	int err = 0;

	if (!a || !ac)
		return EINVAL;

	rx = &a->rx;

	reset = !aucodec_equal(ac, rx->ac);

	if (ac != rx->ac) {

		(void)re_printf("Set audio decoder: %s %uHz %dch\n",
				 ac->name, get_srate(ac), ac->ch);

		rx->pt = pt_rx;
		rx->ac = ac;
		rx->dec = mem_deref(rx->dec);
	} else {
		(void)re_printf("Set audio decoder: %s %uHz %dch (not changed)\n",
				 ac->name, get_srate(ac), ac->ch);
	}

	if (ac->decupdh) {
		DEBUG_WARNING("ac: decupdh\n");
		err = ac->decupdh(&rx->dec, ac, params);
		DEBUG_WARNING("ac: decupdh done\n");
		if (err) {
			DEBUG_WARNING("alloc decoder: %m\n", err);
			return err;
		}
	} else {
		DEBUG_WARNING("ac: no decupdh\n");
	}

	stream_set_srate(a->strm, get_srate(ac), get_srate(ac));

	if (reset) {

		rx->auplay = mem_deref(rx->auplay);

		/* Reset audio filter chain */
		list_flush(&rx->filtl);

		DEBUG_WARNING("audio_decoder_set: audio_start\n");
		err |= audio_start(a);
	}

	DEBUG_WARNING("audio_decoder_set: status = %d\n", err);
	return err;
}


/**
 * Use the next audio encoder in the local list of negotiated codecs
 *
 * @param audio  Audio object
 */
void audio_encoder_cycle(struct audio *audio)
{
	const struct sdp_format *rc = NULL;

	if (!audio)
		return;

	rc = sdp_media_format_cycle(stream_sdpmedia(audio_strm(audio)));
	if (!rc) {
		(void)re_printf("cycle audio: no remote codec found\n");
		return;
	}

	(void)audio_encoder_set(audio, rc->data, rc->pt, rc->params);
}


struct stream *audio_strm(const struct audio *a)
{
	return a ? a->strm : NULL;
}


int audio_send_digit_rfc2833(struct audio *a, char key)
{
	int err = 0;

	if (!a)
		return EINVAL;

	if (key > 0) {
		(void)re_printf("send DTMF digit: '%c'\n", key);
		err = telev_send(a->telev, telev_digit2code(key), false);
	}
	else if (a->tx.cur_key) {
		/* Key release */
		(void)re_printf("send DTMF digit end: '%c'\n", a->tx.cur_key);
		err = telev_send(a->telev,
				 telev_digit2code(a->tx.cur_key), true);
	}

	a->tx.cur_key = key;

	return err;
}


int audio_send_digit_inband(struct audio *a, char key)
{
	int err = 0;

	if (!a)
		return EINVAL;

	if (key > 0) {
		(void)re_printf("send DTMF digit (inband): '%c'\n", key);
		err = dtmf_queue_digit(&a->tx.dtmfgen, key, a->tx.ac->srate, 0, 0);
		if (err) {
			DEBUG_WARNING("audio: invalid DTMF digit (0x%02x)\n", (int)key);
		}
	}

	a->tx.cur_key = key;

	return err;
}

int audio_start_tone(struct audio *a, unsigned int tone_id, float amplitude, float frequency)
{
	if (!a)
		return EINVAL;
	return tone_generator_start(&a->tx.tonegen, tone_id, amplitude, frequency);
}

int audio_stop_tone(struct audio *a, unsigned int tone_id)
{
	if (!a)
		return EINVAL;
	return tone_generator_stop(&a->tx.tonegen, tone_id);
}


/**
 * Mute the audio stream source (i.e. Microphone)
 *
 * @param a      Audio stream
 * @param muted  True to mute, false to un-mute
 */
void audio_mute(struct audio *a, bool muted)
{
	if (!a)
		return;

	a->tx.muted = muted;
}


/**
 * Get the mute state of an audio source
 *
 * @param a      Audio stream
 *
 * @return True if muted, otherwise false
 */
bool audio_ismuted(const struct audio *a)
{
	if (!a)
		return false;

	return a->tx.muted;
}


void audio_sdp_attr_decode(struct audio *a)
{
	const char *attr;

	if (!a)
		return;

	/* This is probably only meaningful for audio data, but
	   may be used with other media types if it makes sense. */
	attr = sdp_media_rattr(stream_sdpmedia(a->strm), "ptime");
	if (attr) {
		struct autx *tx = &a->tx;
		uint32_t ptime_tx = atoi(attr);

		if (ptime_tx && ptime_tx != a->tx.ptime) {

			DEBUG_NOTICE("peer changed ptime_tx %u -> %u\n",
				     a->tx.ptime, ptime_tx);

			tx->ptime = ptime_tx;

			if (tx->ac) {
				tx->psize = 2 * get_framesize(tx->ac,
							      ptime_tx);
			}
		}
	}
}


static int aucodec_print(struct re_printf *pf, const struct aucodec *ac)
{
	if (!ac)
		return 0;

	return re_hprintf(pf, "%s %uHz/%dch", ac->name, get_srate(ac), ac->ch);
}


int audio_debug(struct re_printf *pf, const struct audio *a)
{
	const struct autx *tx;
	const struct aurx *rx;
	int err;

	if (!a)
		return 0;

	tx = &a->tx;
	rx = &a->rx;

	err  = re_hprintf(pf, "\n--- Audio stream ---\n");

	err |= re_hprintf(pf, " tx:   %H %H ptime=%ums\n",
			  aucodec_print, tx->ac,
			  aubuf_debug, tx->ab,
			  tx->ptime);

	err |= re_hprintf(pf, " rx:   %H %H ptime=%ums pt=%d pt_tel=%d\n",
			  aucodec_print, rx->ac,
			  aubuf_debug, rx->ab,
			  rx->ptime, rx->pt, rx->pt_tel);

	err |= re_hprintf(pf,
			  " %H"
			  " %H",
			  autx_print_pipeline, tx,
			  aurx_print_pipeline, rx);

	err |= stream_debug(pf, a->strm);

	return err;
}


void audio_set_rx_device(struct audio *a, const char *mod, const char *dev)
{
	if (!a)
		return;

	str_ncpy(a->rx.mod, mod, sizeof(a->rx.mod));
	str_ncpy(a->rx.device, dev, sizeof(a->rx.device));
}

void audio_set_tx_device(struct audio *a, const char *mod, const char *dev)
{
	if (!a)
		return;

	str_ncpy(a->tx.mod, mod, sizeof(a->tx.mod));
	str_ncpy(a->tx.device, dev, sizeof(a->tx.device));
}

int audio_set_source(struct audio *au, const char *mod, const char *device)
{
	struct autx *tx;
	int err;

	if (!au)
		return EINVAL;

	tx = &au->tx;

	/* stop the audio device first */
	tx->ausrc = mem_deref(tx->ausrc);

	err = ausrc_alloc(&tx->ausrc, NULL, mod, &tx->ausrc_prm, device,
			  ausrc_read_handler, ausrc_error_handler, au);
	if (err) {
		DEBUG_WARNING("audio: set_source failed (%s.%s): %m\n",
			mod, device, err);
		return err;
	}
	(void)re_printf("%H", autx_print_pipeline, &au->tx);

	return 0;
}


int audio_set_player(struct audio *au, const char *mod, const char *device)
{
	struct aurx *rx;
	int err;

	if (!au)
		return EINVAL;

	rx = &au->rx;

	/* stop the audio device first */
	rx->auplay = mem_deref(rx->auplay);

	err = auplay_alloc(&rx->auplay, mod, &rx->auplay_prm, device,
			   auplay_write_handler, au);
	if (err) {
		DEBUG_WARNING("audio: set_player failed (%s.%s): %m\n",
			mod, device, err);
		return err;
    }
	(void)re_printf("%H", aurx_print_pipeline, &au->rx);

	return 0;
}

const char* audio_get_rx_aucodec_name(const struct audio *a)
{
	if (a == NULL)
		return "";
	if (a->rx.ac == NULL)
		return "";
	if (a->rx.ac->name == NULL)
		return "";
	return a->rx.ac->name;
}
