#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_uri.h>
#include <re_list.h>
#include <re_sa.h>
#include <re_msg.h>
#include <re_sip.h>


/**
 * Decode a pointer-length string into a SIP Call-Alert header
 * @param pl   Pointer-length string
 *
 * @return 0 for success, otherwise errorcode
 */
int sip_alert_info_decode(struct sip_alert_info *alert_info, const struct pl *pl)
{
	struct pl pval;
	int err;

	if (!alert_info || !pl)
		return EINVAL;

	alert_info->info = *pl;

	return 0;
}


