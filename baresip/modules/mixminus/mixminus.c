/**
 * @file mixminus.c  Mixes N-1 audio streams for conferencing
 *
 * Copyright (C) 2021 Sebastian Reimers
 * Copyright (C) 2021 AGFEO GmbH & Co. KG
 */

#include <string.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <baresip_conference.h>

#define DEBUG_MODULE "mixminus"
#define DEBUG_LEVEL 7
#include <re_dbg.h>

enum {
	MAX_SRATE       = 48000,  /* Maximum sample rate in [Hz] */
	MAX_CHANNELS    =     2,  /* Maximum number of channels  */
	MAX_PTIME       =    60,  /* Maximum packet time in [ms] */

	AUDIO_SAMPSZ    = MAX_SRATE * MAX_CHANNELS * MAX_PTIME / 1000
};

struct mix {
	struct aubuf *ab;
	const struct audio *au;
	struct aufilt_prm prm;
	bool ready;
	struct le le_priv;
};

struct mixminus_enc {
	struct aufilt_enc_st af;  /* inheritance */

	const struct audio *au;
	struct list mixers;
	int16_t *sampv;
	int16_t *rsampv;
	int16_t *fsampv;
	struct auresamp resamp;
	struct aufilt_prm prm;
	struct le le_priv;
};

struct mixminus_dec {
	struct aufilt_dec_st af;  /* inheritance */

	const struct audio *au;
	int16_t *fsampv;
	struct aufilt_prm prm;
};

static struct list encs;
static struct lock* mixminus_lock = NULL;


static void enc_destructor(void *arg)
{
	struct mixminus_enc *st = arg;
	struct mixminus_enc *enc;
	struct le *le, *lem;
	struct mix *mix;

	lock_write_get(mixminus_lock);	

	DEBUG_INFO("mixminus: destructing enc %p\n", st);

	list_flush(&st->mixers);
	mem_deref(st->sampv);
	mem_deref(st->rsampv);
	mem_deref(st->fsampv);
	list_unlink(&st->le_priv);

	for (le = list_head(&encs); le; le = le->next) {
		enc = le->data;
		if (!enc)
			continue;

		lem = list_head(&enc->mixers);
		while (lem) {
			mix = lem->data;
			lem = lem->next;

			if (st->au != mix->au)
				continue;

			mix->ready = false;
			list_unlink(&mix->le_priv);
			mem_deref(mix);
		}
	}

	DEBUG_INFO("mixminus: destructed enc %p\n", st);
	lock_rel(mixminus_lock);

}

static void dec_destructor(void *arg)
{
	struct mixminus_dec *st = arg;
	DEBUG_INFO("mixminus: destructing dec %p\n", st);
	mem_deref(st->fsampv);
	DEBUG_INFO("mixminus: destructed dec %p\n", st);
}

static void mix_destructor(void *arg)
{
	struct mix *mix = arg;
	DEBUG_INFO("mixminus: destructing mix %p\n", mix);
	mem_deref(mix->ab);
}

static int encode_update(struct aufilt_enc_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm,
			 const struct audio *au)
{
	struct mixminus_enc *st, *enc;
	struct mix *mix;
	size_t psize;
	struct le *le;
	int err = 0;
	(void)af;

	if (!stp || !ctx || !prm)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), enc_destructor);
	if (!st)
		return ENOMEM;

	lock_write_get(mixminus_lock);

	psize = AUDIO_SAMPSZ * sizeof(int16_t);

	st->sampv = mem_zalloc(psize, NULL);
	if (!st->sampv) {
		err = ENOMEM;
		goto out;
	}

	st->rsampv = mem_zalloc(psize, NULL);
	if (!st->rsampv) {
		err = ENOMEM;
		goto out;
	}

	st->fsampv = mem_zalloc(psize, NULL);
	if (!st->fsampv) {
		err = ENOMEM;
		goto out;
	}

	st->prm = *prm;
	st->au = au;
	auresamp_init(&st->resamp);

	list_append(&encs, &st->le_priv, st);

	/* add mix to other encs */
	for (le = list_head(&encs); le; le = le->next) {
		enc = le->data;
		if (!enc)
			continue;
		if (enc->au == au)
			continue;

		mix = mem_zalloc(sizeof(*mix), mix_destructor);
		if (!mix)
			return ENOMEM;
		psize = st->prm.srate * st->prm.ch * 20 / 1000;
		err = aubuf_alloc(&mix->ab, psize, 5 * psize);
		if (err)
			goto out;

		mix->au = st->au; /* using audio object as id */
		mix->ready = false;

		list_append(&enc->mixers, &mix->le_priv, mix);
	}

	/* add other mixes to new enc */
	for (le = list_head(&encs); le; le = le->next) {
		enc = le->data;
		if (!enc)
			continue;
		if (enc->au == au)
			continue;

		mix = mem_zalloc(sizeof(*mix), mix_destructor);
		if (!mix) {
			err = ENOMEM;
			goto out;
		}

		psize = enc->prm.srate * enc->prm.ch * 20 / 1000;
		err = aubuf_alloc(&mix->ab, psize, 5 * psize);
		if (err)
			goto out;

		mix->au = enc->au; /* using audio object as id */
		mix->ready = false;

		list_append(&st->mixers, &mix->le_priv, mix);
	}

 out:
	if (err)
		mem_deref(st);
	else {
		*stp = (struct aufilt_enc_st *) st;
	}

	lock_rel(mixminus_lock);

	return err;
}


static int decode_update(struct aufilt_dec_st **stp, void **ctx,
			 const struct aufilt *af, struct aufilt_prm *prm,
			 const struct audio *au)
{
	struct mixminus_dec *st;
	size_t psize;
	(void)af;
	(void)prm;

	if (!stp || !ctx)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), dec_destructor);
	if (!st)
		return ENOMEM;

	psize = AUDIO_SAMPSZ * sizeof(int16_t);

	st->fsampv = mem_zalloc(psize, NULL);
	if (!st->fsampv)
		return ENOMEM;

	st->au = au;
	st->prm = *prm;

	*stp = (struct aufilt_dec_st *)st;

	return 0;
}


static void read_samp(struct aubuf *ab, int16_t *sampv, size_t sampc,
		      size_t stime)
{
	size_t i;
	size_t psize = sampc * 2;

	for (i = 0; i < stime - 1; i++) {
		if (aubuf_cur_size(ab) < psize) {
			sys_msleep(1);
		}
		else {
			break;
		}
	}

	aubuf_read_samp(ab, sampv, sampc);
}


static int encode(struct aufilt_enc_st *aufilt_enc_st, int16_t *sampv, size_t *sampc)
{
	struct mixminus_enc *enc = (struct mixminus_enc *)aufilt_enc_st;
	size_t i, inc, outc, stime;
	struct le *lem;
	struct mix *mix;
	int16_t *sampv_mix = enc->sampv;
	int32_t sample;
	int err = 0;

	lock_write_get(mixminus_lock);

	stime = 1000 * (*sampc) / (enc->prm.srate * enc->prm.ch);

	for (lem = list_head(&enc->mixers); lem; lem = lem->next) {
		mix = lem->data;
		if (!mix)
			continue;

		if (!audio_is_conference(mix->au))
			continue;

		if (!mix->ready) {
			mix->ready = true;
			continue;
		}

		if (!mix->prm.srate || !mix->prm.ch)
			continue;

		err = auresamp_setup(&enc->resamp, mix->prm.srate, mix->prm.ch,
				     enc->prm.srate, enc->prm.ch);
		if (err) {
			DEBUG_WARNING("mixminus/auresamp_setup error (%m)\n", err);
			goto out;
		}

		if (enc->resamp.resample) {
			outc = AUDIO_SAMPSZ;
			sampv_mix = enc->rsampv;

			if (enc->prm.srate > mix->prm.srate) {
				inc = *sampc / enc->resamp.ratio;
			}
			else {
				inc = *sampc * enc->resamp.ratio;
			}

			if (enc->prm.ch == 2 && mix->prm.ch == 1) {
				inc = inc / 2;
			}

			if (enc->prm.ch == 1 && mix->prm.ch == 2) {
				inc = inc * 2;
			}

			read_samp(mix->ab, enc->sampv, inc, stime);

			err = auresamp(&enc->resamp, sampv_mix, &outc,
				       enc->sampv, inc);
			if (err) {
				DEBUG_WARNING("mixminus/auresamp error (%m)\n", err);
				goto out;
			}
			if (outc != *sampc) {
				DEBUG_WARNING("mixminus/auresamp sample count error\n");
				goto out;
			}
		}
		else {
            DEBUG_INFO("read_samp from mix %p\n", mix);
			read_samp(mix->ab, sampv_mix, *sampc, stime);
            DEBUG_INFO("read_samp from mix %p done\n", mix);
		}

		for (i = 0; i < *sampc; i++) {
			sample = sampv[i] + sampv_mix[i];

			/* soft clipping */
			if (sample >= 32767)
				sample = 32767;
			if (sample <= -32767)
				sample = -32767;
			sampv[i] = sample;
		}
	}

out:
	lock_rel(mixminus_lock);

	return err;
}


static int decode(struct aufilt_dec_st *aufilt_dec_st, int16_t *sampv, size_t *sampc)
{
	struct mixminus_dec *dec = (struct mixminus_dec *)aufilt_dec_st;
	struct mixminus_enc *enc;
	struct le *le;
	struct le *lem;
	struct mix *mix;

	lock_write_get(mixminus_lock);

	for (le = list_head(&encs); le; le = le->next) {
		enc = le->data;
		if (!enc)
			continue;

		for (lem = list_head(&enc->mixers); lem; lem = lem->next) {
			mix = lem->data;

			if (!mix || dec->au != mix->au)
				continue;

			if (!mix->ready)
				continue;

			mix->prm.ch = dec->prm.ch;
			mix->prm.srate = dec->prm.srate;

			aubuf_write_samp(mix->ab, sampv, *sampc);
		}
	}

	lock_rel(mixminus_lock);

	return 0;
}


int baresip_debug_conference(void)
{
	struct mixminus_enc *enc;
	struct mix *mix;
	struct le *le, *lem;
	unsigned int encs_count = 0;

	lock_write_get(mixminus_lock);

	for (le = list_head(&encs); le; le = le->next) {
		enc = le->data;
		if (!enc)
			continue;

		encs_count++;

		DEBUG_WARNING("mixminus/enc au %x:"
			 "ch %d srate %d, is_conference (%s)\n",
			 enc->au, enc->prm.ch, enc->prm.srate,
			 audio_is_conference(enc->au) ? "true" : "false");

		for (lem = list_head(&enc->mixers); lem; lem = lem->next) {
			mix = lem->data;

			DEBUG_WARNING("\tmix au %x: ch %d srate %d %H\n", mix->au,
				 mix->prm.ch, mix->prm.srate, aubuf_debug,
				 mix->ab);
		}
	}

	if (encs_count == 0) {
		DEBUG_WARNING("mixminus: no encs\n");
	}

	lock_rel(mixminus_lock);

	return 0;
}


int baresip_start_conference(void)
{
	struct le *le, *lec;
	struct call *call;
	struct ua *ua;
	struct audio *au;

	for (le = list_head(uag_list()); le; le = le->next) {
		ua = le->data;
		lec = NULL;

		for (lec = list_head(ua_calls(ua)); lec; lec = lec->next) {
			call = lec->data;
			DEBUG_WARNING("conference with %s\n", call_peeruri(call));
			/* call_hold(call, false); */ /* managed by GUI part */
			au = call_audio(call);
			audio_set_conference(au, true);
		}
	}

	baresip_debug_conference();

	return 0;
}

static struct aufilt mixminus = {
	LE_INIT, "mixminus", encode_update, encode, decode_update, decode
};



static int module_init(void)
{
	int err = lock_alloc(&mixminus_lock);
	if (err)
		return err;

	aufilt_register(&mixminus);

	return 0;
}


static int module_close(void)
{
	aufilt_unregister(&mixminus);
	mixminus_lock = mem_deref(mixminus_lock);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(mixminus) = {
	"mixminus",
	"filter",
	module_init,
	module_close
};
