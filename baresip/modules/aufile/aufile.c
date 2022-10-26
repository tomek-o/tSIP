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
#include <speex/speex_resampler.h>
#include <assert.h>

#define DEBUG_MODULE "aufile"
#define DEBUG_LEVEL 6
#include <re_dbg.h>
#define LOG_PROMPT DEBUG_MODULE": "

/**
 * @defgroup aufile aufile
 *
 * Audio module for using a WAV-file as audio input
 */

static const int SPEEX_RESAMP_QUALITY = 1;	// 0...10
static const int TIMER_TEST_EOF = 100; 		// ms

struct ausrc_st {
	struct ausrc *as;  /* base class */
	struct tmr tmr;
	struct aufile *aufile;
	struct aubuf *aubuf;
	struct aufile_prm fprm;	
	uint32_t ptime;
	size_t sampc;
	bool run;
	bool terminated;
	//pthread_t thread;
	HANDLE thread;
	ausrc_read_h *rh;
	ausrc_error_h *errh;
	void *arg;

	SpeexResamplerState *speex_state;
	unsigned		 in_samples_per_frame;
	unsigned		 out_samples_per_frame;
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

	if (st->speex_state) {
		speex_resampler_destroy(st->speex_state);
		st->speex_state = NULL;
	}

	mem_deref(st->aufile);
	mem_deref(st->aubuf);
	mem_deref(st->as);	
}

DWORD WINAPI play_thread(LPVOID arg)
//static void *play_thread(void *arg)
{
	uint64_t now, ts = tmr_jiffies();
	struct ausrc_st *st = arg;
	int16_t *source_samples = NULL;
	int16_t *sampv;

	HANDLE timer;   /* Timer handle */
	LARGE_INTEGER li;   /* Time defintion */

	li.LowPart = 0;
	li.HighPart = 0;

	if (st->speex_state) {
		source_samples = mem_alloc(st->in_samples_per_frame * 2, NULL);
		if (!source_samples) {
			return NULL;
		}
	}
	sampv = mem_alloc(st->sampc * 2, NULL);
	if (!sampv) {
		mem_deref(source_samples);
		return NULL;
	}

	/* Create timer */
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (timer == NULL) {
		DEBUG_WARNING("Error creating waitable timer!\n");
		mem_deref(sampv);
		mem_deref(source_samples);
		return NULL;
	}

	while (st->run) {
		unsigned int in_length;
		unsigned int out_length;
		int i;
	#if 0
		Sleep(4);	// too imprecise under Windows on low-end PC (Atom N270, Windows Embedded, 10ms frame time, 3 calls at once)
	#else
		/* Set timer properties */
		li.QuadPart = -20000;	/* negative value = relative time; unit = 100 ns */
		if(!SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE)){
			DEBUG_WARNING("Error setting waitable timer!\n");
			break;
		}
		/* Start & wait for timer */
		WaitForSingleObject(timer, INFINITE);
	#endif

		// WinXP (WES 2009) on Atom N270: read handler called with 16ms interval,
		// sporadically pauses up to few hundreds ms
		// => problem (mainly?) if 10ms codec is used
		// 	=> calling whole routine up to 4 times each time to mitigate this
		for (i=0; i<4; i++) {
			now = tmr_jiffies();

			if (ts > now)
				continue;

			in_length = st->in_samples_per_frame;
			out_length = st->out_samples_per_frame;

			if (st->speex_state) {
				aubuf_read(st->aubuf, (uint8_t*)source_samples, st->in_samples_per_frame*sizeof(int16_t));

				speex_resampler_process_interleaved_int(st->speex_state,
								source_samples, &in_length,
								sampv, &out_length);
			#if 0
				if (in_length != st->in_samples_per_frame) {
					DEBUG_INFO(LOG_PROMPT"in_length = %u, in_samples_per_frame = %d\n", in_length, st->in_samples_per_frame);
				}
			#endif
				assert(in_length == st->in_samples_per_frame);
			#if 0
				if (out_length != st->out_samples_per_frame) {
					DEBUG_INFO(LOG_PROMPT"in_length = %d, in_samples_per_frame = %d, out_length = %d, out_samples_per_frame = %d",
						in_length, st->in_samples_per_frame, out_length, st->out_samples_per_frame);
				}
			#endif
				assert(out_length == st->out_samples_per_frame);
			} else {
				aubuf_read(st->aubuf, (uint8_t*)sampv, st->sampc*sizeof(int16_t));
			}
		#if 0
			{
				char buf[64];
				/* debugging exact timing of read handler calls */
				(void)re_printf("[%s] rh %p\n", sys_time(buf, sizeof(buf)), st);
			}
		#endif
			st->rh((uint8_t *)sampv, st->sampc*sizeof(int16_t), st->arg);

			ts += st->ptime;
		}
	}

	if (timer != NULL)
		CloseHandle(timer);

	if (source_samples) {
		mem_deref(source_samples);
	}
	mem_deref(sampv);

	DEBUG_INFO("aufile: player thread exited\n");

	st->terminated = true;

	return 0;
}


static void timeout(void *arg)
{
	struct ausrc_st *st = arg;

	/* check if audio buffer is empty */
	if (aubuf_cur_size(st->aubuf) < (2 * st->in_samples_per_frame)) {

		DEBUG_INFO(LOG_PROMPT"end of wave file, calling error handler\n");

		/* error handler must be called from re_main thread */
		if (st->errh) {
			st->errh(0, "end of file", st->arg);
			return;	/* not rescheduling timer */
		}
	} else {
    	//DEBUG_INFO(LOG_PROMPT"timer: audio buffer is not empty\n");
	}

	tmr_start(&st->tmr, TIMER_TEST_EOF, timeout, st);
}


static int read_file(struct ausrc_st *st)
{
	struct mbuf *mb;
	int err;

	for (;;) {
		mb = mbuf_alloc(128*1024);
		if (!mb)
			return ENOMEM;

		if (st->fprm.fmt == AUFMT_S16LE) {
			mb->end = mb->size;
		} else if (st->fprm.fmt == AUFMT_PCMA || st->fprm.fmt == AUFMT_PCMU) {
        	mb->end = mb->size / sizeof(short);
		}

		err = aufile_read(st->aufile, mb->buf, &mb->end);
		if (err)
			break;

		if (mb->end == 0) {
			DEBUG_INFO(LOG_PROMPT"end of file\n");
			aubuf_stop_buffering(st->aubuf);
			break;
		}

		if (st->fprm.fmt == AUFMT_PCMA) {
			int i;
			for (i=mb->end - 1; i>=0; i--) {
				short val = g711_alaw2pcm(mb->buf[i]);
				memcpy(&mb->buf[i*2], &val, sizeof(val));
			}
			mb->end *= sizeof(short);
		} else if (st->fprm.fmt == AUFMT_PCMU) {
			int i;
			for (i=mb->end - 1; i>=0; i--) {
				short val = g711_ulaw2pcm(mb->buf[i]);
				memcpy(&mb->buf[i*2], &val, sizeof(val));
			}
			mb->end *= sizeof(short);
		}

		aubuf_append(st->aubuf, mb);

		mb = mem_deref(mb);
	}

	DEBUG_INFO(LOG_PROMPT"loaded %zu bytes\n", aubuf_cur_size(st->aubuf));

	mem_deref(mb);
	return err;
}

static int alloc_handler(struct ausrc_st **stp, struct ausrc *as,
			 struct media_ctx **ctx,
			 struct ausrc_prm *prm, const char *dev,
			 ausrc_read_h *rh, ausrc_error_h *errh, void *arg)
{
	struct ausrc_st *st;
	int err;
	DWORD dwtid;
	(void)ctx;

	if (!stp || !as || !prm || !rh)
		return EINVAL;

	DEBUG_INFO(LOG_PROMPT"loading input file '%s'\n", dev);

	st = mem_zalloc(sizeof(*st), destructor);
	if (!st)
		return ENOMEM;

	st->as   = mem_ref(as);
	st->rh   = rh;
	st->errh = errh;
	st->arg  = arg;

	err = aufile_open(&st->aufile, &st->fprm, dev, AUFILE_READ);
	if (err) {
		DEBUG_WARNING(LOG_PROMPT"failed to open file '%s' (%m)\n", dev, err);
		goto out;
	}

	DEBUG_INFO(LOG_PROMPT"%s: %u Hz, %d channels\n", dev, st->fprm.srate, st->fprm.channels);

	if (st->fprm.channels != prm->ch) {
		DEBUG_WARNING(LOG_PROMPT"input file (%s) must have channels = %d\n", dev, prm->ch);
		err = ENODEV;
		goto out;
	}
	if (st->fprm.fmt != AUFMT_S16LE && st->fprm.fmt != AUFMT_PCMA && st->fprm.fmt != AUFMT_PCMU) {
		DEBUG_WARNING(LOG_PROMPT"input file must have S16LE, G.711a or G.711u format\n");
		err = ENODEV;
		goto out;
	}

	st->in_samples_per_frame = prm->frame_size * st->fprm.srate / prm->srate; //prm->frame_size;
	st->out_samples_per_frame = prm->frame_size; //prm->srate / (fprm.srate / prm->frame_size);

	if (st->fprm.srate != prm->srate) {
		DEBUG_WARNING(LOG_PROMPT"using speex resampler\n");
		st->speex_state = speex_resampler_init(prm->ch, st->fprm.srate, prm->srate, SPEEX_RESAMP_QUALITY, &err);
		if (st->speex_state == NULL || err != RESAMPLER_ERR_SUCCESS) {
			err = ENOMEM;
			goto out;
		}
	}

	st->sampc = prm->frame_size; //prm->srate * prm->ch * prm->ptime / 1000;

	st->ptime = prm->frame_size * 1000 / prm->srate / prm->ch; //prm->ptime;

	DEBUG_INFO(LOG_PROMPT"audio ptime=%u sampc=%zu aubuf=[%u:%u]\n",
	     st->ptime, st->sampc,
	     prm->srate * prm->ch * 2,
	     prm->srate * prm->ch * 40);

	/* 1 - inf seconds of audio */
	err = aubuf_alloc(&st->aubuf, st->fprm.srate * prm->ch * 2, 0);
	if (err)
		goto out;

	err = read_file(st);
	// releasing source file immediately
	st->aufile = mem_deref(st->aufile);
	if (err)
		goto out;

	tmr_start(&st->tmr, TIMER_TEST_EOF, timeout, st);

	st->run = true;
	//err = pthread_create(&st->thread, NULL, play_thread, st);
	st->thread = CreateThread(NULL, 0, play_thread, st, 0, &dwtid);
	if (st->thread == NULL) {
		st->run = false;
		goto out;
	}
	SetThreadPriority(st->thread, THREAD_PRIORITY_HIGHEST);

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
