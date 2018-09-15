/** \file
	\brief Dialog-info
*/
#include <re.h>
#include <baresip.h>
#include "dialog_info.h"


static int module_init(void)
{
	int err;

	err = dialog_info_subscriber_init();
	if (err)
		return err;

	return 0;
}


static int module_close(void)
{
	dialog_info_subscriber_close();

	return 0;
}


const struct mod_export DECL_EXPORTS(dialog_info) = {
	"dialog-info",
	"application",
	module_init,
	module_close
};
