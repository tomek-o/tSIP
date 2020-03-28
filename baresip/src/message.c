/**
 * @file message.c  SIP MESSAGE -- RFC 3428
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <baresip.h>
#include "core.h"

#define DEBUG_MODULE "message"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

static struct sip_lsnr *lsnr;
static message_recv_h *recvh;
static void *recvarg;


static void handle_message(struct ua *ua, const struct sip_msg *msg)
{
	static const char *ctype_text = "text/plain";
	struct pl mtype;
	(void)ua;

	if (re_regex(msg->ctype.p, msg->ctype.l, "[^;]+", &mtype))
		mtype = msg->ctype;

	if (0==pl_strcasecmp(&mtype, ctype_text) && recvh) {
		recvh(&msg->from.auri, &msg->ctype, msg->mb, recvarg);
		(void)sip_reply(uag_sip(), msg, 200, "OK");
	}
	else {
		(void)sip_replyf(uag_sip(), msg, 415, "Unsupported Media Type",
				 "Accept: %s\r\n"
				 "Content-Length: 0\r\n"
				 "\r\n",
				 ctype_text);
	}
}


static bool request_handler(const struct sip_msg *msg, void *arg)
{
	struct ua *ua;

	(void)arg;

	if (pl_strcmp(&msg->met, "MESSAGE"))
		return false;

	ua = uag_find(&msg->uri.user);
	if (!ua) {
		(void)sip_treply(NULL, uag_sip(), msg, 404, "Not Found");
		return true;
	}

	handle_message(ua, msg);

	return true;
}


static void resp_handler(int err, const struct sip_msg *msg, void *arg)
{
	struct ua *ua = arg;

	(void)ua;

	if (err) {
		DEBUG_WARNING("MESSAGE response handler: error = %d\n", err);
		return;
	}

	if (msg->scode >= 300) {
		DEBUG_WARNING("MESSAGE response: code %u, reason: %r\n", msg->scode, &msg->reason);
	}
}


int message_init(message_recv_h *h, void *arg)
{
	int err;

	err = sip_listen(&lsnr, uag_sip(), true, request_handler, NULL);
	if (err)
		return err;

	recvh   = h;
	recvarg = arg;

	return 0;
}


void message_close(void)
{
	lsnr = mem_deref(lsnr);
}


/**
 * Send SIP instant MESSAGE to a peer
 *
 * @param ua    User-Agent object
 * @param peer  Peer SIP Address
 * @param msg   Message to send
 *
 * @return 0 if success, otherwise errorcode
 */
int message_send(struct ua *ua, const char *peer, const char *msg)
{
	struct sip_addr addr;
	char *uri = NULL;
	int err = 0;
	struct mbuf *dialbuf;
	struct pl pl_dialbuf;
	size_t len;
	struct account *acc;

	if (!ua || !peer || !msg)
		return EINVAL;

	acc = ua_prm(ua);
	if (!acc)
		return EINVAL;

	len = str_len(peer);

	dialbuf = mbuf_alloc(128);
	if (!dialbuf)
		return ENOMEM;
	memset(dialbuf->buf, 0, 128);

	/* Append sip: scheme if missing */
	if (0 != re_regex(peer, len, "sip:"))
		err |= mbuf_printf(dialbuf, "sip:");

	err |= mbuf_write_str(dialbuf, peer);

	// assuming that if sip: is present than domain is present
	if (0 != re_regex(peer, len, "sip:")) {
#if HAVE_INET6
		if (AF_INET6 == ua->acc->luri.af)
			err |= mbuf_printf(dialbuf, "@[%r]", &acc->luri.host);
		else
#endif
			err |= mbuf_printf(dialbuf, "@%r", &acc->luri.host);
		/* Also append port if specified and not 5060 */
		switch (acc->luri.port) {
		case 0:
		case SIP_PORT:
			break;
		default:
			err |= mbuf_printf(dialbuf, ":%u", acc->luri.port);
			break;
		}
	}

	if (err)
		goto out;

	pl_dialbuf.p = dialbuf->buf;
	pl_dialbuf.l = dialbuf->size;
	err = sip_addr_decode(&addr, &pl_dialbuf);
	if (err)
		goto out;

	err = pl_strdup(&uri, &addr.auri);
	if (err)
		goto out;

	err = sip_req_send(ua, "MESSAGE", uri, resp_handler, ua,
			   "Accept: text/plain\r\n"
			   "Content-Type: text/plain\r\n"
			   "Content-Length: %zu\r\n"
			   "\r\n%s",
			   str_len(msg), msg);

	mem_deref(uri);

 out:
	mem_deref(dialbuf);
		
	return err;
}
