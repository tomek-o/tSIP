/**
 * @file
 * Null audio - playback.
 */
#include <re.h>
#include <rem.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <baresip.h>


#define DEBUG_MODULE "nullaudio"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


struct auplay_st {
	struct auplay *ap;      /* inheritance */
	uint32_t ptime;
	size_t sampc;
	bool run;
	bool terminated;
	HANDLE thread;
	auplay_write_h *wh;
	void *arg;
};


static void auplay_destructor(void *arg)
{
	struct auplay_st *st = arg;

	if (st->run) {
		st->run = false;
		while (!st->terminated) {
			Sleep(10);
		}
		Sleep(10);
		CloseHandle(st->thread);
	}

	mem_deref(st->ap);
}


static DWORD WINAPI play_thread(LPVOID arg)
{
	uint64_t now, ts = tmr_jiffies();
	struct auplay_st *st = arg;
	int16_t *sampv;

	sampv = mem_alloc(st->sampc * 2, NULL);
	if (!sampv)
		return NULL;
	memset(sampv, 0, st->sampc * 2);

	while (st->run) {

		Sleep(4);

		now = tmr_jiffies();

		if (ts > now)
			continue;

		if (st->wh) {
			st->wh((uint8_t*)sampv, st->sampc*sizeof(int16_t), st->arg);
		}

		ts += st->ptime;
	}

	mem_deref(sampv);

	DEBUG_INFO("nullaudio: rec thread exited\n");

	st->terminated = true;

	return 0;
}

int nullaudio_play_alloc(struct auplay_st **stp, struct auplay *ap,
		       struct auplay_prm *prm, const char *device,
		       auplay_write_h *wh, void *arg)
{
	struct auplay_st *st;
	DWORD dwtid;	
	int err = 0;
	(void)device;

	if (!stp || !ap || !prm)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), auplay_destructor);
	if (!st)
		return ENOMEM;

	st->ap  = mem_ref(ap);
	st->wh  = wh;
	st->arg = arg;

	prm->fmt = AUFMT_S16LE;


	st->sampc = prm->frame_size; //prm->srate * prm->ch * prm->ptime / 1000;

	st->ptime = prm->frame_size * 1000 / prm->srate / prm->ch; //prm->ptime;

	DEBUG_INFO("nullaudio play: audio ptime=%u sampc=%zu\n", st->ptime, st->sampc);	


	st->run = true;
	st->thread = CreateThread(NULL, 0, play_thread, st, 0, &dwtid);
	if (st->thread == NULL) {
		st->run = false;
		err = ENOMEM;
	}

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}
