#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_uri.h>
#include <re_list.h>
#include <re_sa.h>
#include <re_msg.h>
#include <re_sip.h>

static const char* HEADER_NAME = "Access-URL";

static int sip_answer_after_decode(const struct pl *pl, const char *name, struct pl *val)
{
	char expr[128];
	struct pl v;

	if (!pl || !name || !val)
		return EINVAL;

	(void)re_snprintf(expr, sizeof(expr),
			  "[ \t\r\n]*%s[ \t\r\n]*=[ \t\r\n]*[~ \t\r\n;]+",
			  name);

	if (re_regex(pl->p, pl->l, expr, NULL, NULL, NULL, &v))
		return ENOENT;

	*val = v;

	return 0;
}

/**
 * Decode a pointer-length string into a SIP Access-URL header
 * @param name Full header name
 * @param pl   Pointer-length string
 *
 * @return 0 for success, otherwise errorcode
 */
int sip_access_url_decode(const struct pl *name, struct sip_access_url *access_url, const struct pl *pl)
{
	struct pl pval;
	int err;

	if (!access_url || !pl)
		return EINVAL;

	if (pl_strcasecmp(name, HEADER_NAME) != 0) {
		/* 	Is hash function for header names really perfect (sip_hdrid)? Probably not.
            Since this is custom/non-standard header preventing collision might be even more important.
		*/
		return EINVAL;
	}

	if (0 != re_regex(pl->p, pl->l, "[~ \t\r\n<]*[ \t\r\n]*<[^>]+>[^]*",
			  NULL, NULL, &access_url->url, NULL)) {
		return EINVAL;
	}

	access_url->expires = -1;
	access_url->mode = SIP_ACCESS_URL_MODE_UNKNOWN;

	if (!msg_param_decode(pl, "mode", &pval)) {
		if (pl_strcasecmp(&pval, "active") == 0) {
			access_url->mode = SIP_ACCESS_URL_MODE_ACTIVE;
		} else if (pl_strcasecmp(&pval, "passive") == 0) {
			access_url->mode = SIP_ACCESS_URL_MODE_PASSIVE;
		}
	}

	return 0;
}


