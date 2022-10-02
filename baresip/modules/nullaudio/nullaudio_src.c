/**
 * @file
 * Null audio driver - input device generating just silence.
 * Useful for PCs without audio input device (autosensing jack, no microphone connected).
 * or with no access to input device (apparently windows service since Vista).
 */
#include <re.h>
#include <rem.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <baresip.h>


#define DEBUG_MODULE "nullaudio"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


struct ausrc_st {
	struct ausrc *as;  /* base class / inheritance */
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


static void ausrc_destructor(void *arg)
{
	struct ausrc_st *st = arg;

	if (st->run) {
		st->run = false;
		while (!st->terminated) {
			Sleep(10);
		}
		Sleep(10);
		CloseHandle(st->thread);
	}

	mem_deref(st->as);
}

static DWORD WINAPI rec_thread(LPVOID arg)
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

		// returning zero samples
		memset(sampv, 0, st->sampc * 2);
		st->rh((uint8_t *)sampv, st->sampc*sizeof(int16_t), st->arg);

		ts += st->ptime;
	}

	mem_deref(sampv);

	DEBUG_INFO("nullaudio: player thread exited\n");

	st->terminated = true;

	return 0;
}

int nullaudio_src_alloc(struct ausrc_st **stp, struct ausrc *as,
		      struct media_ctx **ctx,
		      struct ausrc_prm *prm, const char *device,
		      ausrc_read_h *rh, ausrc_error_h *errh, void *arg)
{
	struct ausrc_st *st;
	DWORD dwtid;	
	int err = 0;

	(void)ctx;
	(void)device;
	(void)errh;

	if (!stp || !as || !prm)
		return EINVAL;

	if (prm->srate == 0 || prm->ch == 0 || prm->frame_size == 0) {
		DEBUG_INFO("nullaudio src: invalid/unitialized audio prm\n");
		return EINVAL;
	}

	st = mem_zalloc(sizeof(*st), ausrc_destructor);
	if (!st)
		return ENOMEM;

	st->as  = mem_ref(as);
	st->rh  = rh;
	st->arg = arg;

	prm->fmt = AUFMT_S16LE;

	st->sampc = prm->frame_size; //prm->srate * prm->ch * prm->ptime / 1000;

	st->ptime = prm->frame_size * 1000 / prm->srate / prm->ch; //prm->ptime;

	DEBUG_INFO("nullaudio src: audio ptime=%u sampc=%zu\n", st->ptime, st->sampc);

	st->run = true;
	st->thread = CreateThread(NULL, 0, rec_thread, st, 0, &dwtid);
	if (st->thread == NULL) {
		st->run = false;
		err = ENOMEM;
	}

	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}
