/**
 * @file presence.h Presence module interface
 *
 * Copyright (C) 2010 Creytiv.com
 */


int  presence_subscriber_init(void);
void presence_subscriber_close(void);
void presence_subscriber_resubscribe(void);


int  presence_notifier_init(void);
void presence_notifier_close(void);
