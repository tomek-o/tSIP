/**
 * @file dialog_info_notifier.c  Dialog-info notifier (RFC 4235)
 *
 * Other users (phones, other tSIP instances) subscribe to the dialog state of our account (BLF)
 * and get NOTIFY with state of all our calls (one dialog element per call) every time a call changes.
 * Useful in peer-to-peer mode when there is no PBX that provides this function.
 *
 * SUBSCRIBE requests are dispatched by the core (see sub_server_register()): the server is disabled
 * by default and only allowed addresses reach this module.
 */
#include <string.h>
#include <re.h>
#include <baresip.h>
#include "dialog_info.h"

#define DEBUG_MODULE "dialog-info-notifier"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

enum {
	MAX_NOTIFIERS = 64,
	MAX_DIALOGS = 16	/* calls reported in single NOTIFY */
};

struct notifier {
	struct le le;
	struct sipnot *not;
	struct ua *ua;
	char *entity;			/**< URI of the subscribed user (To of SUBSCRIBE) */
	unsigned int version;		/**< dialog-info version, incremented with every NOTIFY */
};

static struct list notifierl;


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


static int write_attr(struct mbuf *mb, const char *name, const char *value)
{
	int err;

	if (!value || !value[0])
		return 0;

	err  = mbuf_printf(mb, " %s=\"", name);
	err |= xml_escape(mb, value);
	err |= mbuf_write_str(mb, "\"");

	return err;
}


static const char *state_str(enum call_dialog_state st)
{
	switch (st) {

	case CALL_DLG_TRYING:    return "trying";
	case CALL_DLG_EARLY:     return "early";
	case CALL_DLG_CONFIRMED: return "confirmed";
	default:                 return "terminated";
	}
}


/** One dialog element for call */
static int write_dialog(struct mbuf *mb, struct call *call, const struct call_dialog_info *info,
			const char *aor, int identity_mode)
{
	const char *remote_uri = info->peer_uri;
	const char *remote_name = info->peer_name;
	const char *dlg_id;
	char id[32];
	int err;

	if (identity_mode == 3 && info->pai_uri && info->pai_uri[0]) {
		remote_uri = info->pai_uri;
		remote_name = info->pai_name;
	}

	/* stable for the lifetime of the call (call-id is not known before the INVITE is sent
	   and is the same for both legs of a call between two accounts of one application) */
	re_snprintf(id, sizeof(id), "d%lx", (unsigned long)(size_t)call);
	dlg_id = id;

	err  = mbuf_write_str(mb, "  <dialog");
	err |= write_attr(mb, "id", dlg_id);
	err |= write_attr(mb, "call-id", info->callid);
	err |= write_attr(mb, "local-tag", info->local_tag);
	err |= write_attr(mb, "remote-tag", info->remote_tag);
	err |= mbuf_printf(mb, " direction=\"%s\">\r\n", info->incoming ? "recipient" : "initiator");
	err |= mbuf_printf(mb, "    <state>%s</state>\r\n", state_str(info->state));

	err |= mbuf_write_str(mb, "    <local><identity>");
	err |= xml_escape(mb, (info->local_uri && info->local_uri[0]) ? info->local_uri : aor);
	err |= mbuf_write_str(mb, "</identity></local>\r\n");

	if (identity_mode > 0 && remote_uri && remote_uri[0]) {
		err |= mbuf_write_str(mb, "    <remote><identity");
		if (identity_mode >= 2)
			err |= write_attr(mb, "display", remote_name);
		err |= mbuf_write_str(mb, ">");
		err |= xml_escape(mb, remote_uri);
		err |= mbuf_write_str(mb, "</identity></remote>\r\n");
	}

	err |= mbuf_write_str(mb, "  </dialog>\r\n");

	return err;
}


/** Send full state of all calls of the UA; closing call (if any) is not reported */
static int notify(struct notifier *not, const struct call *closing)
{
	const char *aor = not->entity ? not->entity : ua_aor(not->ua);
	const int identity_mode = conf_config()->subsrv.dialog_identity;
	unsigned int n = 0;
	struct le *le;
	struct mbuf *mb;
	int err;

	mb = mbuf_alloc(2048);
	if (!mb)
		return ENOMEM;

	err  = mbuf_printf(mb,
		"<?xml version=\"1.0\"?>\r\n"
		"<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"%u\" state=\"full\" entity=\"",
		not->version);
	err |= xml_escape(mb, aor);
	err |= mbuf_write_str(mb, "\">\r\n");

	for (le = list_head(ua_calls(not->ua)); le && n < MAX_DIALOGS && !err; le = le->next) {

		struct call *call = le->data;
		struct call_dialog_info info;

		if (call == closing)
			continue;

		if (call_dialog_info(call, &info) || info.state == CALL_DLG_NONE)
			continue;

		err |= write_dialog(mb, call, &info, aor, identity_mode);
		++n;
	}

	err |= mbuf_write_str(mb, "</dialog-info>\r\n");
	if (err)
		goto out;

	mb->pos = 0;

	err = sipevent_notify(not->not, mb, SIPEVENT_ACTIVE, 0, 0);
	if (err) {
		DEBUG_WARNING("dialog-info: notify for %s failed (%m)\n", aor, err);
	}
	else {
		++not->version;
	}

 out:
	mem_deref(mb);
	return err;
}


static void sipnot_close_handler(int err, const struct sip_msg *msg, void *arg)
{
	struct notifier *not = arg;

	if (err) {
		DEBUG_INFO("dialog-info: notifier closed (%m)\n", err);
	}
	else if (msg) {
		DEBUG_INFO("dialog-info: notifier closed (%u %r)\n", msg->scode, &msg->reason);
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


static int auth_handler(char **username, char **password, const char *realm, void *arg)
{
	return account_auth(arg, username, password, realm);
}


static int notifier_add(const struct sip_msg *msg, struct ua *ua)
{
	const struct sip_hdr *hdr;
	struct sipevent_event se;
	struct notifier *not;
	int err;

	if (list_count(&notifierl) >= MAX_NOTIFIERS) {
		DEBUG_WARNING("dialog-info: too many subscriptions\n");
		return ENOMEM;
	}

	hdr = sip_msg_hdr(msg, SIP_HDR_EVENT);
	if (!hdr)
		return EPROTO;

	err = sipevent_event_decode(&se, &hdr->val);
	if (err)
		return err;

	if (pl_strcasecmp(&se.event, "dialog")) {
		DEBUG_INFO("dialog-info: unexpected event '%r'\n", &se.event);
		return EPROTO;
	}

	not = mem_zalloc(sizeof(*not), destructor);
	if (!not)
		return ENOMEM;

	not->ua = mem_ref(ua);
	(void)pl_strdup(&not->entity, &msg->to.auri);

	err = sipevent_accept(&not->not, uag_sipevent_sock(), msg, NULL, &se, 200, "OK",
			      60, 600, 3600,
			      ua_cuser(not->ua), "application/dialog-info+xml",
			      auth_handler, ua_prm(not->ua), true,
			      sipnot_close_handler, not, NULL);
	if (err) {
		DEBUG_WARNING("dialog-info: sipevent_accept failed: %m\n", err);
		mem_deref(not);
		return err;
	}

	list_append(&notifierl, &not->le, not);

	DEBUG_NOTICE("dialog-info: subscription from %J accepted (%u active)\n",
		     &msg->src, list_count(&notifierl));

	(void)notify(not, NULL);

	return 0;
}


static bool sub_handler(const struct sip_msg *msg, struct ua *ua, void *arg)
{
	(void)arg;

	if (notifier_add(msg, ua))
		(void)sip_treply(NULL, uag_sip(), msg, 400, "Bad Dialog Subscription");

	return true;
}


/** Call state changes: new NOTIFY with state of all calls */
static void event_handler(struct ua *ua, enum ua_event ev, struct call *call, const char *prm, void *arg)
{
	struct le *le;

	(void)prm;
	(void)arg;

	switch (ev) {

	case UA_EVENT_CALL_INCOMING:
	case UA_EVENT_CALL_OUTGOING:
	case UA_EVENT_CALL_TRYING:
	case UA_EVENT_CALL_RINGING:
	case UA_EVENT_CALL_PROGRESS:
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


int dialog_info_notifier_init(void)
{
	int err;

	err = sub_server_register("dialog", sub_handler, NULL);
	if (err)
		return err;

	return uag_event_register(event_handler, NULL);
}


void dialog_info_notifier_close(void)
{
	uag_event_unregister(event_handler);
	sub_server_unregister(sub_handler);
	list_flush(&notifierl);
}
