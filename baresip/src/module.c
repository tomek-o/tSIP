/**
 * @file module.c Module loading
 *
 * Copyright (C) 2010 Creytiv.com
 */
#include <re.h>
#include <baresip.h>
#include <module.h>
#include "core.h"


#define DEBUG_MODULE "module"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


struct modapp {
	struct mod *mod;
	struct le le;
};


static struct list modappl;


static void modapp_destructor(void *arg)
{
	struct modapp *modapp = arg;
	list_unlink(&modapp->le);
	mem_deref(modapp->mod);
}


#ifdef STATIC

/* Declared in static.c */
extern const struct mod_export *mod_table[];

static const struct mod_export *find_module(const struct pl *pl)
{
	struct pl name;
	uint32_t i;

	if (re_regex(pl->p, pl->l, "[^.]+.[^]*", &name, NULL))
		name = *pl;

	for (i=0; ; i++) {
		const struct mod_export *me = mod_table[i];
		if (!me)
			return NULL;
		if (0 == pl_strcasecmp(&name, me->name))
			return me;
	}

	// unreachable
	//return NULL;
}
#endif


static int load_module(struct mod **modp, const struct pl *modpath,
		       const struct pl *name)
{
	char file[256];
	struct mod *m = NULL;
	int err = 0;

	if (!name)
		return EINVAL;

#ifdef STATIC
	/* Try static first */
	err = mod_add(&m, find_module(name));
	if (!err)
		goto out;
#endif

	/* Then dynamic */
	if (re_snprintf(file, sizeof(file), "%r/%r", modpath, name) < 0) {
		err = ENOMEM;
		goto out;
	}
	err = mod_load(&m, file);
	if (err)
		goto out;

	if (modp)
		*modp = m;

 out:
	if (err) {
		DEBUG_WARNING("module %r: %m\n", name, err);
	}

	return err;
}

#ifdef STATIC
int load_module2(struct mod **modp, const struct pl *name)
{
	struct mod *m = NULL;
	int err = 0;

	if (!name)
		return EINVAL;

	/* Try static */
	err = mod_add(&m, find_module(name));
	if (err)
		goto out;

	err = mod_call_init(m);
	if (err)
		goto out;

	if (modp)
		*modp = m;

 out:
	if (err) {
		DEBUG_WARNING("module %r: %m\n", name, err);
	}

	return err;
}
#endif

int load_module_dynamic(struct mod **modp, const struct pl *modpath,
			   const struct pl *name)
{
	char file[256];
	struct mod *m = NULL;
	int err = 0;

	if (!name)
		return EINVAL;

	if (re_snprintf(file, sizeof(file), "%r\\%r", modpath, name) < 0) {
		err = ENOMEM;
		goto out;
	}
	err = mod_load(&m, file);
	if (err)
		goto out;

	// set function pointers
    {
		pf_set_mem_alloc_cb pf = (pf_set_mem_alloc_cb)mod_sym(m, "set_mem_alloc_cb");
		if (pf)
			pf(mem_alloc);
	}
	{
		pf_set_mem_deref_cb pf = (pf_set_mem_deref_cb)mod_sym(m, "set_mem_deref_cb");
		if (pf)
			pf(mem_deref);
	}
	{
		pf_set_aucodec_register_cb pf = (pf_set_aucodec_register_cb)mod_sym(m, "set_aucodec_register_cb");
		if (pf)
			pf(aucodec_register);
	}
	{
		pf_set_aucodec_unregister_cb pf = (pf_set_aucodec_unregister_cb)mod_sym(m, "set_aucodec_unregister_cb");
		if (pf)
			pf(aucodec_unregister);
	}

	{
		pf_validate_dll pf = (pf_validate_dll)mod_sym(m, "validate_dll");
		if (pf) {
			int rc = pf();
			if (rc) {
				DEBUG_INFO("Failed to validate dll, rc = %d\n", rc);
				err = ELIBBAD;
				goto out;
			}
		} else {
			DEBUG_INFO("No validate_dll() symbol\n");
			err = ELIBBAD;
			goto out;
        }
	}

	err = mod_call_init(m);
	if (err) {
		goto out;
    }

	if (modp)
		*modp = m;

 out:
	if (err) {
		DEBUG_WARNING("module %r: %m\n", name, err);
	}

	return err;
}

static int module_handler(const struct pl *val, void *arg)
{
	(void)load_module(NULL, arg, val);
	return 0;
}


static int module_tmp_handler(const struct pl *val, void *arg)
{
	struct mod *mod = NULL;
	(void)load_module(&mod, arg, val);
	mem_deref(mod);
	return 0;
}


static int module_app_handler(const struct pl *val, void *arg)
{
	struct modapp *modapp;

	modapp = mem_zalloc(sizeof(*modapp), modapp_destructor);
	if (!modapp)
		return ENOMEM;

	if (load_module(&modapp->mod, arg, val)) {
		mem_deref(modapp);
		return 0;
	}

	list_prepend(&modappl, &modapp->le, modapp);

	return 0;
}


int module_init(const struct conf *conf)
{
	struct pl path;
	int err;

	if (conf_get(conf, "module_path", &path))
		pl_set_str(&path, ".");

	err = conf_apply(conf, "module", module_handler, &path);
	if (err)
		return err;

	err = conf_apply(conf, "module_tmp", module_tmp_handler, &path);
	if (err)
		return err;

	err = conf_apply(conf, "module_app", module_app_handler, &path);
	if (err)
		return err;

	return 0;
}


void module_app_unload(void)
{
	list_flush(&modappl);
}
