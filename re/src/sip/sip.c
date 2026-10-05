/**
 * @file sip.c  SIP Core
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re_types.h>
#include <re_mem.h>
#include <re_mbuf.h>
#include <re_sa.h>
#include <re_list.h>
#include <re_hash.h>
#include <re_fmt.h>
#include <re_uri.h>
#include <re_sys.h>
#include <re_tmr.h>
#include <re_udp.h>
#include <re_stun.h>
#include <re_msg.h>
#include <re_sip.h>
#include "sip.h"


static void destructor(void *arg)
{
	struct sip *sip = arg;

	if (sip->closing) {
		sip->closing = false;
		mem_ref(sip);
		if (sip->exith)
			sip->exith(sip->arg);
		return;
	}

	sip_request_close(sip);
	sip_request_close(sip);

	hash_flush(sip->ht_ctrans);
	mem_deref(sip->ht_ctrans);

	hash_flush(sip->ht_strans);
	hash_clear(sip->ht_strans_mrg);
	mem_deref(sip->ht_strans);
	mem_deref(sip->ht_strans_mrg);

	hash_flush(sip->ht_conn);
	mem_deref(sip->ht_conn);

	hash_flush(sip->ht_udpconn);
	mem_deref(sip->ht_udpconn);

	list_flush(&sip->transpl);
	list_flush(&sip->lsnrl);

	mem_deref(sip->software);
	mem_deref(sip->dnsc);
	mem_deref(sip->stun);
}


static void lsnr_destructor(void *arg)
{
	struct sip_lsnr *lsnr = arg;

	if (lsnr->lsnrp)
		*lsnr->lsnrp = NULL;

	list_unlink(&lsnr->le);
}


/**
 * Allocate a SIP stack instance
 *
 * @param sipp     Pointer to allocated SIP stack
 * @param dnsc     DNS Client (optional)
 * @param ctsz     Size of client transactions hashtable (power of 2)
 * @param stsz     Size of server transactions hashtable (power of 2)
 * @param tcsz     Size of SIP transport hashtable (power of 2)
 * @param software Software identifier
 * @param exith    SIP-stack exit handler
 * @param arg      Handler argument
 *
 * @return 0 if success, otherwise errorcode
 */
int sip_alloc(struct sip **sipp, struct dnsc *dnsc, uint32_t ctsz,
	      uint32_t stsz, uint32_t tcsz, const char *software,
	      sip_exit_h *exith, void *arg)
{
	struct sip *sip;
	int err;

	if (!sipp)
		return EINVAL;

	sip = mem_zalloc(sizeof(*sip), destructor);
	if (!sip)
		return ENOMEM;

	sip->tp_def = SIP_TRANSP_NONE;
	err = sip_transp_init(sip, tcsz);
	if (err)
		goto out;

	err = sip_ctrans_init(sip, ctsz);
	if (err)
		goto out;

	err = sip_strans_init(sip, stsz);
	if (err)
		goto out;

	err = hash_alloc(&sip->ht_udpconn, tcsz);
	if (err)
		goto out;

	err = stun_alloc(&sip->stun, NULL, NULL, NULL);
	if (err)
		goto out;

	if (software) {
		err = str_dup(&sip->software, software);
		if (err)
			goto out;
	}

	sip->dnsc  = mem_ref(dnsc);
	sip->exith = exith;
	sip->arg   = arg;

 out:
	if (err)
		mem_deref(sip);
	else
		*sipp = sip;

	return err;
}


/**
 * Close the SIP stack instance
 *
 * @param sip   SIP stack instance
 * @param force Don't wait for transactions to complete
 */
void sip_close(struct sip *sip, bool force)
{
	if (!sip)
		return;

	if (force) {
		sip_request_close(sip);
		sip_request_close(sip);
	}
	else if (!sip->closing) {
		sip->closing = true;
		sip_ctrans_shutdown(sip);
		mem_deref(sip);
	}
}


/**
 * Send a SIP message
 *
 * @param sip  SIP stack instance
 * @param sock Optional socket to send from
 * @param tp   SIP transport
 * @param dst  Destination network address
 * @param mb   Buffer containing SIP message
 *
 * @return 0 if success, otherwise errorcode
 */
int sip_send(struct sip *sip, void *sock, enum sip_transp tp,
	     const struct sa *dst, struct mbuf *mb)
{
	return sip_transp_send(NULL, sip, sock, tp, dst, NULL, mb, NULL, NULL);
}


/**
 * Listen for incoming SIP Requests and SIP Responses
 *
 * @param lsnrp Pointer to allocated listener
 * @param sip   SIP stack instance
 * @param req   True for Request, false for Response
 * @param msgh  SIP message handler
 * @param arg   Handler argument
 *
 * @return 0 if success, otherwise errorcode
 */
int sip_listen(struct sip_lsnr **lsnrp, struct sip *sip, bool req,
	       sip_msg_h *msgh, void *arg)
{
	struct sip_lsnr *lsnr;

	if (!sip || !msgh)
		return EINVAL;

	lsnr = mem_zalloc(sizeof(*lsnr), lsnr_destructor);
	if (!lsnr)
		return ENOMEM;

	list_append(&sip->lsnrl, &lsnr->le, lsnr);

	lsnr->msgh = msgh;
	lsnr->arg = arg;
	lsnr->req = req;

	if (lsnrp) {
		lsnr->lsnrp = lsnrp;
		*lsnrp = lsnr;
	}

	return 0;
}


/**
 * Print debug information about the SIP stack
 *
 * @param pf  Print function for debug output
 * @param sip SIP stack instance
 *
 * @return 0 if success, otherwise errorcode
 */
int sip_debug(struct re_printf *pf, const struct sip *sip)
{
	int err;

	if (!sip)
		return 0;

	err  = sip_transp_debug(pf, sip);
	err |= sip_ctrans_debug(pf, sip);
	err |= sip_strans_debug(pf, sip);

	return err;
}


void sip_log_messages(struct sip *sip, bool log, bool only_first_lines)
{
	if (!sip)
		return;
	sip->log_messages = log;
	sip->log_messages_only_first_lines = only_first_lines;
}


static bool compact_headers_rfc3261; /* RFC 3261 headers */
static bool compact_headers_ext;     /* headers from SIP extensions */


/**
 * Select full or compact header names for outgoing messages
 *
 * @param enabled     True to use compact names of headers defined in
 *                    RFC 3261 7.3.3 (v, f, t, i, m, c, l, e, k, s)
 * @param ext_enabled True to use compact names also for headers defined
 *                    in SIP extensions (Event "o", Allow-Events "u",
 *                    Refer-To "r", Referred-By "b", Session-Expires "x");
 *                    effective only together with enabled; some peers
 *                    do not recognize these short forms
 *
 * @note Process-wide setting, applies to messages created after the call
 */
void sip_set_compact_headers(bool enabled, bool ext_enabled)
{
	compact_headers_rfc3261 = enabled;
	compact_headers_ext = false;
	if (enabled)
		compact_headers_ext = ext_enabled;
}


/**
 * Get header name to use in outgoing messages
 *
 * @param id SIP Header ID
 *
 * @return Compact name if enabled and header has compact form, otherwise
 *         full name; empty string for unsupported header ID
 */
const char *sip_hname(enum sip_hdrid id)
{
	switch (id) {

	/* RFC 3261 */
	case SIP_HDR_CALL_ID:          return compact_headers_rfc3261 ? "i" : "Call-ID";
	case SIP_HDR_CONTACT:          return compact_headers_rfc3261 ? "m" : "Contact";
	case SIP_HDR_CONTENT_ENCODING: return compact_headers_rfc3261 ? "e" : "Content-Encoding";
	case SIP_HDR_CONTENT_LENGTH:   return compact_headers_rfc3261 ? "l" : "Content-Length";
	case SIP_HDR_CONTENT_TYPE:     return compact_headers_rfc3261 ? "c" : "Content-Type";
	case SIP_HDR_FROM:             return compact_headers_rfc3261 ? "f" : "From";
	case SIP_HDR_SUBJECT:          return compact_headers_rfc3261 ? "s" : "Subject";
	case SIP_HDR_SUPPORTED:        return compact_headers_rfc3261 ? "k" : "Supported";
	case SIP_HDR_TO:               return compact_headers_rfc3261 ? "t" : "To";
	case SIP_HDR_VIA:              return compact_headers_rfc3261 ? "v" : "Via";

	/* extensions: RFC 6665 (o, u), RFC 3515 (r), RFC 3892 (b), RFC 4028 (x) */
	case SIP_HDR_ALLOW_EVENTS:     return compact_headers_ext ? "u" : "Allow-Events";
	case SIP_HDR_EVENT:            return compact_headers_ext ? "o" : "Event";
	case SIP_HDR_REFER_TO:         return compact_headers_ext ? "r" : "Refer-To";
	case SIP_HDR_REFERRED_BY:      return compact_headers_ext ? "b" : "Referred-By";
	case SIP_HDR_SESSION_EXPIRES:  return compact_headers_ext ? "x" : "Session-Expires";

	default:                       return "";
	}
}

