#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_uri.h>
#include <re_list.h>
#include <re_sa.h>
#include <re_msg.h>
#include <re_sip.h>

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
 * Decode a pointer-length string into a SIP Call-Info header
 * @param pl   Pointer-length string
 *
 * @return 0 for success, otherwise errorcode
 */
int sip_call_info_decode(struct sip_call_info *call_info, const struct pl *pl)
{
	struct pl pval;
	int err;

	if (!call_info || !pl)
		return EINVAL;

	call_info->answer_after = -1;

	if (!msg_param_decode(pl, "answer-after", &pval)) {
		// "Snom and others": <uri>;answer-after=X
		call_info->answer_after = pl_u32(&pval);
		if (call_info->answer_after < 0)
			call_info->answer_after = -1;
	} else {
		// Polycom: Answer-After=X
        if (!sip_answer_after_decode(pl, "Answer-After", &pval)) {
			call_info->answer_after = pl_u32(&pval);
			if (call_info->answer_after < 0)
				call_info->answer_after = -1;			
		}
	}

	return 0;
}


