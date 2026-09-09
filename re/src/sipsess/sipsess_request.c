/**
 * @file sipsess/request.c  SIP Session Non-INVITE Request
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
#include <re_tmr.h>
#include <re_msg.h>
#include <re_sip.h>
#include <re_sipsess.h>
#include "sipsess.h"


static void destructor(void *arg)
{
	struct sipsess_request *req = arg;

	list_unlink(&req->le);
	mem_deref(req->ctype);
	mem_deref(req->body);
	mem_deref(req->req);
	mem_deref(req->method);
	mem_deref(req->hdrs);

	/* wait for pending requests */
	if (req->sess->terminated && !req->sess->requestl.head)
		mem_deref(req->sess);
}


static void internal_resp_handler(int err, const struct sip_msg *msg,
				  void *arg)
{
	(void)err;
	(void)msg;
	(void)arg;
}


int sipsess_request_alloc(struct sipsess_request **reqp, struct sipsess *sess,
			  const char *ctype, struct mbuf *body,
			  const char *method, const char *hdrs,
			  sip_resp_h *resph, void *arg)
{
	struct sipsess_request *req;
	int err = 0;

	if (!reqp || !sess || sess->terminated)
		return EINVAL;

	req = mem_zalloc(sizeof(*req), destructor);
	if (!req)
		return ENOMEM;

	list_append(&sess->requestl, &req->le, req);

	if (ctype) {
		err = str_dup(&req->ctype, ctype);
		if (err)
			goto out;
	}

	if (method) {
		err = str_dup(&req->method, method);
		if (err)
			goto out;
	}

	if (hdrs) {
		err = str_dup(&req->hdrs, hdrs);
		if (err)
			goto out;
	}

	req->sess  = sess;
	req->body  = mem_ref(body);
	req->resph = resph ? resph : internal_resp_handler;
	req->arg   = arg;

 out:
	if (err)
		mem_deref(req);
	else
		*reqp = req;

	return 0;
}


static int generic_request(struct sipsess_request *req);


static void generic_resp_handler(int err, const struct sip_msg *msg,
				 void *arg)
{
	struct sipsess_request *req = arg;

	if (err || sip_request_loops(&req->ls, msg->scode))
		goto out;

	if (msg->scode < 200) {
		return;
	}
	else if (msg->scode < 300) {
		;
	}
	else {
		if (req->sess->terminated)
			goto out;

		switch (msg->scode) {

		case 401:
		case 407:
			err = sip_auth_authenticate(req->sess->auth, msg);
			if (err) {
				err = (err == EAUTH) ? 0 : err;
				break;
			}

			err = generic_request(req);
			if (err)
				break;

			return;

		case 408:
		case 481:
			sipsess_terminate(req->sess, 0, msg);
			break;
		}
	}

 out:
	if (!req->sess->terminated) {
		if (err == ETIMEDOUT)
			sipsess_terminate(req->sess, err, NULL);
		else
			req->resph(err, msg, req->arg);
	}

	mem_deref(req);
}


static int generic_request(struct sipsess_request *req)
{
	return sip_drequestf(&req->req, req->sess->sip, true, req->method,
			     req->sess->dlg, 0, req->sess->auth,
			     NULL, generic_resp_handler, req,
			     "%s", req->hdrs);
}


/**
 * Send a SIP request of an arbitrary method in the SIP Session, reusing its
 * dialog (Call-ID, tags) so the peer recognizes it as belonging to the
 * session - unlike sip_req_send(), which always starts a new, separate
 * transaction/dialog to a given URI.
 *
 * @param sess      SIP Session
 * @param method    SIP method
 * @param hdrs      Extra header lines and body, e.g.
 *                  "Content-Type: text/plain\r\nContent-Length: 2\r\n\r\nhi"
 * @param resph     Response handler
 * @param arg       Handler argument
 *
 * @return 0 if success, otherwise errorcode
 */
int sipsess_send_request(struct sipsess *sess, const char *method,
			 const char *hdrs, sip_resp_h *resph, void *arg)
{
	struct sipsess_request *req;
	int err;

	if (!sess || sess->terminated || !method || !hdrs)
		return EINVAL;

	if (!sip_dialog_established(sess->dlg))
		return ENOTCONN;

	err = sipsess_request_alloc(&req, sess, NULL, NULL, method, hdrs,
				    resph, arg);
	if (err)
		return err;

	err = generic_request(req);
	if (err)
		mem_deref(req);

	return err;
}
