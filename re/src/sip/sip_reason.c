#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_uri.h>
#include <re_list.h>
#include <re_sa.h>
#include <re_msg.h>
#include <re_sip.h>

/* Example: Reason: SIP;cause=200;text="Call completed elsewhere" */

#define HEADER_NAME "Reason"

/**
 * Decode a pointer-length string into a SIP Reason header
 * @param name Full header name
 * @param pl   Pointer-length string
 *
 * @return 0 for success, otherwise errorcode
 */
int sip_reason_decode(const struct pl *name, struct sip_reason *s_reason, const struct pl *pl)
{
	struct pl pval;
	int err;
	int i;

	if (!s_reason || !pl)
		return EINVAL;

	if (pl_strcasecmp(name, HEADER_NAME) != 0) {
		/* 	Is hash function for header names really perfect (sip_hdrid)? Probably not. */
		return EINVAL;
	}

	s_reason->protocol = *pl;
	for (i=0; i<pl->l; i++) {
		if (pl->p[i] == ';') {
			s_reason->protocol.l = i;
			break;
		}
	}

	s_reason->cause = -1;

	if (!msg_param_decode(pl, "cause", &pval)) {
		s_reason->cause = pl_u32(&pval);
	}

	msg_param_decode(pl, "text", &s_reason->text);

	return 0;
}


