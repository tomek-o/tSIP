/**
 * @file stream.c  Generic Media Stream
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h> 
#include <string.h>
#include <time.h>
#include <baresip.h>
#include "core.h"


#define DEBUG_MODULE "stream"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

#define MAGIC 0x00814ea5
#include "magic.h"


enum {
	RTP_RECV_SIZE    = 8192,  /**< Receive buffer for incoming RTP     */
	RTP_CHECK_INTERVAL = 1000  /* how often to check for RTP [ms] */
};


/** Defines a generic media stream */
struct stream {
	MAGIC_DECL

	struct le le;            /**< Linked list element                   */
	struct config_avt cfg;   /**< Stream configuration                  */
	struct call *call;       /**< Ref. to call object                   */
	struct sdp_media *sdp;   /**< SDP Media line                        */
	struct rtp_sock *rtp;    /**< RTP Socket                            */
	struct rtpkeep *rtpkeep; /**< RTP Keepalive                         */
	struct jbuf *jbuf;       /**< Jitter Buffer for incoming RTP        */
	struct mnat_media *mns;  /**< Media NAT traversal state             */
	const struct menc *menc; /**< Media encryption module               */
	struct menc_sess *mencs; /**< Media encryption session state        */
	struct menc_media *mes;  /**< Media Encryption media state          */
	uint32_t ssrc_rx;        /**< Incoming syncronizing source          */
	uint32_t pseq;           /**< Sequence number for incoming RTP      */
	bool rtcp;               /**< Enable RTCP                           */
	bool rtcp_mux;           /**< RTP/RTCP multiplex supported by peer  */
	bool jbuf_started;
	stream_rtp_h *rtph;      /**< Stream RTP handler                    */
	stream_rtcp_h *rtcph;    /**< Stream RTCP handler                   */
	void *arg;               /**< Handler argument                      */

	stream_error_h *errorh;
	void *errorh_arg;
	struct tmr tmr_rtp;
	uint64_t ts_last;
	bool terminated;
	uint32_t rtp_timeout_ms;  /* [ms] */

	int pt_enc;
	/*int pt_dec; todo: enable this */

	struct tmr tmr_stats;
	struct {
		uint32_t n_tx;
		uint32_t n_rx;
		size_t b_tx;
		size_t b_rx;
		size_t bitrate_tx;
		size_t bitrate_rx;
		uint64_t ts;
	} stats;
};
static void stream_close(struct stream *strm, int err)
{
	stream_error_h *errorh = strm->errorh;

	strm->terminated = true;
	strm->errorh = NULL;

	if (errorh) {
		errorh(strm, err, strm->errorh_arg);
	}
}

/* TODO: should we check RTP per stream, or should we have the
 *       check in struct call instead?
 */
static void check_rtp_handler(void *arg)
{
	struct stream *strm = arg;
	const uint64_t now = tmr_jiffies();
	int diff_ms;

	tmr_start(&strm->tmr_rtp, RTP_CHECK_INTERVAL,
		  check_rtp_handler, strm);

	/* If no RTP was received at all, check later */
	if (!strm->ts_last)
		return;

	/* We are in sendrecv mode, check when the last RTP packet
	 * was received.
	 */
	if (sdp_media_dir(strm->sdp) == SDP_SENDRECV) {

		diff_ms = (int)(now - strm->ts_last);

		//DEBUG_NOTICE("stream: last \"%s\" RTP packet: %d milliseconds\n",
		//      sdp_media_name(strm->sdp), diff_ms);

		if (diff_ms > (int)strm->rtp_timeout_ms) {

			re_printf("stream %s: no RTP packets received for"
			     " %d milliseconds\n",
			     sdp_media_name(strm->sdp), diff_ms);

			stream_close(strm, ETIMEDOUT);
		}
	}
	else {
		re_printf("check_rtp: not checking (dir=%s)\n",
			  sdp_dir_name(sdp_media_dir(strm->sdp)));
	}
}

static inline int lostcalc(struct stream *s, uint16_t seq)
{
	const uint16_t delta = seq - s->pseq;
	int lostc;

	if (s->pseq == (uint32_t)-1)
		lostc = 0;
	else if (delta == 0)
		return -1;
	else if (delta < 3000)
		lostc = delta - 1;
	else if (delta < 0xff9c)
		lostc = 0;
	else
		return -2;

	s->pseq = seq;

	return lostc;
}


static void stream_destructor(void *arg)
{
	struct stream *s = arg;

	tmr_cancel(&s->tmr_rtp);
	list_unlink(&s->le);
	tmr_cancel(&s->tmr_stats);
	mem_deref(s->rtpkeep);
	mem_deref(s->sdp);
	mem_deref(s->mes);
	mem_deref(s->mencs);
	mem_deref(s->mns);
	mem_deref(s->jbuf);
	mem_deref(s->rtp);
}


static void rtp_recv(const struct sa *src, const struct rtp_header *hdr,
		     struct mbuf *mb, void *arg)
{
	struct stream *s = arg;
	bool flush = false;
	int err;

	s->ts_last = tmr_jiffies();

	if (!mbuf_get_left(mb))
		return;

	if (!(sdp_media_ldir(s->sdp) & SDP_RECVONLY))
		return;

#if 0
	{
		char buf[64];
		/* debugging exact timing of read handler calls */
		(void)re_printf("[%s] rtp_recv\n", sys_time(buf, sizeof(buf)));
	}
#endif

	++s->stats.n_rx;
	s->stats.b_rx += mbuf_get_left(mb);

	if (hdr->ssrc != s->ssrc_rx) {
		if (s->ssrc_rx) {
			flush = true;
			DEBUG_NOTICE("%s: SSRC changed %x -> %x"
					" (%u bytes from %J)\n",
				     sdp_media_name(s->sdp),
				     s->ssrc_rx, hdr->ssrc,
				     mbuf_get_left(mb), src);
		}
		s->ssrc_rx = hdr->ssrc;
	}

	if (s->jbuf) {

		struct rtp_header hdr2;
		void *mb2 = NULL;

		/* Put frame in Jitter Buffer */
		if (flush)
			jbuf_flush(s->jbuf);

		err = jbuf_put(s->jbuf, hdr, mb);
		if (err) {
			(void)re_printf("%s: dropping %u bytes from %J (%m)\n",
					sdp_media_name(s->sdp), mb->end,
					src, err);
		}

		if (jbuf_get(s->jbuf, &hdr2, &mb2)) {

			if (!s->jbuf_started)
				return;

			memset(&hdr2, 0, sizeof(hdr2));
		}

		s->jbuf_started = true;

		if (lostcalc(s, hdr2.seq) > 0)
			s->rtph(hdr, NULL, s->arg);

		s->rtph(&hdr2, mb2, s->arg);

		mem_deref(mb2);
	}
	else {
		if (lostcalc(s, hdr->seq) > 0)
			s->rtph(hdr, NULL, s->arg);

		s->rtph(hdr, mb, s->arg);
	}
}


static void rtcp_handler(const struct sa *src, struct rtcp_msg *msg, void *arg)
{
	struct stream *s = arg;
	(void)src;

	if (s->rtcph)
		s->rtcph(msg, s->arg);
}


static int stream_sock_alloc(struct stream *s, int af)
{
	struct sa laddr;
	int tos, err;

	if (!s)
		return EINVAL;

	/* we listen on all interfaces */
	sa_init(&laddr, af);

	err = rtp_listen(&s->rtp, IPPROTO_UDP, &laddr,
			 s->cfg.rtp_ports.min, s->cfg.rtp_ports.max,
			 s->rtcp, rtp_recv, rtcp_handler, s);
	if (err)
		return err;

	tos = s->cfg.rtp_tos;
	(void)udp_setsockopt(rtp_sock(s->rtp), IPPROTO_IP, IP_TOS,
			     &tos, sizeof(tos));
	(void)udp_setsockopt(rtcp_sock(s->rtp), IPPROTO_IP, IP_TOS,
			     &tos, sizeof(tos));

	udp_rxsz_set(rtp_sock(s->rtp), RTP_RECV_SIZE);

	return 0;
}


enum {TMR_INTERVAL = 3};
static void tmr_stats_handler(void *arg)
{
	struct stream *s = arg;
	const uint64_t now = tmr_jiffies();
	uint32_t diff;

	tmr_start(&s->tmr_stats, TMR_INTERVAL * 1000, tmr_stats_handler, s);

 	if (now <= s->stats.ts)
		return;

	if (s->stats.ts) {
		diff = (uint32_t)(now - s->stats.ts);
		s->stats.bitrate_tx = 1000 * 8 * s->stats.b_tx / diff;
		s->stats.bitrate_rx = 1000 * 8 * s->stats.b_rx / diff;
	}

	/* Reset counters */
	s->stats.b_tx = s->stats.b_rx = 0;
	s->stats.ts = now;
}


int stream_alloc(struct stream **sp, const struct config_avt *cfg,
		 struct call *call, struct sdp_session *sdp_sess,
		 const char *name, int label,
		 const struct mnat *mnat, struct mnat_sess *mnat_sess,
		 const struct menc *menc, struct menc_sess *menc_sess,
		 stream_rtp_h *rtph, stream_rtcp_h *rtcph, void *arg)
{
	struct stream *s;
	int err;

	if (!sp || !cfg || !call || !rtph)
		return EINVAL;

	s = mem_zalloc(sizeof(*s), stream_destructor);
	if (!s)
		return ENOMEM;

	MAGIC_INIT(s);

	tmr_init(&s->tmr_stats);

	s->cfg   = *cfg;
	s->call  = call;
	s->rtph  = rtph;
	s->rtcph = rtcph;
	s->arg   = arg;
	s->pseq  = -1;
	s->rtcp  = s->cfg.rtcp_enable;

	err = stream_sock_alloc(s, call_af(call));
	if (err)
		goto out;

	/* Jitter buffer */
	if (cfg->jbuf_del.min && cfg->jbuf_del.max) {

		err = jbuf_alloc(&s->jbuf, cfg->jbuf_del.min,
				 cfg->jbuf_del.max);
		if (err)
			goto out;
	}

	err = sdp_media_add(&s->sdp, sdp_sess, name,
			    sa_port(rtp_local(s->rtp)),
			    (menc && menc->sdp_proto) ? menc->sdp_proto :
			    sdp_proto_rtpavp);
	if (err)
		goto out;

	if (label) {
		err |= sdp_media_set_lattr(s->sdp, true,
					   "label", "%d", label);
	}

	/* RFC 5761 */
	if (cfg->rtcp_mux)
		err |= sdp_media_set_lattr(s->sdp, true, "rtcp-mux", NULL);

	if (mnat) {
		err |= mnat->mediah(&s->mns, mnat_sess, IPPROTO_UDP,
				    rtp_sock(s->rtp),
				    s->rtcp ? rtcp_sock(s->rtp) : NULL,
				    s->sdp);
	}

	if (menc) {
		s->menc = menc;
		s->mencs = mem_ref(menc_sess);
		err |= menc->mediah(&s->mes, menc_sess,
				    s->rtp,
				    IPPROTO_UDP,
				    rtp_sock(s->rtp),
				    s->rtcp ? rtcp_sock(s->rtp) : NULL,
				    s->sdp);
	}

	if (err)
		goto out;

	s->pt_enc = -1;

	list_append(call_streaml(call), &s->le, s);

 out:
	if (err)
		mem_deref(s);
	else
		*sp = s;

	return err;
}



int stream_alloc2(struct stream **sp, const struct config_avt *cfg,
 		 struct sdp_media * sdp,
		 int af,
		 const char *name, int label,
		 stream_rtp_h *rtph, stream_rtcp_h *rtcph, void *arg)
{
	struct stream *s;
	int err;

	if (!sp || !cfg || !rtph)
		return EINVAL;

	s = mem_zalloc(sizeof(*s), stream_destructor);
	if (!s)
		return ENOMEM;

	MAGIC_INIT(s);

	tmr_init(&s->tmr_stats);

	s->cfg   = *cfg;
	s->call  = NULL;
	s->rtph  = rtph;
	s->rtcph = NULL;
	s->arg   = arg;
	s->pseq  = -1;
	s->rtcp  = 0;

	err = stream_sock_alloc(s, af);
	if (err)
		goto out;

	/* Jitter buffer */
	if (cfg->jbuf_del.min && cfg->jbuf_del.max) {

		err = jbuf_alloc(&s->jbuf, cfg->jbuf_del.min,
				 cfg->jbuf_del.max);
		if (err)
			goto out;
	}
#if 0
	err = sdp_media_add(&s->sdp, sdp_sess, name,
				sa_port(rtp_local(s->rtp)),
				sdp_proto_rtpavp);
	if (err)
		goto out;
#endif
	s->sdp = mem_ref(sdp);

	if (label) {
		err |= sdp_media_set_lattr(s->sdp, true,
					   "label", "%d", label);
	}

	if (err)
		goto out;

	s->pt_enc = -1;

 out:
	if (err)
		mem_deref(s);
	else
		*sp = s;

	return err;
}


struct sdp_media *stream_sdpmedia(const struct stream *s)
{
	return s ? s->sdp : NULL;
}


int stream_start(struct stream *s)
{
	if (!s)
		return EINVAL;

	tmr_start(&s->tmr_stats, 1, tmr_stats_handler, s);

	return 0;
}


static void stream_start_keepalive(struct stream *s)
{
	const char *rtpkeep;

	if (!s)
		return;

	if (!s->call)
		return;

	rtpkeep = ua_prm(call_get_ua(s->call))->rtpkeep;

	s->rtpkeep = mem_deref(s->rtpkeep);

	if (rtpkeep && sdp_media_rformat(s->sdp, NULL)) {
		int err;
		err = rtpkeep_alloc(&s->rtpkeep, rtpkeep,
				    IPPROTO_UDP, s->rtp, s->sdp);
		if (err) {
			DEBUG_WARNING("rtpkeep_alloc failed: %m\n", err);
		}
	}
}


int stream_send(struct stream *s, bool marker, int pt, uint32_t ts,
		struct mbuf *mb)
{
	int err = 0;

	if (!s)
		return EINVAL;

	if (!sa_isset(sdp_media_raddr(s->sdp), SA_ALL))
		return 0;
	if (sdp_media_dir(s->sdp) != SDP_SENDRECV)
		return 0;

	s->stats.b_tx += mbuf_get_left(mb);

	if (pt < 0)
		pt = s->pt_enc;

	if (pt >= 0) {
		err = rtp_send(s->rtp, sdp_media_raddr(s->sdp),
			       marker, pt, ts, mb);
	}

	rtpkeep_refresh(s->rtpkeep, ts);

	++s->stats.n_tx;

	return err;
}


static void stream_remote_set(struct stream *s, const char *cname)
{
	struct sa rtcp;

	if (!s)
		return;

	/* RFC 5761 */
	if (s->cfg.rtcp_mux && sdp_media_rattr(s->sdp, "rtcp-mux")) {

		if (!s->rtcp_mux)
			(void)re_printf("%s: RTP/RTCP multiplexing enabled\n",
					sdp_media_name(s->sdp));
		s->rtcp_mux = true;
	}

	rtcp_enable_mux(s->rtp, s->rtcp_mux);

	sdp_media_raddr_rtcp(s->sdp, &rtcp);

	rtcp_start(s->rtp, cname,
		   s->rtcp_mux ? sdp_media_raddr(s->sdp): &rtcp);
}


void stream_update(struct stream *s, const char *cname)
{
	const struct sdp_format *fmt;
	int err = 0;

	if (!s)
		return;

	fmt = sdp_media_rformat(s->sdp, NULL);

	s->pt_enc = fmt ? fmt->pt : -1;

	if (sdp_media_has_media(s->sdp))
		stream_remote_set(s, cname);

	if (s->menc && s->menc->mediah) {
		err = s->menc->mediah(&s->mes, s->mencs, s->rtp,
				      IPPROTO_UDP,
				      rtp_sock(s->rtp),
				      s->rtcp ? rtcp_sock(s->rtp) : NULL,
				      s->sdp);
		if (err) {
			DEBUG_WARNING("mediaenc update: %m\n", err);
		}
	}
}


void stream_update_encoder(struct stream *s, int pt_enc)
{
	if (!s)
		return;

	if (pt_enc >= 0)
		s->pt_enc = pt_enc;
}


int stream_jbuf_stat(struct re_printf *pf, const struct stream *s)
{
	struct jbuf_stat stat;
	int err;

	if (!s)
		return EINVAL;

	err  = re_hprintf(pf, " %s:", sdp_media_name(s->sdp));

	err |= jbuf_stats(s->jbuf, &stat);
	if (err) {
		err = re_hprintf(pf, "Jbuf stat: (not available)");
	}
	else {
		err = re_hprintf(pf, "Jbuf stat: put=%u get=%u or=%u ur=%u",
				  stat.n_put, stat.n_get,
				  stat.n_overflow, stat.n_underflow);
	}

	return err;
}


void stream_hold(struct stream *s, bool hold)
{
	if (!s)
		return;

	sdp_media_set_ldir(s->sdp, hold ? SDP_SENDONLY : SDP_SENDRECV);
}


void stream_set_srate(struct stream *s, uint32_t srate_tx, uint32_t srate_rx)
{
	if (!s)
		return;

	rtcp_set_srate(s->rtp, srate_tx, srate_rx);
}


void stream_send_fir(struct stream *s, bool pli)
{
	int err;

	if (!s)
		return;

	if (pli)
		err = rtcp_send_pli(s->rtp, s->ssrc_rx);
	else
		err = rtcp_send_fir(s->rtp, rtp_sess_ssrc(s->rtp));

	if (err) {
		DEBUG_WARNING("Send FIR: %m\n", err);
	}
}


void stream_reset(struct stream *s)
{
	if (!s)
		return;

	jbuf_flush(s->jbuf);

	stream_start_keepalive(s);
}


void stream_set_bw(struct stream *s, uint32_t bps)
{
	if (!s)
		return;

	sdp_media_set_lbandwidth(s->sdp, SDP_BANDWIDTH_AS, bps / 1024);
}

void stream_enable_rtp_timeout(struct stream *strm, uint32_t timeout_ms)
{
	if (!strm)
		return;

	strm->rtp_timeout_ms = timeout_ms;

	tmr_cancel(&strm->tmr_rtp);

	if (timeout_ms) {

		DEBUG_INFO("stream: Enable RTP timeout (%u milliseconds)\n",
		     timeout_ms);

		strm->ts_last = tmr_jiffies();
		tmr_start(&strm->tmr_rtp, 10, check_rtp_handler, strm);
	}
}


void stream_set_error_handler(struct stream *strm,
			      stream_error_h *errorh, void *arg)
{
	if (!strm)
		return;

	strm->errorh     = errorh;
	strm->errorh_arg = arg;
}


int stream_debug(struct re_printf *pf, const struct stream *s)
{
	struct sa rrtcp;
	int err;

	if (!s)
		return 0;

	err  = re_hprintf(pf, " %s dir=%s pt_enc=%d\n", sdp_media_name(s->sdp),
			  sdp_dir_name(sdp_media_dir(s->sdp)),
			  s->pt_enc);

	sdp_media_raddr_rtcp(s->sdp, &rrtcp);
	err |= re_hprintf(pf, " remote: %J/%J\n",
			  sdp_media_raddr(s->sdp), &rrtcp);

	err |= rtp_debug(pf, s->rtp);
	err |= jbuf_debug(pf, s->jbuf);

	return err;
}


int stream_print(struct re_printf *pf, const struct stream *s)
{
	if (!s)
		return 0;

	return re_hprintf(pf, " %s=%u/%u", sdp_media_name(s->sdp),
			  s->stats.bitrate_tx, s->stats.bitrate_rx);
}
