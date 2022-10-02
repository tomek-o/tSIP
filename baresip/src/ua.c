/**
 * @file src/ua.c  User-Agent
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <string.h>
#include <re.h>
#include <baresip.h>
#include "core.h"


#define DEBUG_MODULE "ua"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

/** Magic number */
#define MAGIC 0x0a0a0a0a
#include "magic.h"


enum {
	MAX_CALLS       =    1
};


/** Defines a SIP User Agent object */
struct ua {
	MAGIC_DECL                   /**< Magic number for struct ua         */
	struct ua **uap;             /**< Pointer to application's ua        */
	struct le le;                /**< Linked list element                */
	struct account *acc;         /**< Account Parameters                 */
	struct list regl;            /**< List of Register clients           */
	struct list calls;           /**< List of active calls (struct call) */
	struct play *play;           /**< Playback of ringtones etc. (basic) */
	struct play *play2;          /**< Playback of ringtones etc. #2 (incoming MESSAGE)     */
	struct pl extensionv[8];     /**< Vector of SIP extensions           */
	size_t    extensionc;        /**< Number of SIP extensions           */
	char *cuser;                 /**< SIP Contact username               */
	int af;                      /**< Preferred Address Family           */
};

struct ua_eh {
	struct le le;
	ua_event_h *h;
	void *arg;
};

static struct {
	struct config_sip *cfg;        /**< SIP configuration               */
	struct list ual;               /**< List of User-Agents (struct ua) */
	struct list ehl;               /**< Event handlers (struct ua_eh)   */
	struct sip *sip;               /**< SIP Stack                       */
	struct sip_lsnr *lsnr;         /**< SIP Listener                    */
	struct sipsess_sock *sock;     /**< SIP Session socket              */
	struct sipevent_sock *evsock;  /**< SIP Event socket                */
	uint32_t transports;           /**< Supported transports mask       */
	bool prefer_ipv6;              /**< Force IPv6 transport            */
#ifdef USE_TLS
	struct tls *tls;               /**< TLS Context                     */
#endif
} uag = {
	NULL,
	LIST_INIT,
	LIST_INIT,
	NULL,
	NULL,
	NULL,
	NULL,
	0,
	false,
#ifdef USE_TLS
	NULL,
#endif
};


/* prototypes */
static int  ua_call_alloc(struct call **callp, struct ua *ua,
			  enum vidmode vidmode, const struct sip_msg *msg,
			  struct call *xcall, const char *local_uri);


/* This function is called when all SIP transactions are done */
static void exit_handler(void *arg)
{
	(void)arg;

	re_cancel();
}


void ua_printf(const struct ua *ua, const char *fmt, ...)
{
	va_list ap;

	if (!ua)
		return;

	va_start(ap, fmt);
	(void)re_printf("%r@%r: ",
			 &ua->acc->luri.user, &ua->acc->luri.host);
	(void)re_vprintf(fmt, ap);
	va_end(ap);
}


/**
 * Send a User-Agent event to all UA event handlers
 *
 * @param ua   User-Agent object (optional)
 * @param ev   User-agent event
 * @param call Call object (optional)
 * @param fmt  Formatted arguments
 * @param ...  Variable arguments
 */
void ua_event(struct ua *ua, enum ua_event ev, struct call *call,
	      const char *fmt, ...)
{
	struct le *le;
	char buf[256];
	va_list ap;

	if (!ua)
		return;

	va_start(ap, fmt);
	(void)re_vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	/* send event to all clients */
	for (le = uag.ehl.head; le; le = le->next) {

		struct ua_eh *eh = le->data;

		eh->h(ua, ev, call, buf, eh->arg);
	}
}


/**
 * Start registration of a User-Agent
 *
 * @param ua User-Agent
 *
 * @return 0 if success, otherwise errorcode
 */
static int ua_register(struct ua *ua)
{
	struct account *acc;
	struct le *le;
	struct uri uri;
	char reg_uri[128];
	char params[256] = "";
	unsigned i;
	int err;

	if (!ua)
		return EINVAL;

	acc = ua->acc;
	uri = ua->acc->luri;
	uri.user = uri.password = pl_null;
	if (re_snprintf(reg_uri, sizeof(reg_uri), "%H", uri_encode, &uri) < 0)
		return ENOMEM;

	if (str_isset(uag.cfg->uuid)) {
		if (re_snprintf(params, sizeof(params),
				";+sip.instance=\"<urn:uuid:%s>\"",
				uag.cfg->uuid) < 0)
			return ENOMEM;
	}

	if (acc->regq) {
		if (re_snprintf(&params[strlen(params)],
				sizeof(params) - strlen(params),
				";q=%s", acc->regq) < 0)
			return ENOMEM;
	}

	if (acc->mnat && acc->mnat->ftag) {
		if (re_snprintf(&params[strlen(params)],
				sizeof(params) - strlen(params),
				";%s", acc->mnat->ftag) < 0)
			return ENOMEM;
	}

	ua_event(ua, UA_EVENT_REGISTERING, NULL, NULL);

	for (le = ua->regl.head, i=0; le; le = le->next, i++) {
		struct reg *reg = le->data;

		err = reg_register(reg, reg_uri, params,
				   acc->regint, acc->outbound[i]);
		if (err) {
			DEBUG_WARNING("SIP register failed: %m\n", err);
			return err;
		}
	}

	return 0;
}


/**
 * Unregister all Register clients of a User-Agent
 *
 * @param ua User-Agent
 */
void ua_unregister(struct ua *ua)
{
	struct le *le;

	if (!ua)
		return;

	ua_event(ua, UA_EVENT_UNREGISTERING, NULL, NULL);

	for (le = ua->regl.head; le; le = le->next) {
		struct reg *reg = le->data;

		reg_unregister(reg);
	}

	ua_event(ua, UA_EVENT_UNREGISTER_OK, NULL, NULL);
}


bool ua_isregistered(const struct ua *ua)
{
	struct le *le;

	if (!ua)
		return false;

	for (le = ua->regl.head; le; le = le->next) {

		const struct reg *reg = le->data;

		/* it is enough if one of the registrations work */
		if (reg_isok(reg))
			return true;
	}

	return false;
}

unsigned int ua_regint(const struct ua *ua)
{
	return ua->acc->regint;
}

int ua_reregister(struct ua *ua)
{
	if (!ua)
		return EINVAL;
	if (ua->acc->regint == 0)
	{
		return -1;
	}
	return ua_register(ua);
}

int ua_play_file(struct ua *ua, const char *audio_mod, const char *audio_dev, const char *filename, int repeat, bool loop_without_silence)
{
	if (!ua)
		return EINVAL;
	return play_file(&ua->play, audio_mod, audio_dev, filename, repeat, loop_without_silence);
}

int ua_play_stop(struct ua *ua)
{
	if (!ua)
		return EINVAL;
	ua->play = mem_deref(ua->play);
	return 0;
}

int ua_play_file2(struct ua *ua, const char *audio_mod, const char *audio_dev, const char *filename)
{
	if (!ua)
		return EINVAL;
	return play_file(&ua->play2, audio_mod, audio_dev, filename, 0, false);
}


static const char *translate_errorcode(uint16_t scode)
{
	switch (scode) {

	case 404: return "notfound.wav";
	case 486: return "busy.wav";
	case 487: return NULL; /* ignore */
	default:  return "error.wav";
	}
}


static void call_event_handler(struct call *call, enum call_event ev,
			       const char *str, void *arg)
{
	struct ua *ua = arg;
	const char *peeruri;
	struct call *call2 = NULL;
	struct config * cfg = conf_config();	
	int err;

	MAGIC_CHECK(ua);

	peeruri = call_peeruri(call);

	/* stop any ringtones */
	ua->play = mem_deref(ua->play);

	switch (ev) {

	case CALL_EVENT_INCOMING:
		switch (ua->acc->answermode) {

		case ANSWERMODE_EARLY:
			(void)call_progress(call);
			break;

		case ANSWERMODE_AUTO:
			(void)call_answer(call, 200, "", "");
			break;

		case ANSWERMODE_MANUAL:
		default:
		#if 0	// ring generation moved to upper layer
			if (list_count(&ua->calls) > 1) {
				(void)play_file(&ua->play,
							"callwaiting.wav", 3);
			}
			else {
				/* Alert user */
				(void)play_file(&ua->play, "ring.wav", -1);
			}
        #endif
			ua_event(ua, UA_EVENT_CALL_INCOMING, call, peeruri);
			break;
		}
		break;

	case CALL_EVENT_TRYING:
		ua_event(ua, UA_EVENT_CALL_TRYING, call, peeruri);
		break;

	case CALL_EVENT_RINGING:
		(void)play_file(&ua->play, cfg->audio.alert_mod, cfg->audio.alert_dev, "ringback.wav", -1, false);

		ua_event(ua, UA_EVENT_CALL_RINGING, call, peeruri);
		break;

	case CALL_EVENT_OUTGOING:
		ua_event(ua, UA_EVENT_CALL_OUTGOING, call, peeruri);
		break;

	case CALL_EVENT_PROGRESS:
		ua_printf(ua, "Call in-progress: %s\n", peeruri);
		ua_event(ua, UA_EVENT_CALL_PROGRESS, call, peeruri);
		break;

	case CALL_EVENT_ESTABLISHED:
		ua_printf(ua, "Call established: %s\n", peeruri);
		ua_event(ua, UA_EVENT_CALL_ESTABLISHED, call, peeruri);
		break;

	case CALL_EVENT_CLOSED:
		if (call_scode(call)) {
			const char *tone;
			tone = translate_errorcode(call_scode(call));
			if (tone)
				(void)play_file(&ua->play, cfg->audio.alert_mod, cfg->audio.alert_dev, tone, 1, false);
		}
		ua_event(ua, UA_EVENT_CALL_CLOSED, call, str);
		mem_deref(call);
		break;

	case CALL_EVENT_TRANSFER:

		/*
		 * Create a new call to transfer target.
		 *
		 * NOTE: we will automatically connect a new call to the
		 *       transfer target
		 */

		ua_printf(ua, "transferring call to %s\n", str);

		err = ua_call_alloc(&call2, ua, VIDMODE_ON, NULL, call,
				    call_localuri(call));
		if (!err) {
			struct pl pl;

			pl_set_str(&pl, str);

			err = call_connect(call2, &pl, NULL);
			if (err) {
				DEBUG_WARNING("transfer: connect error: %m\n",
					      err);
			} else {
				ua_event(ua, UA_EVENT_CALL_TRANSFER, call2, str);
            }
		}

		if (err) {
			(void)call_notify_sipfrag(call, 500, "%m", err);
			mem_deref(call2);
		}
		break;

	case CALL_EVENT_TRANSFER_FAILED:
		ua_printf(ua, "Call transfer failed: %s\n", peeruri);
		ua_event(ua, UA_EVENT_CALL_TRANSFER_FAILED, call, peeruri);
		break;

	case CALL_EVENT_REINVITE_RECEIVED:
		ua_event(ua, UA_EVENT_CALL_REINVITE_RECEIVED, call, peeruri);	
		break;

	}
}

static void call_dtmf_handler(struct call *call, char key, void *arg)
{
	struct ua *ua = arg;
	char key_str[2];

	MAGIC_CHECK(ua);

	if (key != '\0') {

		key_str[0] = key;
		key_str[1] = '\0';

		ua_event(ua, UA_EVENT_CALL_DTMF_START, call, key_str);
	}
	else {
		ua_event(ua, UA_EVENT_CALL_DTMF_END, call, NULL);
	}
}

static int ua_call_alloc(struct call **callp, struct ua *ua,
			 enum vidmode vidmode, const struct sip_msg *msg,
			 struct call *xcall, const char *local_uri)
{
	struct call_prm cprm;
	int err;

	if (*callp) {
		DEBUG_WARNING("call_alloc: call is already allocated\n");
		return EALREADY;
	}

	cprm.vidmode = vidmode;
	cprm.af      = ua->af;

	err = call_alloc(callp, conf_config(), &ua->calls,
			  ua->acc->dispname,
			  local_uri ? local_uri : ua->acc->aor,
			  ua->acc, ua, &cprm,
			  msg, xcall, call_event_handler, ua);
	if (err)
		return err;

	call_set_handlers(*callp, NULL, call_dtmf_handler, ua);

	return 0;
}


static void handle_options(struct ua *ua, const struct sip_msg *msg)
{
	struct call *call = NULL;
	struct mbuf *desc = NULL;
	int err;

	err = ua_call_alloc(&call, ua, VIDMODE_ON, NULL, NULL, NULL);
	if (err) {
		(void)sip_treply(NULL, uag.sip, msg, 500, "Call Error");
		return;
	}

	err = call_sdp_get(call, &desc, true);
	if (err)
		goto out;

	err = sip_treplyf(NULL, NULL, uag.sip,
			  msg, true, 200, "OK",
			  "Contact: <sip:%s@%J%s>\r\n"
			  "Content-Type: application/sdp\r\n"
			  "Content-Length: %zu\r\n"
			  "\r\n"
			  "%b",
			  ua->cuser, &msg->dst, sip_transp_param(msg->tp),
			  mbuf_get_left(desc),
			  mbuf_buf(desc),
			  mbuf_get_left(desc));
	if (err) {
		DEBUG_WARNING("options: sip_treplyf: %m\n", err);
	}

 out:
	mem_deref(desc);
	mem_deref(call);
}


static void handle_refer(struct ua *ua, const struct sip_msg *msg)
{
	int err;
	const struct sip_hdr *hdr;
	char refer_target[128];

	/* get the transfer target */
	hdr = sip_msg_hdr(msg, SIP_HDR_REFER_TO);
	if (!hdr) {
		DEBUG_WARNING("bad REFER request from %r\n", &msg->from.auri);
		(void)sip_treply(NULL, NULL, msg, 400, "Missing Refer-To header");
		return;
	}

	err = sip_treply(NULL, uag.sip, msg, 202, "Accepted");

	/*
	If a REFER request is accepted (that is, a 2xx class response is
	returned), the recipient MUST create a subscription and send
	notifications of the status of the refer
	*/
	{
	int TODO__CREATE_SUBSCRIPTION_FOR_OOD_REFER;
    }

	if (err) {
		DEBUG_WARNING("options: sip_treply: %m\n", err);
	}

	(void)re_snprintf(refer_target, sizeof(refer_target), "%r", &hdr->val);
	ua_event(ua, UA_EVENT_CALL_TRANSFER_OOD, NULL, refer_target);
}



static void ua_destructor(void *arg)
{
	struct ua *ua = arg;

	if (ua->uap) {
		*ua->uap = NULL;
		ua->uap = NULL;
	}

	list_unlink(&ua->le);

	ua_event(ua, UA_EVENT_UNREGISTERING, NULL, NULL);

	list_flush(&ua->calls);
	list_flush(&ua->regl);
	mem_deref(ua->play);
	mem_deref(ua->play2);
	mem_deref(ua->cuser);
	mem_deref(ua->acc);
}


static bool request_handler(const struct sip_msg *msg, void *arg)
{
	struct ua *ua;

	(void)arg;

	if (pl_strcmp(&msg->met, "OPTIONS") == 0) {
		ua = uag_find(&msg->uri.user);
		if (!ua) {
			(void)sip_treply(NULL, uag_sip(), msg, 404, "Not Found");
			return true;
		}
		handle_options(ua, msg);
		return true;
	} else if (pl_strcmp(&msg->met, "REFER") == 0) {
		if (pl_isset(&msg->to.tag)) {
			// handling only out-of-dialog REFER here, leaving to session handler
			return false;	
		}
		ua = uag_find(&msg->uri.user);
		if (!ua) {
			(void)sip_treply(NULL, uag_sip(), msg, 404, "Not Found");
			return true;
		}		
		handle_refer(ua, msg);	// 403 Forbidden or 202 Accepted depending on configuration
		return true;
	}

	return false;
}


static void add_extension(struct ua *ua, const char *extension)
{
	struct pl e;

	if (ua->extensionc >= ARRAY_SIZE(ua->extensionv)) {
		DEBUG_WARNING("maximum %u number of SIP extensions\n");
		return;
	}

	pl_set_str(&e, extension);

	ua->extensionv[ua->extensionc++] = e;
}


/**
 * Allocate a SIP User-Agent
 *
 * @param uap   Pointer to allocated User-Agent object
 * @param aor   SIP Address-of-Record (AOR)
 * @param pwd	Password
 * @param cuser Contact user; default "hex" used if NULL or empty string is passed
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_alloc(struct ua **uap, const char *aor, const char *pwd, const char *cuser)
{
	struct ua *ua;
	int err;

	if (!aor)
		return EINVAL;

	ua = mem_zalloc(sizeof(*ua), ua_destructor);
	if (!ua)
		return ENOMEM;

	MAGIC_INIT(ua);

	list_init(&ua->calls);

#if HAVE_INET6
	ua->af   = uag.prefer_ipv6 ? AF_INET6 : AF_INET;
#else
	ua->af   = AF_INET;
#endif

    if (cuser && strlen(cuser)) {
		err = re_sdprintf(&ua->cuser, "%s", cuser);
	} else {
		err = re_sdprintf(&ua->cuser, "%p", ua);
	}
	if (err)
		goto out;

	/* Decode SIP address */

	err = account_alloc(&ua->acc, aor, pwd);
	if (err)
		goto out;

	if (ua->acc->sipnat) {
		ua_printf(ua, "Using sipnat: `%s'\n", ua->acc->sipnat);
	}

	if (ua->acc->mnat) {
		ua_printf(ua, "Using medianat `%s'\n",
			  ua->acc->mnat->id);

		if (0 == str_casecmp(ua->acc->mnat->id, "ice"))
			add_extension(ua, "ice");
	}

	if (ua->acc->menc) {
		ua_printf(ua, "Using media encryption `%s'\n",
			  ua->acc->menc->id);
	}

	/* Register clients */
	if (0 == str_casecmp(ua->acc->sipnat, "outbound")) {

		size_t i;

		add_extension(ua, "path");
		add_extension(ua, "outbound");

		if (!str_isset(uag.cfg->uuid)) {

			DEBUG_WARNING("outbound requires valid UUID!\n");
			err = ENOSYS;
			goto out;
		}

		for (i=0; i<ARRAY_SIZE(ua->acc->outbound); i++) {

			if (ua->acc->outbound[i]) {
				err = reg_add(&ua->regl, ua, (int)i+1);
				if (err)
					break;
			}
		}
	}
	else {
		err = reg_add(&ua->regl, ua, 0);
	}
	if (err)
		goto out;

	list_append(&uag.ual, &ua->le, ua);

	if (ua->acc->regint) {
		err = ua_register(ua);
	}

 out:
	if (err)
		mem_deref(ua);
	else if (uap) {
		*uap = ua;

		ua->uap = uap;
	}

	return err;
}


/**
 * Connect an outgoing call to a given SIP uri
 *
 * @param ua        User-Agent
 * @param callp     Optional pointer to allocated call object
 * @param from_uri  Optional From uri, or NULL for default AOR
 * @param uri       SIP uri to connect to
 * @param params    Optional URI parameters
 * @param vmode     Video mode
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_connect(struct ua *ua, struct call **callp,
	       const char *from_uri, const char *uri,
	       const char *params, enum vidmode vmode, const char* extra_hdr_lines)
{
	struct call *call = NULL;
	struct mbuf *dialbuf;
	struct pl pl;
	struct pl pl_extra_hdr_lines;
	size_t len;
	int err = 0;

	if (!ua || !str_isset(uri))
		return EINVAL;

	len = str_len(uri);

	dialbuf = mbuf_alloc(128);
	if (!dialbuf)
		return ENOMEM;

	if (params)
		err |= mbuf_printf(dialbuf, "<");

	/* Append sip: scheme if missing */
	if (0 != re_regex(uri, len, "sip:"))
		err |= mbuf_printf(dialbuf, "sip:");

	err |= mbuf_write_str(dialbuf, uri);

#if 0
	/* Append domain if missing */
	if (0 != re_regex(uri, len, "[^@]+@[^]+", NULL, NULL)) {
#else
	// above: does not allow to call to sip:ip_address, required user part of URI
	// below: assuming that if sip: is present than domain is present
	if (0 != re_regex(uri, len, "sip:")) {
#endif

#if HAVE_INET6
		if (AF_INET6 == ua->acc->luri.af)
			err |= mbuf_printf(dialbuf, "@[%r]",
					   &ua->acc->luri.host);
		else
#endif
			err |= mbuf_printf(dialbuf, "@%r",
					   &ua->acc->luri.host);

		/* Also append port if specified and not 5060 */
		switch (ua->acc->luri.port) {

		case 0:
		case SIP_PORT:
			break;

		default:
			err |= mbuf_printf(dialbuf, ":%u", ua->acc->luri.port);
			break;
		}
	}

	if (params) {
		err |= mbuf_printf(dialbuf, ";%s", params);
	}

	/* Append any optional URI parameters */
	err |= mbuf_write_pl(dialbuf, &ua->acc->luri.params);

	if (params)
		err |= mbuf_printf(dialbuf, ">");

	if (err)
		goto out;

	err = ua_call_alloc(&call, ua, vmode, NULL, NULL, from_uri);
	if (err)
		goto out;

	pl.p = (char *)dialbuf->buf;
	pl.l = dialbuf->end;

	pl_set_str(&pl_extra_hdr_lines, extra_hdr_lines);

	err = call_connect(call, &pl, (pl.l)?(&pl_extra_hdr_lines):(NULL));

	if (err)
		mem_deref(call);
	else if (callp)
		*callp = call;

 out:
	mem_deref(dialbuf);

	return err;
}


/**
 * Hangup the current call
 *
 * @param ua     User-Agent
 * @param call   Call to hangup, or NULL for current call
 * @param scode  Optional status code
 * @param reason Optional reason
 */
void ua_hangup(struct ua *ua, struct call *call,
	       uint16_t scode, const char *reason)
{
	if (!ua)
		return;

	if (!call) {
		call = ua_call(ua);
		if (!call)
			return;
	}

	ua->play = mem_deref(ua->play);

	(void)call_hangup(call, scode, reason);

	mem_deref(call);
}


/**
 * Answer an incoming call
 *
 * @param ua   User-Agent
 * @param call Call to hangup, or NULL for current call
 * @param audio_mod Audio module for call audio output, empty string for default
 * @param audio_dev Audio device for call audio output, empty string for default
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_answer(struct ua *ua, struct call *call, const char *audio_mod, const char *audio_dev)
{
	if (!ua)
		return EINVAL;

	if (!call) {
		DEBUG_WARNING("ua_answer: no call param, using ua default\n");
		call = ua_call(ua);
		if (!call) {
			DEBUG_NOTICE("answer: no incoming calls found\n");
			return ENOENT;
		}
	}

	/* todo: put previous call on-hold (if configured) */

	ua->play = mem_deref(ua->play);

	return call_answer(call, 200, audio_mod, audio_dev);
}


/**
 * Print the user-agent registration status
 *
 * @param pf  Print function
 * @param ua  User-Agent object
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_print_status(struct re_printf *pf, const struct ua *ua)
{
	struct le *le;
	int err;

	if (!ua)
		return 0;

	err = re_hprintf(pf, "%-42s", ua->acc->aor);

	for (le = ua->regl.head; le; le = le->next)
		err |= reg_status(pf, le->data);

	err |= re_hprintf(pf, "\n");

	return err;
}


/**
 * Send SIP OPTIONS message to a peer
 *
 * @param ua      User-Agent object
 * @param uri     Peer SIP Address
 * @param resph   Response handler
 * @param arg     Handler argument
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_options_send(struct ua *ua, const char *uri,
		    options_resp_h *resph, void *arg)
{
	int err;

	if (!ua)
		return EINVAL;

	err = sip_req_send(ua, "OPTIONS", uri, resph, arg,
			   "Accept: application/sdp\r\n"
			   "Content-Length: 0\r\n"
			   "\r\n");
	if (err) {
		DEBUG_WARNING("send options: (%m)\n", err);
	}

	return err;
}


/**
 * Get the AOR of a User-Agent
 *
 * @param ua User-Agent object
 *
 * @return AOR
 */
const char *ua_aor(const struct ua *ua)
{
	return ua ? ua->acc->aor : NULL;
}


/**
 * Get the outbound SIP proxy of a User-Agent
 *
 * @param ua User-Agent object
 *
 * @return Outbound SIP proxy uri
 */
const char *ua_outbound(const struct ua *ua)
{
	/* NOTE: we pick the first outbound server, should be rotated? */
	return ua ? ua->acc->outbound[0] : NULL;
}


/**
 * Get the current call object of a User-Agent
 *
 * @param ua User-Agent object
 *
 * @return Current call, NULL if no active calls
 *
 *
 * Current call strategy:
 *
 * We can only have 1 current call. The current call is the one that was
 * added last (end of the list), which is not on-hold
 */
struct call *ua_call(const struct ua *ua)
{
	struct le *le;

	if (!ua)
		return NULL;

	for (le = ua->calls.tail; le; le = le->prev) {

		struct call *call = le->data;

		/* todo: check if call is on-hold */

		return call;
	}

	return NULL;
}


/**
 * Print the user-agent debug information
 *
 * @param pf  Print function
 * @param ua  User-Agent object
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_debug(struct re_printf *pf, const struct ua *ua)
{
	struct le *le;
	int err;

	if (!ua)
		return 0;

	err  = re_hprintf(pf, "--- %s ---\n", ua->acc->aor);
	err |= re_hprintf(pf, " cuser:     %s\n", ua->cuser);
	err |= re_hprintf(pf, " af:        %s\n", net_af2name(ua->af));
	err |= re_hprintf(pf, " %H", ua_print_supported, ua);

	err |= account_debug(pf, ua->acc);

	for (le = ua->regl.head; le; le = le->next)
		err |= reg_debug(pf, le->data);

	return err;
}


/* One instance */


#ifdef USE_TLS
static int add_transp_clientcert(void)
{
	struct le *le;
	int err = 0;

	for (le = list_head(&uag.ual); le; le = le->next) {
		struct account *acc = ua_account(le->data);
		if (acc->cert) {
			err = sip_transp_add_ccert(uag.sip,
					&acc->laddr.uri, acc->cert);
			if (err) {
				DEBUG_WARNING("uag: SIP/TLS add client "
					"certificate %s failed: %m\n",
					acc->cert, err);
				return err;
			}
		}
	}

	return err;
}
#endif


static int add_transp_af(const struct sa *laddr)
{
	struct sa local;
#ifdef USE_TLS
	const char *cert = NULL;
	const char *cafile = NULL;
	const char *capath = NULL;
#endif
	int err = 0;

	if (str_isset(uag.cfg->local)) {
		err = sa_decode(&local, uag.cfg->local,
				str_len(uag.cfg->local));
		if (err) {
			err = sa_set_str(&local, uag.cfg->local, 0);
			if (err) {
				DEBUG_WARNING("decode failed: %s\n",
					      uag.cfg->local);
				return err;
			}
		}

		if (!sa_isset(&local, SA_ADDR)) {
			uint16_t port = sa_port(&local);
			(void)sa_set_sa(&local, &laddr->u.sa);
			sa_set_port(&local, port);
		}

		if (sa_af(laddr) != sa_af(&local))
			return 0;
	}
	else {
		sa_cpy(&local, laddr);
		sa_set_port(&local, 0);
	}

	if (u32mask_enabled(uag.transports, SIP_TRANSP_UDP))
		err |= sip_transp_add(uag.sip, SIP_TRANSP_UDP, &local);
	if (u32mask_enabled(uag.transports, SIP_TRANSP_TCP))
		err |= sip_transp_add(uag.sip, SIP_TRANSP_TCP, &local);
	if (err) {
		DEBUG_WARNING("ua: SIP Transport failed: %m\n", err);
		return err;
	}

#ifdef USE_TLS
	if (u32mask_enabled(uag.transports, SIP_TRANSP_TLS)) {
		/* Build our SSL context*/
		if (!uag.tls) {
			if (str_isset(uag.cfg->cert)) {
				cert = uag.cfg->cert;
				DEBUG_INFO("SIP Certificate: %s\n", cert);
			}

			err = tls_alloc(&uag.tls, TLS_METHOD_SSLV23,
					cert, NULL);
			if (err) {
				DEBUG_WARNING("ua: tls_alloc() failed: %m\n", err);
				return err;
			}

#if defined(WIN32) || defined(__WIN32__)
			if (uag.cfg->use_windows_root_ca_store) {
				tls_add_windows_ca(uag.tls);
			}
#endif

			if (str_isset(uag.cfg->cafile))
				cafile = uag.cfg->cafile;
			if (str_isset(uag.cfg->capath))
				capath = uag.cfg->capath;

			if (cafile || capath) {
				DEBUG_INFO("ua: adding SIP CA file: %s\n", cafile);
				if (capath) {
					DEBUG_INFO("ua: adding SIP CA path: %s\n", capath);
				}

				err = tls_add_cafile_path(uag.tls,
					cafile, capath);
				if (err) {
					DEBUG_WARNING("ua: tls_add_ca() failed:"
						" %m\n", err);
				}
			}

			if (!uag.cfg->verify_server)
				tls_disable_verify_server(uag.tls);
		}

		if (sa_isset(&local, SA_PORT))
			sa_set_port(&local, sa_port(&local) + 1);

		err = sip_transp_add(uag.sip, SIP_TRANSP_TLS, &local, uag.tls);
		if (err) {
			DEBUG_WARNING("ua: SIP/TLS transport failed: %m\n", err);
			return err;
		}

		err = add_transp_clientcert();
		if (err)
			return err;
	}
#endif

#if 0
	if (u32mask_enabled(uag.transports, SIP_TRANSP_WS)) {
		err = sip_transp_add_websock(uag.sip, SIP_TRANSP_WS, &local,
				false, NULL, NULL);
		if (err) {
			DEBUG_WARNING("ua: could not add Websock transport (%m)\n",
					err);
			return err;
		}
	}
#endif

#if 0 //def USE_TLS
	if (u32mask_enabled(uag.transports, SIP_TRANSP_WSS)) {
		if (!uag.wss_tls) {
			err = tls_alloc(&uag.wss_tls, TLS_METHOD_SSLV23,
					NULL, NULL);
			if (err) {
				warning("ua: wss tls_alloc() failed: %m\n",
					err);
				return err;
			}

			err = tls_set_verify_purpose(uag.wss_tls, "sslserver");
			if (err) {
				warning("ua: wss tls_set_verify_purpose() "
					"failed: %m\n", err);
				return err;
			}

			if (cafile || capath) {
				err = tls_add_cafile_path(uag.wss_tls, cafile,
							  capath);
				if (err) {
					warning("ua: wss tls_add_ca() failed:"
							" %m\n", err);
				}
			}

			if (!uag.cfg->verify_server)
				tls_disable_verify_server(uag.wss_tls);
		}
		err = sip_transp_add_websock(uag.sip, SIP_TRANSP_WSS, &local,
				false, uag.cfg->cert, uag.wss_tls);
		if (err) {
			warning("ua: could not add secure Websock transport "
				"(%m)\n", err);
			return err;
		}
	}
#endif

	//sip_settos(uag.sip, uag.cfg->tos);
	return err;
}


static int ua_add_transp(void)
{
	int err = 0;

	if (!uag.prefer_ipv6) {
		if (sa_isset(net_laddr_af(AF_INET), SA_ADDR))
			err |= add_transp_af(net_laddr_af(AF_INET));
	}

#if HAVE_INET6
	if (sa_isset(net_laddr_af(AF_INET6), SA_ADDR))
		err |= add_transp_af(net_laddr_af(AF_INET6));
#endif

	return err;
}


static bool require_handler(const struct sip_hdr *hdr,
			    const struct sip_msg *msg, void *arg)
{
	struct ua *ua = arg;
	bool supported = false;
	size_t i;
	(void)msg;

	for (i=0; i<ua->extensionc; i++) {

		if (!pl_casecmp(&hdr->val, &ua->extensionv[i])) {
			supported = true;
			break;
		}
	}

	return !supported;
}


/* Handle incoming calls */
static void sipsess_conn_handler(const struct sip_msg *msg, void *arg)
{
	const struct sip_hdr *hdr;
	struct ua *ua;
	struct call *call = NULL;
	char str[256];
	int err;

	(void)arg;

	ua = uag_find(&msg->uri.user);
	if (!ua) {
		DEBUG_WARNING("%r: UA not found: %r\n",
			      &msg->from.auri, &msg->uri.user);
		(void)sip_treply(NULL, uag_sip(), msg, 404, "Not Found");
		return;
	}

	/* handle multiple calls */
	if (list_count(&ua->calls) + 1 > MAX_CALLS) {
		DEBUG_NOTICE("rejected call from %r (maximum %d calls)\n",
			     &msg->from.auri, MAX_CALLS);
		(void)sip_treply(NULL, uag.sip, msg, 486, "Busy Here");
		return;
	}

	/* Handle Require: header, check for any required extensions */
	hdr = sip_msg_hdr_apply(msg, true, SIP_HDR_REQUIRE,
				require_handler, ua);
	if (hdr) {
		DEBUG_NOTICE("call from %r rejected with 420"
			     " -- option-tag '%r' not supported\n",
			     &msg->from.auri, &hdr->val);

		(void)sip_treplyf(NULL, NULL, uag.sip, msg, false,
				  420, "Bad Extension",
				  "Unsupported: %r\r\n"
				  "Content-Length: 0\r\n\r\n",
				  &hdr->val);
		return;
	}

	err = ua_call_alloc(&call, ua, VIDMODE_ON, msg, NULL, NULL);
	if (err) {
		DEBUG_WARNING("call_alloc: %m\n", err);
		goto error;
	}

	err = call_accept(call, uag.sock, msg);
	if (err)
		goto error;

	return;

 error:
	mem_deref(call);
	{
	int TODO__GET_RID_OF_PERCENT_M_IN_REPLIES;	// unparseable messages
    }
	(void)re_snprintf(str, sizeof(str), "Error (%m)", err);
	(void)sip_treply(NULL, uag.sip, msg, 500, str);
}


static void net_change_handler(void *arg)
{
	(void)arg;

	(void)re_printf("IP-address changed: %j\n", net_laddr_af(AF_INET));

	(void)uag_reset_transp(true, true);
}


static int cmd_quit(struct re_printf *pf, void *unused)
{
	int err;

	(void)unused;

	err = re_hprintf(pf, "Quit\n");

	ua_stop_all(false);

	return err;
}


static const struct cmd cmdv[] = {
	{'q',       0, "Quit",                     cmd_quit             },
};


/**
 * Initialise the User-Agents
 *
 * @param software    SIP User-Agent string
 * @param udp         Enable UDP transport
 * @param tcp         Enable TCP transport
 * @param tls         Enable TLS transport
 * @param prefer_ipv6 Prefer IPv6 flag
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_init(const char *software, bool udp, bool tcp, bool tls, bool prefer_ipv6)
{
	struct config *cfg = conf_config();
	uint32_t bsize;
	int err;

	uag.cfg = &cfg->sip;
	bsize = cfg->sip.trans_bsize;

	play_init(cfg);

	/* Initialise Network */
	err = net_init(&cfg->net, prefer_ipv6 ? AF_INET6 : AF_INET);
	if (err) {
		DEBUG_WARNING("network init failed: %m\n", err);
		return err;
	}

	if (cfg->sip.transports) {
		uag.transports = cfg->sip.transports;
	}
	else {
		u32mask_enable(&uag.transports, SIP_TRANSP_UDP, udp);
		u32mask_enable(&uag.transports, SIP_TRANSP_TCP, tcp);
		u32mask_enable(&uag.transports, SIP_TRANSP_TLS, tls);
		//u32mask_enable(&uag.transports, SIP_TRANSP_WS,  true);
		//u32mask_enable(&uag.transports, SIP_TRANSP_WSS, true);
	}
	uag.prefer_ipv6 = prefer_ipv6;

	list_init(&uag.ual);

	err = sip_alloc(&uag.sip, net_dnsc(), bsize, bsize, bsize,
			software, exit_handler, NULL);
	if (err) {
		DEBUG_WARNING("sip stack failed: %m\n", err);
		goto out;
	}

	err = ua_add_transp();
	if (err)
		goto out;

	err = sip_listen(&uag.lsnr, uag.sip, true, request_handler, NULL);
	if (err)
		goto out;

	err = sipsess_listen(&uag.sock, uag.sip, bsize,
			     sipsess_conn_handler, NULL);
	if (err)
		goto out;

	err = sipevent_listen(&uag.evsock, uag.sip, bsize, bsize, NULL, NULL);
	if (err)
		goto out;

	err = cmd_register(cmdv, ARRAY_SIZE(cmdv));
	if (err)
		goto out;

	net_change(60, net_change_handler, NULL);

 out:
	if (err) {
		DEBUG_WARNING("init failed (%m)\n", err);
		ua_close();
	}
	return err;
}

void ua_log_messages(bool log, bool only_first_lines)
{
	sip_log_messages(uag.sip, log, only_first_lines);
}

/**
 * Close all active User-Agents
 */
void ua_close(void)
{
	cmd_unregister(cmdv);
	net_close();
	play_close();

	uag.evsock   = mem_deref(uag.evsock);
	uag.sock     = mem_deref(uag.sock);
	uag.lsnr     = mem_deref(uag.lsnr);
	uag.sip      = mem_deref(uag.sip);

#ifdef USE_TLS
	uag.tls = mem_deref(uag.tls);
#endif

	list_flush(&uag.ual);
	list_flush(&uag.ehl);
}


/**
 * Stop all User-Agents
 *
 * @param forced True to force, otherwise false
 */
void ua_stop_all(bool forced)
{
	module_app_unload();

	if (!list_isempty(&uag.ual)) {
		const uint32_t n = list_count(&uag.ual);
		(void)re_printf("Stopping %u useragent%s.. %s\n",
				 n, n==1 ? "" : "s",
				 forced ? "(Forced)" : "");
	}

	if (forced)
		sipsess_close_all(uag.sock);
	else
		list_flush(&uag.ual);

	sip_close(uag.sip, forced);
	re_printf("Useragent(s) stopped\n");
}


/**
 * Reset the SIP transports for all User-Agents
 *
 * @param reg      True to reset registration
 * @param reinvite True to update active calls
 *
 * @return 0 if success, otherwise errorcode
 */
int uag_reset_transp(bool reg, bool reinvite)
{
	struct le *le;
	int err;

	/* Update SIP transports */
	sip_transp_flush(uag.sip);

	(void)net_check();
	err = ua_add_transp();
	if (err)
		return err;

	/* Re-REGISTER all User-Agents */
	for (le = uag.ual.head; le; le = le->next) {
		struct ua *ua = le->data;

		if (reg && ua->acc->regint) {
			err |= ua_register(ua);
		}

		/* update all active calls */
		if (reinvite) {
			struct le *lec;

			for (lec = ua->calls.head; lec; lec = lec->next) {
				struct call *call = lec->data;

				err |= call_reset_transp(call);
			}
		}
	}

	return err;
}


/**
 * Print the SIP Status for all User-Agents
 *
 * @param pf     Print handler for debug output
 * @param unused Unused parameter
 *
 * @return 0 if success, otherwise errorcode
 */
int ua_print_sip_status(struct re_printf *pf, void *unused)
{
	(void)unused;
	return sip_debug(pf, uag.sip);
}


/**
 * Print all calls for a given User-Agent
 */
int ua_print_calls(struct re_printf *pf, const struct ua *ua)
{
	struct le *le;
	int err = 0;

	err |= re_hprintf(pf, "\n--- List of active calls (%u): ---\n",
			  list_count(&ua->calls));

	for (le = ua->calls.head; le; le = le->next) {

		const struct call *call = le->data;

		err |= re_hprintf(pf, "  %H\n", call_info, call);
	}

	err |= re_hprintf(pf, "\n");

	return err;
}


/**
 * Get the global SIP Stack
 *
 * @return SIP Stack
 */
struct sip *uag_sip(void)
{
	return uag.sip;
}


/**
 * Get the global SIP Session socket
 *
 * @return SIP Session socket
 */
struct sipsess_sock *uag_sipsess_sock(void)
{
	return uag.sock;
}


/**
 * Get the global SIP Event socket
 *
 * @return SIP Event socket
 */
struct sipevent_sock *uag_sipevent_sock(void)
{
	return uag.evsock;
}


struct tls *uag_tls(void)
{
#ifdef USE_TLS
	return uag.tls;
#else
	return NULL;
#endif
}


/**
 * Get the name of the User-Agent event
 *
 * @param ev User-Agent event
 *
 * @return Name of the event
 */
const char *uag_event_str(enum ua_event ev)
{
	switch (ev) {

	case UA_EVENT_REGISTERING:      return "REGISTERING";
	case UA_EVENT_REGISTER_OK:      return "REGISTER_OK";
	case UA_EVENT_REGISTER_FAIL:    return "REGISTER_FAIL";
	case UA_EVENT_UNREGISTERING:    return "UNREGISTERING";
	case UA_EVENT_CALL_INCOMING:    return "CALL_INCOMING";
	case UA_EVENT_CALL_RINGING:     return "CALL_RINGING";
	case UA_EVENT_CALL_PROGRESS:    return "CALL_PROGRESS";
	case UA_EVENT_CALL_ESTABLISHED: return "CALL_ESTABLISHED";
	case UA_EVENT_CALL_CLOSED:      return "CALL_CLOSED";
	case UA_EVENT_CALL_TRANSFER:	return "CALL_TRANSFER";
    case UA_EVENT_AUDIO_ERROR:      return "AUDIO_ERROR";
	default: return "?";
	}
}


/**
 * Get the current SIP socket file descriptor for a User-Agent
 *
 * @param ua User-Agent
 *
 * @return File descriptor, or -1 if not available
 */
int ua_sipfd(const struct ua *ua)
{
	struct le *le;

	if (!ua)
		return -1;

	for (le = ua->regl.head; le; le = le->next) {

		struct reg *reg = le->data;
		int fd;

		fd = reg_sipfd(reg);
		if (fd != -1)
			return fd;
	}

	return -1;
}


/**
 * Find the correct UA from the contact user
 *
 * @param cuser Contact username
 *
 * @return Matching UA if found, NULL if not found
 */
struct ua *uag_find(const struct pl *cuser)
{
	struct le *le;

	for (le = uag.ual.head; le; le = le->next) {
		struct ua *ua = le->data;

		if (0 == pl_strcasecmp(cuser, ua->cuser))
			return ua;
	}

	/* Try also matching by AOR, for better interop */
	for (le = uag.ual.head; le; le = le->next) {
		struct ua *ua = le->data;

		if (0 == pl_casecmp(cuser, &ua->acc->luri.user))
			return ua;
	}

	/* "Local" account - check if any account is set to answer any incoming call */
	for (le = uag.ual.head; le; le = le->next) {
		struct ua *ua = le->data;

		if (ua->acc->answer_any != 0)
			return ua;
	}

	return NULL;
}


/**
 * Find a User-Agent (UA) from an Address-of-Record (AOR)
 *
 * @param aor Address-of-Record string
 *
 * @return User-Agent (UA) if found, otherwise NULL
 */
struct ua *uag_find_aor(const char *aor)
{
	struct le *le;

	for (le = uag.ual.head; le; le = le->next) {
		struct ua *ua = le->data;

		if (str_isset(aor) && str_cmp(ua->acc->aor, aor))
			continue;

		return ua;
	}

	return NULL;
}


/**
 * Get the contact user of a User-Agent (UA)
 *
 * @param ua User-Agent
 *
 * @return Contact user
 */
const char *ua_cuser(const struct ua *ua)
{
	return ua ? ua->cuser : NULL;
}

/**
 * Get Account of a User-Agent
 *
 * @param ua User-Agent
 *
 * @return Pointer to UA's account
 */
struct account *ua_account(const struct ua *ua)
{
	return ua ? ua->acc : NULL;
}


struct list *uag_list(void)
{
	return &uag.ual;
}


/**
 * Return list of methods supported by the UA
 *
 * @return String of supported methods
 */
const char *uag_allowed_methods(void)
{
	return "INVITE,ACK,BYE,CANCEL,OPTIONS,REFER,NOTIFY,SUBSCRIBE,INFO";
}


int ua_print_supported(struct re_printf *pf, const struct ua *ua)
{
	size_t i;
	int err;

	err = re_hprintf(pf, "Supported:");

	for (i=0; i<ua->extensionc; i++) {
		err |= re_hprintf(pf, "%s%r",
				  i==0 ? " " : ",", &ua->extensionv[i]);
	}

	err |= re_hprintf(pf, "\r\n");

	return err;
}


struct account *ua_prm(const struct ua *ua)
{
	return ua ? ua->acc : NULL;
}


static void eh_destructor(void *arg)
{
	struct ua_eh *eh = arg;
	list_unlink(&eh->le);
}


/**
 * Register a User-Agent event handler
 *
 * @param h   Event handler
 * @param arg Handler argument
 *
 * @return 0 if success, otherwise errorcode
 */
int uag_event_register(ua_event_h *h, void *arg)
{
	struct ua_eh *eh;

	if (!h)
		return EINVAL;

	uag_event_unregister(h);

	eh = mem_zalloc(sizeof(*eh), eh_destructor);
	if (!eh)
		return ENOMEM;

	eh->h = h;
	eh->arg = arg;

	list_append(&uag.ehl, &eh->le, eh);

	return 0;
}


/**
 * Unregister a User-Agent event handler
 *
 * @param h   Event handler
 */
void uag_event_unregister(ua_event_h *h)
{
	struct le *le;

	for (le = uag.ehl.head; le; le = le->next) {

		struct ua_eh *eh = le->data;

		if (eh->h == h) {
			mem_deref(eh);
			break;
		}
	}
}
