#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <re.h>
#include <baresip.h>

#include "agc.h"
#include "gate.h"


#define DEBUG_MODULE "softvol"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

#define SOFTVOL_BASE 128
#define SOFTVOL_METER_BLOCK_MS 20	/* level is published every 20 ms */

/* Level meter of one direction (TX or RX) of one audio stream.
 * Accumulated in the audio thread, published (under meter_mtx) per block. */
struct meter {
	unsigned int blk_len;	/* samples per block */
	unsigned int blk_cnt;
	int blk_peak;
	double blk_sumsq;
	int peak;		/* last complete block */
	float rms;		/* last complete block */
};

struct softvol_st {
	struct le le;		/* in meter_list */
	const struct audio *au;
	unsigned int meter_id;	/* 0 = not bound to a call */
	struct meter tx;
	struct meter rx;
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

/* Registered streams; accessed from audio threads and from the GUI thread */
static mtx_t meter_mtx;
static struct list meter_list;

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

	mtx_lock(&meter_mtx);
	list_unlink(&st->le);
	mtx_unlock(&meter_mtx);
}


static int softvol_alloc(struct softvol_st **stp, void **ctx, struct aufilt_prm *prm,
			 const struct audio *au)
{
	struct softvol_st *st;
	unsigned int blk_len;
	int err = 0;

	if (!stp || !ctx || !prm)
		return EINVAL;

	if (*ctx) {
		*stp = mem_ref(*ctx);
		return 0;
	}

	st = mem_zalloc(sizeof(*st), softvol_destructor);
	if (!st)
		return ENOMEM;

	st->au = au;

	/* block length in samples, all channels */
	blk_len = prm->srate * prm->ch * SOFTVOL_METER_BLOCK_MS / 1000;
	if (blk_len == 0)
		blk_len = 1;
	st->tx.blk_len = blk_len;
	st->rx.blk_len = blk_len;

	mtx_lock(&meter_mtx);
	list_append(&meter_list, &st->le, st);
	mtx_unlock(&meter_mtx);

out:
	if (err) {
		mem_deref(st);
	} else {
		*ctx = *stp = st;
	}

	return err;
}


static int encode_update(struct aufilt_enc_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm, const struct audio *au)
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

	gate_reset(&st->gate_state, prm->srate);

	err = softvol_alloc(&st->st, ctx, prm, au);

	if (err)
		mem_deref(st);
	else
		*stp = (struct aufilt_enc_st *)st;

	return err;
}

static int decode_update(struct aufilt_dec_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm, const struct audio *au)
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

	err = softvol_alloc(&st->st, ctx, prm, au);

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

/* Accumulate peak and RMS; publish every block. Samples are post gain. */
static void meter_samples(struct meter *m, const int16_t *sampv, size_t sampc)
{
	size_t i;

	for (i=0; i<sampc; i++) {
		int val = sampv[i];

		if (val < 0)
			val = -val;
		if (val > m->blk_peak)
			m->blk_peak = val;
		m->blk_sumsq += (double)sampv[i] * (double)sampv[i];

		if (++m->blk_cnt >= m->blk_len) {
			mtx_lock(&meter_mtx);
			m->peak = m->blk_peak;
			m->rms = (float)sqrt(m->blk_sumsq / m->blk_cnt);
			mtx_unlock(&meter_mtx);

			m->blk_peak = 0;
			m->blk_sumsq = 0.0;
			m->blk_cnt = 0;
		}
	}
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

	for (i=0; i<*sampc; i++) {
		sampv[i] = saturate_s16(((int32_t)sampv[i] * softvol_tx) / SOFTVOL_BASE);
	}

	meter_samples(&wr->tx, sampv, *sampc);

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

	if (agc_enabled && agc_target != 0) {
		agc_process(&dst->agc_state, sampv, *sampc, agc_target, max_gain, attack_rate, release_rate);
	}

	for (i=0; i<*sampc; i++) {
		sampv[i] = saturate_s16(((int32_t)sampv[i] * softvol_rx) / SOFTVOL_BASE);
	}

	meter_samples(&softvol->rx, sampv, *sampc);

	return status;
}

static struct aufilt softvol = {
	LE_INIT, "softvol", encode_update, encode, decode_update, decode
};

static int module_init(void) {
	mtx_init(&meter_mtx, mtx_plain);
	list_init(&meter_list);

	aufilt_register(&softvol);
	return 0;
}

static int module_close(void) {
	aufilt_unregister(&softvol);
	mtx_destroy(&meter_mtx);
	return 0;
}

/* Bind the level meter of an audio stream (one call) to an id chosen by the caller */
void softvol_meter_bind(const struct audio *au, unsigned int id)
{
	struct le *le;

	mtx_lock(&meter_mtx);
	for (le = list_head(&meter_list); le; le = le->next) {
		struct softvol_st *st = le->data;

		if (st->au == au)
			st->meter_id = id;
	}
	mtx_unlock(&meter_mtx);
}

bool softvol_get_levels(unsigned int id, struct softvol_levels *lv)
{
	struct le *le;
	bool found = false;

	if (!lv || id == 0)
		return false;

	memset(lv, 0, sizeof(*lv));

	mtx_lock(&meter_mtx);
	for (le = list_head(&meter_list); le; le = le->next) {
		struct softvol_st *st = le->data;

		if (st->meter_id != id)
			continue;

		lv->rx_peak = st->rx.peak;
		lv->rx_rms = st->rx.rms;
		lv->tx_peak = st->tx.peak;
		lv->tx_rms = st->tx.rms;
		found = true;
		break;
	}
	mtx_unlock(&meter_mtx);

	return found;
}

EXPORT_SYM const struct mod_export DECL_EXPORTS(softvol) = {
	"softvol",
	"filter",
	module_init,
	module_close
};

