/**
 * @file sidetone.c  Sidetone: transmitted audio mixed into local playback,
 * controlled per call (sidetone_set), disabled by default.
 *
 * Two audio filters with separate positions in filter list (see conf.c):
 * - "sidetone_tap" (encode only): copies transmitted audio (after TX softvol),
 * - "sidetone_mix" (decode only): mixes copy into received audio AFTER
 *   recorder (sidetone is not recorded) and BEFORE AEC (AEC reference
 *   contains sidetone, so it is not echoed back to remote party).
 *
 * TX and RX run in different threads with independent clocks: bounded
 * aubuf (min/max size) handles drift - underrun gives silence, overflow
 * drops oldest samples.
 */
#include <string.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>

#define DEBUG_MODULE "sidetone"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

enum {
	LEVEL_DEFAULT = 50,		/* percent of transmitted audio level */
	LEVEL_MAX = 200,
	BUF_MIN_MS = 20,
	BUF_MAX_MS = 160
};

/** per-audio (per-call) state shared by tap and mix */
struct sidetone_st {
	struct le le;
	const struct audio *au;		/* audio object as id */
	struct aubuf *ab;
	bool enabled;
	unsigned int level;		/* percent */
	uint32_t srate;			/* TX parameters (tap) */
	uint8_t ch;
	bool mismatch_logged;
};

struct tap_st {
	struct aufilt_enc_st af;	/* base class */
	struct sidetone_st *st;
};

struct mix_st {
	struct aufilt_dec_st af;	/* base class */
	struct sidetone_st *st;
	uint32_t srate;			/* RX parameters */
	uint8_t ch;
	int16_t *sampv;
	size_t sampv_size;
};

static struct list states;
static struct lock *lock = NULL;


static void state_destructor(void *arg)
{
	struct sidetone_st *st = arg;

	lock_write_get(lock);
	list_unlink(&st->le);
	lock_rel(lock);
	mem_deref(st->ab);
}


static struct sidetone_st *state_find(const struct audio *au)
{
	struct le *le;

	for (le = list_head(&states); le; le = le->next) {
		struct sidetone_st *st = le->data;
		if (st->au == au)
			return st;
	}
	return NULL;
}


/** get (referenced) state for audio object, create if not existing */
static int state_get(struct sidetone_st **stp, const struct audio *au)
{
	struct sidetone_st *st;

	lock_write_get(lock);
	st = state_find(au);
	if (st) {
		*stp = mem_ref(st);
		lock_rel(lock);
		return 0;
	}
	st = mem_zalloc(sizeof(*st), state_destructor);
	if (!st) {
		lock_rel(lock);
		return ENOMEM;
	}
	st->au = au;
	st->level = LEVEL_DEFAULT;
	list_append(&states, &st->le, st);
	*stp = st;
	lock_rel(lock);
	return 0;
}


static void tap_destructor(void *arg)
{
	struct tap_st *tap = arg;

	list_unlink(&tap->af.le);
	mem_deref(tap->st);
}


static void mix_destructor(void *arg)
{
	struct mix_st *mix = arg;

	list_unlink(&mix->af.le);
	mem_deref(mix->st);
	mem_deref(mix->sampv);
}


static int tap_update(struct aufilt_enc_st **stp, void **ctx,
		      const struct aufilt *af, struct aufilt_prm *prm,
		      const struct audio *au)
{
	struct tap_st *tap;
	size_t psize;
	int err;
	(void)ctx;

	if (!stp || !af || !prm || !au)
		return EINVAL;
	if (*stp)
		return 0;

	tap = mem_zalloc(sizeof(*tap), tap_destructor);
	if (!tap)
		return ENOMEM;

	err = state_get(&tap->st, au);
	if (err)
		goto out;

	lock_write_get(lock);
	tap->st->srate = prm->srate;
	tap->st->ch = prm->ch;
	tap->st->ab = mem_deref(tap->st->ab);
	psize = 2 * prm->srate * prm->ch / 1000;	/* bytes per ms */
	err = aubuf_alloc(&tap->st->ab, "sidetone", psize * BUF_MIN_MS, psize * BUF_MAX_MS);
	lock_rel(lock);

 out:
	if (err)
		mem_deref(tap);
	else
		*stp = (struct aufilt_enc_st *)tap;
	return err;
}


static int mix_update(struct aufilt_dec_st **stp, void **ctx,
		      const struct aufilt *af, struct aufilt_prm *prm,
		      const struct audio *au)
{
	struct mix_st *mix;
	int err;
	(void)ctx;

	if (!stp || !af || !prm || !au)
		return EINVAL;
	if (*stp)
		return 0;

	mix = mem_zalloc(sizeof(*mix), mix_destructor);
	if (!mix)
		return ENOMEM;

	mix->srate = prm->srate;
	mix->ch = prm->ch;
	err = state_get(&mix->st, au);

	if (err)
		mem_deref(mix);
	else
		*stp = (struct aufilt_dec_st *)mix;
	return err;
}


static int tap(struct aufilt_enc_st *aufilt_enc_st, int16_t *sampv, size_t *sampc)
{
	struct tap_st *tap = (struct tap_st *)aufilt_enc_st;
	struct sidetone_st *st = tap->st;

	lock_write_get(lock);
	if (st->enabled && st->ab)
		aubuf_write_samp(st->ab, sampv, *sampc);
	lock_rel(lock);

	return 0;
}


static int mix(struct aufilt_dec_st *aufilt_dec_st, int16_t *sampv, size_t *sampc)
{
	struct mix_st *mix = (struct mix_st *)aufilt_dec_st;
	struct sidetone_st *st = mix->st;
	unsigned int level;
	size_t i;

	lock_write_get(lock);
	if (!st->enabled || !st->ab) {
		lock_rel(lock);
		return 0;
	}
	if (st->srate != mix->srate || st->ch != mix->ch) {
		/* different TX/RX format (unusual) - resampling not implemented */
		if (!st->mismatch_logged) {
			DEBUG_WARNING("TX/RX format differs (%u Hz/%u ch vs %u Hz/%u ch), "
				"sidetone not mixed\n", st->srate, st->ch, mix->srate, mix->ch);
			st->mismatch_logged = true;
		}
		lock_rel(lock);
		return 0;
	}
	if (mix->sampv_size < *sampc) {
		mix->sampv = mem_deref(mix->sampv);
		mix->sampv = mem_alloc(*sampc * sizeof(int16_t), NULL);
		if (!mix->sampv) {
			mix->sampv_size = 0;
			lock_rel(lock);
			return ENOMEM;
		}
		mix->sampv_size = *sampc;
	}
	/* silence on underrun */
	aubuf_read_samp(st->ab, mix->sampv, *sampc);
	level = st->level;
	lock_rel(lock);

	for (i = 0; i < *sampc; i++) {
		int32_t v = (int32_t)sampv[i] + ((int32_t)mix->sampv[i] * (int32_t)level) / 100;
		if (v > 32767)
			v = 32767;
		else if (v < -32768)
			v = -32768;
		sampv[i] = (int16_t)v;
	}

	return 0;
}


/**
 * Enable/disable sidetone for audio stream of a call
 *
 * @param au      Audio object (call_audio())
 * @param enabled true to mix transmitted audio into local playback
 * @param level   Sidetone level in percent of transmitted audio (0..200)
 *
 * @return 0 if success, ENOENT if audio filters are not set up (yet)
 */
int sidetone_set(const struct audio *au, bool enabled, unsigned int level)
{
	struct sidetone_st *st;

	if (!lock)
		return ENOENT;
	if (level > LEVEL_MAX)
		level = LEVEL_MAX;

	lock_write_get(lock);
	st = state_find(au);
	if (!st) {
		lock_rel(lock);
		return ENOENT;
	}
	if (enabled && !st->enabled && st->ab)
		aubuf_flush(st->ab);	/* start with minimal latency */
	st->enabled = enabled;
	st->level = level;
	lock_rel(lock);
	return 0;
}


/** Get sidetone state; @return 0 if success, ENOENT if not available */
int sidetone_get(const struct audio *au, bool *enabled, unsigned int *level)
{
	struct sidetone_st *st;

	if (!lock)
		return ENOENT;
	lock_write_get(lock);
	st = state_find(au);
	if (st) {
		if (enabled)
			*enabled = st->enabled;
		if (level)
			*level = st->level;
	}
	lock_rel(lock);
	return st ? 0 : ENOENT;
}


static struct aufilt sidetone_tap = {
	LE_INIT, "sidetone_tap", tap_update, tap, NULL, NULL
};

static struct aufilt sidetone_mix = {
	LE_INIT, "sidetone_mix", NULL, NULL, mix_update, mix
};


/* Module is registered twice (two exports) so that each filter can be placed
   at its own position in filter list (load order) */
static int tap_module_init(void)
{
	int err = 0;
	if (!lock)
		err = lock_alloc(&lock);
	if (err)
		return err;
	aufilt_register(&sidetone_tap);
	return 0;
}


static int tap_module_close(void)
{
	aufilt_unregister(&sidetone_tap);
	return 0;
}


static int mix_module_init(void)
{
	int err = 0;
	if (!lock)
		err = lock_alloc(&lock);
	if (err)
		return err;
	aufilt_register(&sidetone_mix);
	return 0;
}


static int mix_module_close(void)
{
	aufilt_unregister(&sidetone_mix);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(sidetone_tap) = {
	"sidetone_tap",
	"filter",
	tap_module_init,
	tap_module_close
};

EXPORT_SYM const struct mod_export DECL_EXPORTS(sidetone_mix) = {
	"sidetone_mix",
	"filter",
	mix_module_init,
	mix_module_close
};
