/**
 * @file subscriber.c Presence subscriber
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <baresip.h>
#include "presence.h"

#define DEBUG_MODULE "presence"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

/*
 * Subscriber - we subscribe to the status information of N resources.
 *
 * For each entry in the address book marked with ;presence=p2p,
 * we send a SUBSCRIBE to that person, and expect to receive
 * a NOTIFY when her status changes.
 */


struct presence {
	struct le le;
	struct sipsub *sub;
	struct tmr tmr;
	enum presence_status status;
	unsigned failc;
	struct contact *contact;
	unsigned int expires;
};

static struct list presencel;


static void tmr_handler(void *arg);


static uint32_t wait_term(const struct sipevent_substate *substate)
{
	uint32_t wait;

	switch (substate->reason) {

	case SIPEVENT_DEACTIVATED:
	case SIPEVENT_TIMEOUT:
		wait = 5;
		break;

	case SIPEVENT_REJECTED:
	case SIPEVENT_NORESOURCE:
		wait = 3600;
		break;

	case SIPEVENT_PROBATION:
	case SIPEVENT_GIVEUP:
	default:
		wait = 300;
		if (pl_isset(&substate->retry_after))
			wait = max(wait, pl_u32(&substate->retry_after));
		break;
	}

	return wait;
}


static uint32_t wait_fail(unsigned failc)
{
	switch (failc) {

	case 1:  return 30;
	case 2:  return 300;
	case 3:  return 600;
	default: return 3600;
	}
}


static void notify_handler(struct sip *sip, const struct sip_msg *msg, bool termconf, void *arg)
{
	enum presence_status status = PRESENCE_CLOSED;
	struct presence *pres = arg;
	const struct sip_hdr *hdr;
	struct pl pl;

	pres->failc = 0;

	hdr = sip_msg_hdr(msg, SIP_HDR_CONTENT_TYPE);
	if (!hdr || 0 != pl_strcasecmp(&hdr->val, "application/pidf+xml")) {

		if (hdr) {
			DEBUG_WARNING("presence: unsupported content-type: '%r'\n", &hdr->val);
		} else if (termconf && (mbuf_get_left(msg->mb) == 0)) {
			/* Ignore missing content type for terminated subscription (e.g. noresource) */
			(void)sip_treply(NULL, sip, msg, 200, "OK");
			return;
		}

		sip_treplyf(NULL, NULL, sip, msg, false,
			    415, "Unsupported Media Type",
			    "Accept: application/pidf+xml\r\n"
			    "Content-Length: 0\r\n"
			    "\r\n");
		return;
	}

	if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb),
		      "<status>[^<]*<basic>[^<]*</basic>[^<]*</status>",
		      NULL, &pl, NULL)) {

		if (!pl_strcasecmp(&pl, "open"))
			status = PRESENCE_OPEN;
	}

	pl.l = 0;
	if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb),
			  "<note>[^<]*</note>", &pl)) {
		//DEBUG_WARNING("presence: note = %r\n", &pl);
	}

	if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<rpid:away/>") ||
		!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<rpid:away />")
		) {
		status = PRESENCE_CLOSED;
	}
	else if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<rpid:busy/>") ||
		!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<rpid:busy />") ||
		!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<ep:busy/>") ||
		!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<ep:busy />")
		) {

		status = PRESENCE_BUSY;
	}
	else if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<rpid:on-the-phone/>") ||
		!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), "<rpid:on-the-phone />")
		) {
		status = PRESENCE_BUSY;
	}

	(void)sip_treply(NULL, sip, msg, 200, "OK");

	contact_set_presence(pres->contact, status, &pl);
}


static void close_handler(int err, const struct sip_msg *msg,
			  const struct sipevent_substate *substate, void *arg)
{
	struct presence *pres = arg;
	uint32_t wait;

	pres->sub = mem_deref(pres->sub);

	DEBUG_INFO("presence: subscriber closed <%r>: ",
	     &contact_addr(pres->contact)->auri);

	if (substate) {
		DEBUG_INFO("%s", sipevent_reason_name(substate->reason));
		wait = wait_term(substate);
	}
	else if (msg) {
		DEBUG_INFO("%u %r", msg->scode, &msg->reason);
		wait = wait_fail(++pres->failc);
	}
	else {
		DEBUG_INFO("%m", err);
		wait = wait_fail(++pres->failc);
	}

	DEBUG_INFO("; will retry in %u secs (failc=%u)\n", wait, pres->failc);

	tmr_start(&pres->tmr, wait * 1000, tmr_handler, pres);

	contact_set_presence(pres->contact, PRESENCE_UNKNOWN, &pl_null);
}


static void destructor(void *arg)
{
	struct presence *pres = arg;

	list_unlink(&pres->le);
	tmr_cancel(&pres->tmr);
	mem_deref(pres->contact);
	mem_deref(pres->sub);
}


static int auth_handler(char **username, char **password,
			const char *realm, void *arg)
{
	return account_auth(arg, username, password, realm);
}


static int subscribe(struct presence *pres)
{
	const char *routev[1];
	struct ua *ua;
	char uri[256];
	int err;

	/* We use the first UA */
	ua = uag_find_aor(NULL);
	if (!ua) {
		DEBUG_WARNING("presence: no UA found\n");
		return ENOENT;
	}

	pl_strcpy(&contact_addr(pres->contact)->auri, uri, sizeof(uri));

	routev[0] = ua_outbound(ua);

	err = sipevent_subscribe(&pres->sub, uag_sipevent_sock(), uri, NULL,
				 ua_aor(ua), "presence", "application/pidf+xml", NULL, pres->expires,
				 ua_cuser(ua), routev, routev[0] ? 1 : 0,
				 auth_handler, ua_prm(ua), true, NULL,
				 notify_handler, close_handler, pres,
				 "%H", ua_print_supported, ua);
	if (err) {
		DEBUG_WARNING("presence: sipevent_subscribe failed: %m\n", err);
	}

	return err;
}


static void tmr_handler(void *arg)
{
	struct presence *pres = arg;

	if (subscribe(pres)) {
		tmr_start(&pres->tmr, wait_fail(++pres->failc) * 1000,
			  tmr_handler, pres);
	}
}


static int presence_alloc(struct contact *contact, unsigned int expires)
{
	struct presence *pres;

	pres = mem_zalloc(sizeof(*pres), destructor);
	if (!pres)
		return ENOMEM;

	pres->status  = PRESENCE_UNKNOWN;
	pres->contact = mem_ref(contact);
	pres->expires = expires;

	tmr_init(&pres->tmr);
	tmr_start(&pres->tmr, 1000, tmr_handler, pres);

	list_append(&presencel, &pres->le, pres);

	return 0;
}


int subscriber_init(void)
{
	struct le *le;
	int err = 0;

	for (le = list_head(contact_list()); le; le = le->next) {

		struct contact *c = le->data;
		struct sip_addr *addr = contact_addr(c);
		struct pl val;

		if (0 == msg_param_decode(&addr->params, "presence", &val) &&
		    0 == pl_strcasecmp(&val, "p2p")) {
			int expires = 600;
			if (msg_param_decode(&addr->params, "presence_expires", &val) == 0) {
				expires = pl_u32(&val);
				if (expires > 72 * 3600)
					expires = 600;
			}
			err |= presence_alloc(le->data, expires);
		}
	}

	DEBUG_INFO("Subscribing to %u contacts\n", list_count(&presencel));

	return err;
}


void subscriber_close(void)
{
	list_flush(&presencel);
}
