/**
 * @file aufile.c WAV Audio Source
 *
 * Copyright (C) 2015 Creytiv.com
 *
 * Modified for Win32 build.
 */

#define _BSD_SOURCE 1
//#include <pthread.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>

#define DEBUG_MODULE "aufile"
#define DEBUG_LEVEL 6
#include <re_dbg.h>

/**
 * @defgroup aufile aufile
 *
 * Audio module for using a WAV-file as audio input
 */


struct ausrc_st {
	struct ausrc *as;  /* base class */
	struct tmr tmr;
	struct aufile *aufile;
	struct aubuf *aubuf;
	uint32_t ptime;
	size_t sampc;
	bool run;
	bool terminated;
	//pthread_t thread;
	HANDLE thread;
	ausrc_read_h *rh;
	ausrc_error_h *errh;
	void *arg;
};


static struct ausrc *ausrc;


static void destructor(void *arg)
{
	struct ausrc_st *st = arg;

	if (st->run) {
		st->run = false;
		//pthread_join(st->thread, NULL);
		while (!st->terminated) {
			Sleep(10);
		}
		Sleep(10);
		CloseHandle(st->thread);
	}

	tmr_cancel(&st->tmr);

	mem_deref(st->aufile);
	mem_deref(st->aubuf);
	mem_deref(st->as);	
}

DWORD WINAPI play_thread(LPVOID arg)
//static void *play_thread(void *arg)
{
	uint64_t now, ts = tmr_jiffies();
	struct ausrc_st *st = arg;
	int16_t *sampv;

	sampv = mem_alloc(st->sampc * 2, NULL);
	if (!sampv)
		return NULL;

	while (st->run) {

		Sleep(4);

		now = tmr_jiffies();

		if (ts > now)
			continue;

		aubuf_read(st->aubuf, (uint8_t*)sampv, st->sampc*sizeof(int16_t));

		st->rh((uint8_t *)sampv, st->sampc*sizeof(int16_t), st->arg);

		ts += st->ptime;
	}

	mem_deref(sampv);

	DEBUG_INFO("aufile: player thread exited\n");

	st->terminated = true;

	return 0;
}


static void timeout(void *arg)
{
	struct ausrc_st *st = arg;

	tmr_start(&st->tmr, 1000, timeout, st);

	/* check if audio buffer is empty */
	if (aubuf_cur_size(st->aubuf) < (2 * st->sampc)) {

		DEBUG_INFO("aufile: end of file\n");

		/* error handler must be called from re_main thread */
		if (st->errh)
			st->errh(0, "end of file", st->arg);
	}
}


static int read_file(struct ausrc_st *st)
{
	struct mbuf *mb;
	int err;

	for (;;) {

		mb = mbuf_alloc(4096);
		if (!mb)
			return ENOMEM;

		mb->end = mb->size;

		err = aufile_read(st->aufile, mb->buf, &mb->end);
		if (err)
			break;

		if (mb->end == 0) {
			DEBUG_INFO("aufile: end of file\n");
			break;
		}

		aubuf_append(st->aubuf, mb);

		mb = mem_deref(mb);
	}

	DEBUG_INFO("aufile: loaded %zu bytes\n", aubuf_cur_size(st->aubuf));

	mem_deref(mb);
	return err;
}

static int alloc_handler(struct ausrc_st **stp, struct ausrc *as,
			 struct media_ctx **ctx,
			 struct ausrc_prm *prm, const char *dev,
			 ausrc_read_h *rh, ausrc_error_h *errh, void *arg)
{
	struct ausrc_st *st;
	struct aufile_prm fprm;
	int err;
	DWORD dwtid;
	(void)ctx;

	if (!stp || !as || !prm || !rh)
		return EINVAL;

	DEBUG_INFO("aufile: loading input file '%s'\n", dev);

	st = mem_zalloc(sizeof(*st), destructor);
	if (!st)
		return ENOMEM;

	st->as   = mem_ref(as);
	st->rh   = rh;
	st->errh = errh;
	st->arg  = arg;

	err = aufile_open(&st->aufile, &fprm, dev, AUFILE_READ);
	if (err) {
		DEBUG_WARNING("aufile: failed to open file '%s' (%m)\n", dev, err);
		goto out;
	}

	DEBUG_INFO("aufile: %s: %u Hz, %d channels\n",
	     dev, fprm.srate, fprm.channels);

	if (fprm.srate != prm->srate) {
		DEBUG_WARNING("aufile: input file (%s) must have sample-rate"
			" %u Hz\n", dev, prm->srate);
		err = ENODEV;
		goto out;
	}
	if (fprm.channels != prm->ch) {
		DEBUG_WARNING("aufile: input file (%s) must have channels = %d\n",
			dev, prm->ch);
		err = ENODEV;
		goto out;
	}
	if (fprm.fmt != AUFMT_S16LE) {
		DEBUG_WARNING("aufile: input file must have format S16LE\n");
		err = ENODEV;
		goto out;
	}

	st->sampc = prm->frame_size; //prm->srate * prm->ch * prm->ptime / 1000;

	st->ptime = prm->frame_size * 1000 / prm->srate / prm->ch; //prm->ptime;

	DEBUG_INFO("aufile: audio ptime=%u sampc=%zu aubuf=[%u:%u]\n",
	     st->ptime, st->sampc,
	     prm->srate * prm->ch * 2,
	     prm->srate * prm->ch * 40);

	/* 1 - inf seconds of audio */
	err = aubuf_alloc(&st->aubuf,
			  prm->srate * prm->ch * 2,
			  0);
	if (err)
		goto out;

	err = read_file(st);
	if (err)
		goto out;

	tmr_start(&st->tmr, 1000, timeout, st);

	st->run = true;
	//err = pthread_create(&st->thread, NULL, play_thread, st);
	st->thread = CreateThread(NULL, 0, play_thread, st, 0, &dwtid);
	if (st->thread == NULL) {
		st->run = false;
		goto out;
	}

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}


static int module_init(void)
{
	return ausrc_register(&ausrc, "aufile", alloc_handler);
}


static int module_close(void)
{
	ausrc = mem_deref(ausrc);

	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(aufile) = {
	"aufile",
	"ausrc",
	module_init,
	module_close
};
