/**
 * @file src/contact.c  Contacts handling
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <baresip.h>
#include <assert.h>


struct contact {
	struct le le;
	struct sip_addr addr;
	char *buf;
	enum presence_status status;
	char presence_note[64];
	int id;
	contact_dlginfo_h *dlginfo_h;
	contact_presence_h *presence_h;

	int dialog_data_cnt;
    struct dialog_data ddata[MAX_DIALOG_DATA_CNT];	
};

static struct list cl;


static void destructor(void *arg)
{
	struct contact *c = arg;

	list_unlink(&c->le);
	mem_deref(c->buf);
}


/**
 * Add a contact
 *
 * @param contactp Pointer to allocated contact (optional)
 * @param addr     Contact in SIP address format
 *
 * @return 0 if success, otherwise errorcode
 */
int contact_add(struct contact **contactp, const struct pl *addr, int id, contact_dlginfo_h *dlginfo_h, contact_presence_h *presence_h)
{
	struct contact *c;
	struct pl pl;
	int err;

	c = mem_zalloc(sizeof(*c), destructor);
	if (!c)
		return ENOMEM;

	c->id = id;
	c->dlginfo_h = dlginfo_h;
	c->presence_h = presence_h;

	err = pl_strdup(&c->buf, addr);
	if (err)
		goto out;

	pl_set_str(&pl, c->buf);

	err = sip_addr_decode(&c->addr, &pl);
	if (err) {
		(void)re_printf("contact: decode error '%r'\n", addr);
		goto out;
	}

	c->status = PRESENCE_UNKNOWN;
    c->dialog_data_cnt = 0;

	list_append(&cl, &c->le, c);

 out:
	if (err)
		mem_deref(c);
	else if (contactp)
		*contactp = c;

	return err;
}


/**
 * Get the SIP address of a contact
 *
 * @param c Contact
 *
 * @return SIP Address
 */
struct sip_addr *contact_addr(const struct contact *c)
{
	return c ? (struct sip_addr *)&c->addr : NULL;
}


/**
 * Get the contact string
 *
 * @param c Contact
 *
 * @return Contact string
 */
const char *contact_str(const struct contact *c)
{
	return c ? c->buf : NULL;
}


/**
 * Get the list of contacts
 *
 * @return List of contacts
 */
struct list *contact_list(void)
{
	return &cl;
}


void contact_set_presence(struct contact *c, enum presence_status status, const struct pl *note)
{
	int len;
	if (!c)
		return;

	if (c->status != PRESENCE_UNKNOWN && c->status != status) {

		(void)re_printf("<%r> changed status from %s to %s (note: %r)\n",
				&c->addr.auri,
				contact_presence_str(c->status),
				contact_presence_str(status),
				note);
	}

	c->status = status;
	len = note->l < (sizeof(c->presence_note)-1) ? note->l : (sizeof(c->presence_note)-1);	
	if (len) {
		strncpy(c->presence_note, note->p, len);
	}
	c->presence_note[len] = '\0';
	if (c->presence_h) {
		c->presence_h(c->id, c->status, c->presence_note);
	}
}


const char *contact_presence_str(enum presence_status status)
{
	switch (status) {

	default:
	case PRESENCE_UNKNOWN: return "Unknown";
	case PRESENCE_OPEN:    return "Online";
	case PRESENCE_CLOSED:  return "Offline";
	case PRESENCE_BUSY:    return "Busy";
	}
}


void contact_set_dialog_info(struct contact *c, const struct dialog_data *ddata, unsigned int ddata_cnt)
{
	int len;
	if (!c)
		return;
#if 0
	if (c->status != DIALOG_INFO_UNKNOWN && c->dlg_info_status != status) {

		(void)re_printf("<%r> changed status from %s to %s\n",
				&c->addr.auri,
				contact_dialog_info_str(c->dlg_info_status),
				contact_dialog_info_str(status));
	}
#else
	{
		static int TODO__ADD_DIALOG_INFO_LOG_ON_CHANGE;
	}
#endif

	assert(sizeof(c->ddata) >= sizeof(*ddata)*ddata_cnt);

	c->dialog_data_cnt = ddata_cnt;
	memcpy(c->ddata, ddata, ddata_cnt * sizeof(ddata[0]));

	if (c->dlginfo_h) {
		c->dlginfo_h(c->id, c->ddata, c->dialog_data_cnt);
	}
}

const char *contact_dialog_info_str(enum dialog_info_status status)
{
	switch (status) {
	case DIALOG_INFO_UNKNOWN: 		return "Unknown";
	case DIALOG_INFO_TERMINATED:    return "Terminated";
	case DIALOG_INFO_EARLY:			return "Early";
	case DIALOG_INFO_CONFIRMED:		return "Confirmed";
	default:						return "???";
	}
}

#if 0
int contacts_print(struct re_printf *pf, void *unused)
{
	struct le *le;
	int err;

	(void)unused;

	err = re_hprintf(pf, "\n--- Contacts: (%u) ---\n",
			 list_count(contact_list()));

	for (le = list_head(contact_list()); le && !err; le = le->next) {
		const struct contact *c = le->data;
		const struct sip_addr *addr = &c->addr;

		err = re_hprintf(pf, "%20s  %20s  %r <%r>\n",
				 contact_presence_str(c->status),
				 contact_dialog_info_str(c->dlg_info_status),
				 &addr->dname, &addr->auri);
	}

	err |= re_hprintf(pf, "\n");

	return err;
}
#endif
