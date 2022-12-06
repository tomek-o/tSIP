#include <stdlib.h>
#include <string.h>
#include <re.h>
#include <baresip.h>

#include "agc.h"
#include "gate.h"


#define DEBUG_MODULE "softvol"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

#define SOFTVOL_BASE 128

static unsigned int rx_level = 0;	/* maximum from 100 ms interval; ugly: global */

struct softvol_st {
	int samples_per_measurement;
	int sample_counter;
	unsigned int rx_level_max;
};

struct enc_st {
	struct aufilt_enc_st af;  /* base class */
	struct softvol_st *st;
	struct gate_st gate_state;
};

struct dec_st {
	struct aufilt_dec_st af;  /* base class */
	struct softvol_st *st;
	struct agc_st agc_state;
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

	st->samples_per_measurement = prm->srate / 10;

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

	struct config *cfg = conf_config();

	if (!stp || !ctx || !af || !prm)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), enc_destructor);
	if (!st)
		return ENOMEM;

	gate_reset(&st->gate_state, prm->srate);

	err = softvol_alloc(&st->st, ctx, prm);

	if (err)
		mem_deref(st);
	else
		*stp = (struct aufilt_enc_st *)st;

	rx_level = 0;

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

	agc_reset(&st->agc_state);

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

	struct config *cfg = conf_config();
	int softvol_tx = (int)(cfg->audio.softvol_tx);
	
	if (cfg->audio.gate_tx.enabled) {
		gate_process(&est->gate_state, sampv, *sampc,
			cfg->audio.gate_tx.close_threshold, cfg->audio.gate_tx.hold_ms, cfg->audio.gate_tx.attack_ms, cfg->audio.gate_tx.release_ms);
	}

	(void)wr;

	for (i=0; i<*sampc; i++) {
		sampv[i] = saturate_s16(((int32_t)sampv[i] * softvol_tx) / SOFTVOL_BASE);
	}

	return 0;
}


static int decode(struct aufilt_dec_st *st, int16_t *sampv, size_t *sampc)
{
	struct dec_st *dst = (struct dec_st *)st;
	struct softvol_st *softvol = dst->st;
	int status = 0;
	unsigned int i;

	struct config *cfg = conf_config();

	int softvol_rx = (int)(cfg->audio.softvol_rx);
	bool agc_enabled = cfg->audio.agc_rx.enabled;
	uint16_t agc_target = cfg->audio.agc_rx.target;
	float max_gain = cfg->audio.agc_rx.max_gain;
	float attack_rate = cfg->audio.agc_rx.attack_rate;
	float release_rate = cfg->audio.agc_rx.release_rate;

	/* Looking for peak values in 100 ms intervals */
	for (i=0; i<*sampc; i++) {
		int val = sampv[i];
		if (val < 0)
			val = -val;
		if (val > softvol->rx_level_max) {
			softvol->rx_level_max = val;
		}
		softvol->sample_counter++;
		if (softvol->sample_counter > softvol->samples_per_measurement) {
			rx_level = softvol->rx_level_max;
			softvol->rx_level_max = 0;
			softvol->sample_counter = 0;
		}
	}

	if (agc_enabled && agc_target != 0) {
		agc_process(&dst->agc_state, sampv, *sampc, agc_target, max_gain, attack_rate, release_rate);
	}

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

unsigned int softvol_get_rx_level(void) {
	return rx_level;
}

EXPORT_SYM const struct mod_export DECL_EXPORTS(softvol) = {
	"softvol",
	"filter",
	module_init,
	module_close
};


