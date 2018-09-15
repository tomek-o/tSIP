/** \file
*/
#include <re.h>
#include <baresip.h>
#include "dialog_info.h"

#define DEBUG_MODULE "dialog-info"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


struct dialog_info {
	struct le le;
	struct sipsub *sub;
	struct tmr tmr;
	enum dialog_info_status status;
	unsigned failc;
	struct contact *contact;
};

static struct list dialog_infol;


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
	case 3:  return 3600;
	default: return 86400;
	}
}

static const char* STR_EARLY = "early";
static const char* STR_CONFIRMED = "confirmed";
static const char* STR_TERMINATED = "terminated";

static void notify_handler(struct sip *sip, const struct sip_msg *msg,
			   void *arg)
{
	enum dialog_info_status status = DIALOG_INFO_UNKNOWN;
	struct dialog_info *dlg_info = arg;
	const struct sip_hdr *hdr;
	struct pl pl;

	dlg_info->failc = 0;

	hdr = sip_msg_hdr(msg, SIP_HDR_CONTENT_TYPE);
	if (!hdr || 0 != pl_strcasecmp(&hdr->val, "application/dialog-info+xml")) {

		if (hdr)
			DEBUG_WARNING("dialog-info: unsupported content-type: '%r'\n",
				&hdr->val);

		sip_treplyf(NULL, NULL, sip, msg, false,
			    415, "Unsupported Media Type",
			    "Accept: application/dialog-info+xml\r\n"
			    "Content-Length: 0\r\n"
				"\r\n");
		return;
	}

	/** \note Example notify body:

	<?xml version="1.0" encoding="UTF-8"?>
	<dialog-info xmlns="urn:ietf:params:xml:ns:dialog-info" version="0" state="full" entity="sip:%23300@192.168.0.176">
	 <dialog id="sip:%23300@192.168.0.176">
	  <state>early</state>
	 </dialog>
	</dialog-info>

	*/

	/** \todo More precise matching for dialog-info body */
	if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), STR_EARLY)) {
		status = DIALOG_INFO_EARLY;
	} else if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), STR_CONFIRMED)) {
		status = DIALOG_INFO_CONFIRMED;
	} else if (!re_regex((const char *)mbuf_buf(msg->mb), mbuf_get_left(msg->mb), STR_TERMINATED)) {
		status = DIALOG_INFO_TERMINATED;
	} else {
		status = DIALOG_INFO_UNKNOWN;
	}

	(void)sip_treply(NULL, sip, msg, 200, "OK");

	contact_set_dialog_info(dlg_info->contact, status);
}


static void close_handler(int err, const struct sip_msg *msg,
			  const struct sipevent_substate *substate, void *arg)
{
	struct dialog_info *dlg_info = arg;
	uint32_t wait;

	dlg_info->sub = mem_deref(dlg_info->sub);

	DEBUG_INFO("dialog-info: subscriber closed <%r>: ",
	     &contact_addr(dlg_info->contact)->auri);

	if (substate) {
		DEBUG_INFO("%s", sipevent_reason_name(substate->reason));
		wait = wait_term(substate);
	}
	else if (msg) {
		DEBUG_INFO("%u %r", msg->scode, &msg->reason);
		wait = wait_fail(++dlg_info->failc);
	}
	else {
		DEBUG_INFO("%m", err);
		wait = wait_fail(++dlg_info->failc);
	}

	DEBUG_INFO("; will retry in %u secs (failc=%u)\n", wait, dlg_info->failc);

	tmr_start(&dlg_info->tmr, wait * 1000, tmr_handler, dlg_info);

	contact_set_dialog_info(dlg_info->contact, DIALOG_INFO_UNKNOWN);
}


static void destructor(void *arg)
{
	struct dialog_info *dlg_info = arg;

	list_unlink(&dlg_info->le);
	tmr_cancel(&dlg_info->tmr);
	mem_deref(dlg_info->contact);
	mem_deref(dlg_info->sub);
}


static int auth_handler(char **username, char **password,
			const char *realm, void *arg)
{
	return account_auth(arg, username, password, realm);
}


static int subscribe(struct dialog_info *dlg_info)
{
	const char *routev[1];
	struct ua *ua;
	char uri[256];
	int err;

	/* We use the first UA */
	ua = uag_find_aor(NULL);
	if (!ua) {
		DEBUG_WARNING("dialog-info: no UA found\n");
		return ENOENT;
	}

	pl_strcpy(&contact_addr(dlg_info->contact)->auri, uri, sizeof(uri));

	routev[0] = ua_outbound(ua);

	err = sipevent_subscribe(&dlg_info->sub, uag_sipevent_sock(), uri, NULL,
				 ua_aor(ua), "dialog", "application/dialog-info+xml", NULL, 600,
				 ua_cuser(ua), routev, routev[0] ? 1 : 0,
				 auth_handler, ua_prm(ua), true, NULL,
				 notify_handler, close_handler, dlg_info,
				 "%H", ua_print_supported, ua);
	if (err) {
		DEBUG_WARNING("dialog-info: sipevent_subscribe failed: %m\n", err);
	}

	return err;
}


static void tmr_handler(void *arg)
{
	struct dialog_info *dlg_info = arg;

	if (subscribe(dlg_info)) {
		tmr_start(&dlg_info->tmr, wait_fail(++dlg_info->failc) * 1000,
			  tmr_handler, dlg_info);
	}
}


static int dialog_info_alloc(struct contact *contact)
{
	struct dialog_info *dlg_info;

	dlg_info = mem_zalloc(sizeof(*dlg_info), destructor);
	if (!dlg_info)
		return ENOMEM;

	dlg_info->status  = DIALOG_INFO_UNKNOWN;
	dlg_info->contact = mem_ref(contact);

	tmr_init(&dlg_info->tmr);
	tmr_start(&dlg_info->tmr, 1000, tmr_handler, dlg_info);

	list_append(&dialog_infol, &dlg_info->le, dlg_info);

	return 0;
}


int dialog_info_subscriber_init(void)
{
	struct le *le;
	int err = 0;

	for (le = list_head(contact_list()); le; le = le->next) {

		struct contact *c = le->data;
		struct sip_addr *addr = contact_addr(c);
		struct pl val;

		if (0 == sip_param_decode(&addr->params, "dlginfo", &val) &&
		    0 == pl_strcasecmp(&val, "p2p")) {

			err |= dialog_info_alloc(le->data);
		}
	}

	DEBUG_INFO("Subscribing dialog-info to %u contacts\n", list_count(&dialog_infol));

	return err;
}


void dialog_info_subscriber_close(void)
{
	list_flush(&dialog_infol);
}
