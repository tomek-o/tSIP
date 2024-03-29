/**
 * @file reg.c  Register Client
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <baresip.h>
#include "core.h"


#define DEBUG_MODULE "reg"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


/** Register client */
struct reg {
	struct le le;                /**< Linked list element                */
	struct ua *ua;               /**< Pointer to parent UA object        */
	struct sipreg *sipreg;       /**< SIP Register client                */
	int id;                      /**< Registration ID (for SIP outbound) */

	/* status: */
	uint16_t scode;              /**< Registration status code           */
	char *srv;                   /**< SIP Server id                      */
	int af;                      /**< Cached address family for SIP conn */
};


static void destructor(void *arg)
{
	struct reg *reg = arg;

	list_unlink(&reg->le);
	mem_deref(reg->sipreg);
	mem_deref(reg->srv);
}


static int sipmsg_fd(const struct sip_msg *msg)
{
	if (!msg)
		return -1;

	switch (msg->tp) {

	case SIP_TRANSP_UDP:
		return udp_sock_fd(msg->sock, AF_UNSPEC);

	case SIP_TRANSP_TCP:
	case SIP_TRANSP_TLS:
		return tcp_conn_fd(sip_msg_tcpconn(msg));

	default:
		return -1;
	}
}


static int sipmsg_af(const struct sip_msg *msg)
{
	struct sa laddr;
	int err = 0;

	if (!msg)
		return AF_UNSPEC;

	switch (msg->tp) {

	case SIP_TRANSP_UDP:
		err = udp_local_get(msg->sock, &laddr);
		break;

	case SIP_TRANSP_TCP:
	case SIP_TRANSP_TLS:
		err = tcp_conn_local_get(sip_msg_tcpconn(msg), &laddr);
		break;

	default:
		return AF_UNSPEC;
	}

	return err ? AF_UNSPEC : sa_af(&laddr);
}


static const char *af_name(int af)
{
	switch (af) {

	case AF_INET:  return "v4";
	case AF_INET6: return "v6";
	default:       return "v?";
	}
}


static int sip_auth_handler(char **username, char **password,
			    const char *realm, void *arg)
{
	struct account *acc = arg;
	return account_auth(acc, username, password, realm);
}


static void register_handler(int err, const struct sip_msg *msg, void *arg)
{
	struct reg *reg = arg;
	const struct sip_hdr *hdr;

	if (err) {
		DEBUG_WARNING("%s: Register: %m\n", ua_aor(reg->ua), err);

		reg->scode = 999;

		ua_event(reg->ua, UA_EVENT_REGISTER_FAIL, NULL, "%m", err);
		return;
	}

	hdr = sip_msg_hdr(msg, SIP_HDR_SERVER);
	if (hdr) {
		reg->srv = mem_deref(reg->srv);
		(void)pl_strdup(&reg->srv, &hdr->val);
	}

	if (200 <= msg->scode && msg->scode <= 299) {

		uint32_t n_bindings;

		n_bindings = sip_msg_hdr_count(msg, SIP_HDR_CONTACT);
		reg->af    = sipmsg_af(msg);

		if (msg->scode != reg->scode) {
			ua_printf(reg->ua, "Registration {%d/%s/%s} %u %r (%s)"
				  " [%u binding%s]\n",
				  reg->id, sip_transp_name(msg->tp),
				  af_name(reg->af), msg->scode, &msg->reason,
				  reg->srv, n_bindings,
				  1==n_bindings?"":"s");
		}

		reg->scode = msg->scode;

		ua_event(reg->ua, UA_EVENT_REGISTER_OK, NULL, "%u %r",
			 msg->scode, &msg->reason);
	}
	else if (msg->scode >= 300) {

		DEBUG_WARNING("Registration to %s: %u %r (%s)\n", ua_aor(reg->ua),
			      msg->scode, &msg->reason, reg->srv);

		reg->scode = msg->scode;

		ua_event(reg->ua, UA_EVENT_REGISTER_FAIL, NULL, "%u %r",
			 msg->scode, &msg->reason);
	}
}


int reg_add(struct list *lst, struct ua *ua, int regid)
{
	struct reg *reg;

	if (!lst || !ua)
		return EINVAL;

	reg = mem_zalloc(sizeof(*reg), destructor);
	if (!reg)
		return ENOMEM;

	reg->ua    = ua;
	reg->id    = regid;

	list_append(lst, &reg->le, reg);

	return 0;
}


int reg_register(struct reg *reg, const char *reg_uri, const char *params,
		 uint32_t regint, const char *outbound)
{
	const char *routev[1];
	int err;

	if (!reg || !reg_uri)
		return EINVAL;

	reg->scode = 0;
	routev[0] = outbound;

	reg->sipreg = mem_deref(reg->sipreg);
	err = sipreg_register(&reg->sipreg, uag_sip(), reg_uri,
			      ua_aor(reg->ua), ua_aor(reg->ua),
			      regint, ua_cuser(reg->ua),
			      routev[0] ? routev : NULL,
			      routev[0] ? 1 : 0,
			      reg->id,
			      sip_auth_handler, ua_prm(reg->ua), true,
			      register_handler, reg,
			      params[0] ? &params[1] : NULL,
			      "Allow: %s\r\n", uag_allowed_methods());
	if (err)
		return err;

	return 0;
}


void reg_unregister(struct reg *reg)
{
	if (!reg)
		return;

	reg->scode = 0;
	reg->af    = 0;

	reg->sipreg = mem_deref(reg->sipreg);
}


bool reg_isok(const struct reg *reg)
{
	if (!reg)
		return false;

	return 200 <= reg->scode && reg->scode <= 299;
}


static const char *print_scode(uint16_t scode)
{
	if (0 == scode)        return "\x1b[33m" "zzz" "\x1b[;m";
	else if (200 == scode) return "\x1b[32m" "OK " "\x1b[;m";
	else                   return "\x1b[31m" "ERR" "\x1b[;m";
}


int reg_debug(struct re_printf *pf, const struct reg *reg)
{
	int err = 0;

	if (!reg)
		return 0;

	err |= re_hprintf(pf, "\nRegister client:\n");
	err |= re_hprintf(pf, " id:     %d\n", reg->id);
	err |= re_hprintf(pf, " scode:  %u (%s)\n",
			  reg->scode, print_scode(reg->scode));
	err |= re_hprintf(pf, " srv:    %s\n", reg->srv);

	return err;
}


int reg_status(struct re_printf *pf, const struct reg *reg)
{
	if (!reg)
		return 0;

	return re_hprintf(pf, " %s %s", print_scode(reg->scode), reg->srv);
}
