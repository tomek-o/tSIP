/**
 * @file presence.c Presence module
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <baresip.h>
#include "presence.h"


static int module_init(void)
{
	int err;

	err = presence_subscriber_init();
	if (err)
		return err;

	err = presence_notifier_init();
	if (err)
		return err;

	return 0;
}


static int module_close(void)
{
	presence_notifier_close();
	presence_subscriber_close();

	return 0;
}


void presence_resubscribe(void)
{
	presence_subscriber_resubscribe();
}


const struct mod_export DECL_EXPORTS(presence) = {
	"presence",
	"application",
	module_init,
	module_close
};
