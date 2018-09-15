#include <stdlib.h>
#include <string.h>
#include <re.h>
#include <baresip.h>


#define DEBUG_MODULE "softvol"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

#define SOFTVOL_BASE 128

struct softvol_st {
	int dummy;
};

struct enc_st {
	struct aufilt_enc_st af;  /* base class */
	struct softvol_st *st;
};

struct dec_st {
	struct aufilt_dec_st af;  /* base class */
	struct softvol_st *st;
};

static void enc_destructor(void *arg)
{
	struct enc_st *st = arg;

	list_unlink(&st->af.le);
	mem_deref(st->st);
}


static void dec_destructor(void *arg)
{
	struct dec_st *st = arg;

	list_unlink(&st->af.le);
	mem_deref(st->st);
}

static void softvol_destructor(void *arg)
{
	struct softvol_st *st = arg;
    (void)st;
//	mem_deref(st->out);
}


static int softvol_alloc(struct softvol_st **stp, void **ctx, struct aufilt_prm *prm)
{
	struct softvol_st *st;
	int err = 0, tmp;
	int status;
	//const struct config *cfg = conf_config();

	if (!stp || !ctx || !prm)
		return EINVAL;

	if (*ctx) {
		*stp = mem_ref(*ctx);
		return 0;
	}

	st = mem_zalloc(sizeof(*st), softvol_destructor);
	if (!st)
		return ENOMEM;

//	st->skew = cfg->webrtc.skew;



out:
	if (err) {
		mem_deref(st);
	} else {
		*ctx = *stp = st;
	}

	return err;
}


static int encode_update(struct aufilt_enc_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm)
{
	struct enc_st *st;
	int err;

	if (!stp || !ctx || !af || !prm)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), enc_destructor);
	if (!st)
		return ENOMEM;

	err = softvol_alloc(&st->st, ctx, prm);

	if (err)
		mem_deref(st);
	else
		*stp = (struct aufilt_enc_st *)st;

	return err;
}

static int decode_update(struct aufilt_dec_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm)
{
	struct dec_st *st;
	int err;

	if (!stp || !ctx || !af || !prm)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), dec_destructor);
	if (!st)
		return ENOMEM;

	err = softvol_alloc(&st->st, ctx, prm);

	if (err)
		mem_deref(st);
	else
		*stp = (struct aufilt_dec_st *)st;

	return err;
}

static int16_t saturate_s16(int32_t val) {
#if 1
	if (val < -32768) {
		return -32768;
	} else if (val > 32767) {
		return 32767;
	} else {
		return val;
	}
#else
	return val;	
#endif
}

static int encode(struct aufilt_enc_st *st, int16_t *sampv, size_t *sampc)
{
	struct enc_st *est = (struct enc_st *)st;
	struct softvol_st *wr = est->st;
	int status;
	unsigned int i;
	int softvol_tx = (int)(conf_config()->audio.softvol_tx);

	(void)wr;

	for (i=0; i<*sampc; i++) {
		sampv[i] = saturate_s16(((int32_t)sampv[i] * softvol_tx) / SOFTVOL_BASE);
	}

	return 0;
}


static int decode(struct aufilt_dec_st *st, int16_t *sampv, size_t *sampc)
{
	struct dec_st *dst = (struct dec_st *)st;
	struct softvol_st *wr = dst->st;
	int status = 0;
	unsigned int i;
	int softvol_rx = (int)(conf_config()->audio.softvol_rx);

	(void)wr;

	for (i=0; i<*sampc; i++) {
		sampv[i] = saturate_s16(((int32_t)sampv[i] * softvol_rx) / SOFTVOL_BASE);
	}

	return status;
}

static struct aufilt softvol = {
	LE_INIT, "softvol", encode_update, encode, decode_update, decode
};

static int module_init(void) {
	aufilt_register(&softvol);
	return 0;
}

static int module_close(void) {
	aufilt_unregister(&softvol);
	return 0;
}  

EXPORT_SYM const struct mod_export DECL_EXPORTS(softvol) = {
	"softvol",
	"filter",
	module_init,
	module_close
};


