/**
 * @file posix.c  WIN32 thread implementation
 *
 * Copyright (C) 2022 Sebastian Reimers
 */

#include <process.h>
#include <re_types.h>
#include <re_mem.h>
#include <re_thread.h>


#define DEBUG_MODULE "thread"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

#define TSS_DESTRUCTOR_MAX 64

static struct tss_dtor_entry {
	tss_t key;
	tss_dtor_t dtor;
} tss_dtor_tbl[TSS_DESTRUCTOR_MAX];


struct thread {
	thrd_start_t func;
	void *arg;
};

static int dtor_register(tss_t key, tss_dtor_t dtor)
{
	int i;

	for (i = 0; i < TSS_DESTRUCTOR_MAX; i++) {
		if (!tss_dtor_tbl[i].dtor)
			break;
	}

	if (i >= TSS_DESTRUCTOR_MAX) {
		DEBUG_WARNING("thread: max tss destructors reached\n");
		return ENOMEM;
	}

	tss_dtor_tbl[i].key  = key;
	tss_dtor_tbl[i].dtor = dtor;

	return 0;
}


static void tss_dtor_destruct(void)
{
	void *val;
	int i;

	for (i = 0; i < TSS_DESTRUCTOR_MAX; i++) {
		if (!tss_dtor_tbl[i].dtor)
			continue;
		val = tss_get(tss_dtor_tbl[i].key);
		if (val) {
			tss_dtor_tbl[i].dtor(val);
			tss_dtor_tbl[i].dtor = NULL;
		}
	}
}


static unsigned __stdcall thrd_handler(void *p)
{
	struct thread th = *(struct thread *)p;
	int err;

	mem_deref(p);
	err = th.func(th.arg);

	tss_dtor_destruct();

	return err;
}


int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	struct thread *th;
	int err = thrd_success;
	unsigned long handle;

	if (!thr || !func)
		return thrd_error;

	th = mem_alloc(sizeof(struct thread), NULL);
	if (!th)
		return thrd_nomem;

	th->func = func;
	th->arg	 = arg;

	handle = _beginthreadex(NULL, 0, thrd_handler, th, 0, NULL);
	if (handle == 0) {
		if (errno == EAGAIN || errno == EACCES) {
			err = thrd_nomem;
			goto out;
		}
		err = thrd_error;
		goto out;
	}

	*thr = (thrd_t)handle;
out:
	if (err)
		mem_deref(th);

	return err;
}


int thrd_equal(thrd_t lhs, thrd_t rhs)
{
	return GetThreadId(lhs) == GetThreadId(rhs);
}


thrd_t thrd_current(void)
{
	return GetCurrentThread();
}


int thrd_detach(thrd_t thr)
{
	CloseHandle(thr);
	return thrd_success;
}


int thrd_join(thrd_t thr, int *res)
{
	DWORD w, code;

	w = WaitForSingleObject(thr, INFINITE);
	if (w != WAIT_OBJECT_0)
		return thrd_error;

	if (res) {
		if (!GetExitCodeThread(thr, &code)) {
			CloseHandle(thr);
			return thrd_error;
		}
		*res = (int)code;
	}

	CloseHandle(thr);
	return thrd_success;
}


void thrd_exit(int res)
{
	_endthreadex((unsigned)res);
}


int mtx_init(mtx_t *mtx, int type)
{
	(void)type;

	if (!mtx)
		return thrd_error;

	InitializeCriticalSection(mtx);

	return thrd_success;
}


int mtx_lock(mtx_t *mtx)
{
	if (!mtx)
		return thrd_error;

	EnterCriticalSection(mtx);

	return thrd_success;
}


int mtx_trylock(mtx_t *mtx)
{
	if (!mtx)
		return thrd_error;

	return TryEnterCriticalSection(mtx) ? thrd_success : thrd_busy;
}


int mtx_unlock(mtx_t *mtx)
{
	if (!mtx)
		return thrd_error;

	LeaveCriticalSection(mtx);

	return thrd_success;
}


void mtx_destroy(mtx_t *mtx)
{
	if (!mtx)
		return;

	DeleteCriticalSection(mtx);
}


int tss_create(tss_t *key, tss_dtor_t destructor)
{
	int err;

	if (!key)
		return thrd_error;

	*key = TlsAlloc();
	if (*key == TLS_OUT_OF_INDEXES)
		return thrd_error;

	if (!destructor)
		return thrd_success;

	err = dtor_register(*key, destructor);
	if (err) {
		TlsFree(*key);
		return thrd_error;
	}

	return thrd_success;
}


void *tss_get(tss_t key)
{
	return TlsGetValue(key);
}


int tss_set(tss_t key, void *val)
{
	return TlsSetValue(key, val) ? thrd_success : thrd_error;
}


void tss_delete(tss_t key)
{
	TlsFree(key);
}
