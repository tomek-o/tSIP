/**
 * @file notifier.c Presence notifier
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <string.h>
#include <re.h>
#include <baresip.h>
#include "presence.h"

#define DEBUG_MODULE "presence-notifier"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

/*
 * Notifier - other people are subscribing to the status of our AOR.
 * we must maintain a list of active notifications. we receive a SUBSCRIBE
 * message from peer, and send NOTIFY to all peers when the Status changes
 *
 * SUBSCRIBE requests are dispatched by the core (see sub_server_register()):
 * the server is disabled by default and only allowed addresses reach this module.
 */

enum { MAX_NOTIFIERS = 64 };

struct notifier {
	struct le le;
	struct sipnot *not;
	struct ua *ua;
	char *entity;			/**< URI of the subscribed user (To of SUBSCRIBE) */
};

static struct list notifierl;
static presence_server_status_h *status_h = NULL;


static const char *presence_status_str(enum presence_status st)
{
	switch (st) {

	case PRESENCE_OPEN:   return "open";
	case PRESENCE_BUSY:   return "open";
	case PRESENCE_CLOSED: return "closed";
	default: return "closed";
	}
}


/** Append text escaped for XML content / attribute value */
static int xml_escape(struct mbuf *mb, const char *str)
{
	int err = 0;

	for (; str && *str && !err; ++str) {
		switch (*str) {
		case '&':  err = mbuf_write_str(mb, "&amp;");  break;
		case '<':  err = mbuf_write_str(mb, "&lt;");   break;
		case '>':  err = mbuf_write_str(mb, "&gt;");   break;
		case '"':  err = mbuf_write_str(mb, "&quot;"); break;
		case '\'': err = mbuf_write_str(mb, "&apos;"); break;
		default:   err = mbuf_write_u8(mb, (uint8_t)*str); break;
		}
	}

	return err;
}


/** Number of calls of the UA, not counting the one that is just closing */
static unsigned int active_calls(const struct ua *ua, const struct call *closing)
{
	unsigned int n = 0;
	struct le *le;

	for (le = list_head(ua_calls(ua)); le; le = le->next) {
		if (le->data != closing)
			++n;
	}

	return n;
}


static int notify(struct notifier *not, const struct call *closing)
{
	const char *aor = not->entity ? not->entity : ua_aor(not->ua);
	enum presence_status status = PRESENCE_OPEN;
	char note[128] = "";
	struct mbuf *mb;
	int err;

	if (status_h)
		status_h(not->ua, &status, note, sizeof(note));

	mb = mbuf_alloc(1024);
	if (!mb)
		return ENOMEM;

	err  = mbuf_write_str(mb,
	"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\r\n"
	"<presence xmlns=\"urn:ietf:params:xml:ns:pidf\"\r\n"
	"    xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\"\r\n"
	"    xmlns:rpid=\"urn:ietf:params:xml:ns:pidf:rpid\"\r\n"
	"    entity=\"");
	err |= xml_escape(mb, aor);
	err |= mbuf_write_str(mb, "\">\r\n  <dm:person id=\"p4159\"><rpid:activities>");
	if (active_calls(not->ua, closing) > 0)
		err |= mbuf_write_str(mb, "<rpid:on-the-phone/>");
	err |= mbuf_write_str(mb,
	"</rpid:activities></dm:person>\r\n"
	"  <tuple id=\"t4109\">\r\n"
	"    <status>\r\n"
	"      <basic>");
	err |= mbuf_write_str(mb, presence_status_str(status));
	err |= mbuf_write_str(mb, "</basic>\r\n    </status>\r\n    <contact>");
	err |= xml_escape(mb, aor);
	err |= mbuf_write_str(mb, "</contact>\r\n");
	if (note[0]) {
		err |= mbuf_write_str(mb, "    <note>");
		err |= xml_escape(mb, note);
		err |= mbuf_write_str(mb, "</note>\r\n");
	}
	err |= mbuf_write_str(mb, "  </tuple>\r\n</presence>\r\n");
	if (err)
		goto out;

	mb->pos = 0;

	err = sipevent_notify(not->not, mb, SIPEVENT_ACTIVE, 0, 0);
	if (err) {
		DEBUG_WARNING("presence: notify to %s failed (%m)\n", aor, err);
	}

 out:
	mem_deref(mb);
	return err;
}


static void sipnot_close_handler(int err, const struct sip_msg *msg,
				 void *arg)
{
	struct notifier *not = arg;

	if (err) {
		DEBUG_INFO("presence: notifier closed (%m)\n", err);
	}
	else if (msg) {
		DEBUG_INFO("presence: notifier closed (%u %r)\n",
		     msg->scode, &msg->reason);
	}

	not = mem_deref(not);
}


static void destructor(void *arg)
{
	struct notifier *not = arg;

	list_unlink(&not->le);
	mem_deref(not->not);
	mem_deref(not->ua);
	mem_deref(not->entity);
}


static int auth_handler(char **username, char **password,
			const char *realm, void *arg)
{
	return account_auth(arg, username, password, realm);
}


static int notifier_alloc(struct notifier **notp,
			  const struct sip_msg *msg,
			  const struct sipevent_event *se, struct ua *ua)
{
	struct notifier *not;
	int err;

	if (!msg || !se)
		return EINVAL;

	not = mem_zalloc(sizeof(*not), destructor);
	if (!not)
		return ENOMEM;

	not->ua   = mem_ref(ua);
	(void)pl_strdup(&not->entity, &msg->to.auri);

	err = sipevent_accept(&not->not, uag_sipevent_sock(), msg, NULL, se, 200, "OK",
			      60, 600, 3600,
			      ua_cuser(not->ua), "application/pidf+xml",
			      auth_handler, ua_prm(not->ua), true,
			      sipnot_close_handler, not, NULL);
	if (err) {
		DEBUG_WARNING("presence: sipevent_accept failed: %m\n", err);
		goto out;
	}

	list_append(&notifierl, &not->le, not);

 out:
	if (err)
		mem_deref(not);
	else if (notp)
		*notp = not;

	return err;
}


static int notifier_add(const struct sip_msg *msg, struct ua *ua)
{
	const struct sip_hdr *hdr;
	struct sipevent_event se;
	struct notifier *not;
	int err;

	if (list_count(&notifierl) >= MAX_NOTIFIERS) {
		DEBUG_WARNING("presence: too many subscriptions\n");
		return ENOMEM;
	}

	hdr = sip_msg_hdr(msg, SIP_HDR_EVENT);
	if (!hdr)
		return EPROTO;

	err = sipevent_event_decode(&se, &hdr->val);
	if (err)
		return err;

	if (pl_strcasecmp(&se.event, "presence")) {
		DEBUG_INFO("presence: unexpected event '%r'\n", &se.event);
		return EPROTO;
	}

	err = notifier_alloc(&not, msg, &se, ua);
	if (err)
		return err;

	DEBUG_NOTICE("presence: subscription from %J accepted (%u active)\n",
		     &msg->src, list_count(&notifierl));

	(void)notify(not, NULL);

	return 0;
}


void presence_server_set_status_handler(presence_server_status_h *h)
{
	status_h = h;
}


void presence_server_notify_changed(const struct ua *ua)
{
	struct le *le;

	for (le = notifierl.head; le; le = le->next) {

		struct notifier *not = le->data;

		if (!ua || not->ua == ua)
			(void)notify(not, NULL);
	}
}


static bool sub_handler(const struct sip_msg *msg, struct ua *ua, void *arg)
{
	(void)arg;

	if (notifier_add(msg, ua))
		(void)sip_treply(NULL, uag_sip(), msg, 400, "Bad Presence");

	return true;
}


/** Call state changes: "on the phone" activity */
static void event_handler(struct ua *ua, enum ua_event ev,
			  struct call *call, const char *prm, void *arg)
{
	struct le *le;

	(void)prm;
	(void)arg;

	switch (ev) {

	case UA_EVENT_CALL_INCOMING:
	case UA_EVENT_CALL_OUTGOING:
	case UA_EVENT_CALL_ESTABLISHED:
	case UA_EVENT_CALL_CLOSED:
		break;

	default:
		return;
	}

	for (le = notifierl.head; le; le = le->next) {

		struct notifier *not = le->data;

		if (not->ua == ua)
			(void)notify(not, ev == UA_EVENT_CALL_CLOSED ? call : NULL);
	}
}


int presence_notifier_init(void)
{
	int err;

	err = sub_server_register("presence", sub_handler, NULL);
	if (err)
		return err;

	return uag_event_register(event_handler, NULL);
}


void presence_notifier_close(void)
{
	uag_event_unregister(event_handler);
	sub_server_unregister(sub_handler);
	list_flush(&notifierl);
}
