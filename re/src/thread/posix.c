/**
 * @file posix.c  Pthread C11 thread implementation
 *
 * Copyright (C) 2022 Sebastian Reimers
 */
#define _GNU_SOURCE 1

#include <re_types.h>
#include <re_mem.h>
#include <re_thread.h>


struct thread {
	thrd_start_t func;
	void *arg;
};


static void *handler(void *p)
{
	struct thread th = *(struct thread *)p;

	mem_deref(p);

	return (void *)(intptr_t)th.func(th.arg);
}


int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	struct thread *th;
	int err;

	if (!thr || !func)
		return thrd_error;

	th = mem_alloc(sizeof(struct thread), NULL);
	if (!th)
		return thrd_nomem;

	th->func = func;
	th->arg	 = arg;

	err = pthread_create(thr, NULL, handler, th);
	if (err) {
		mem_deref(th);
		return thrd_error;
	}

	return thrd_success;
}


int thrd_equal(thrd_t lhs, thrd_t rhs)
{
	return pthread_equal(lhs, rhs);
}


thrd_t thrd_current(void)
{
	return pthread_self();
}


int thrd_detach(thrd_t thr)
{
	return (pthread_detach(thr) == 0) ? thrd_success : thrd_error;
}


int thrd_join(thrd_t thr, int *res)
{
	void *code;
	int err;

	err = pthread_join(thr, &code);

	if (res)
		*res = (int)(intptr_t)code;

	return err;
}


void call_once(once_flag *flag, void (*func)(void))
{
	pthread_once(flag, func);
}


void thrd_exit(int res)
{
	pthread_exit((void *)(intptr_t)res);
}


int cnd_init(cnd_t *cnd)
{
	if (!cnd)
		return thrd_error;

	return (pthread_cond_init(cnd, NULL) == 0) ? thrd_success : thrd_error;
}


int cnd_signal(cnd_t *cnd)
{
	if (!cnd)
		return thrd_error;

	return (pthread_cond_signal(cnd) == 0) ? thrd_success : thrd_error;
}


int cnd_broadcast(cnd_t *cnd)
{
	if (!cnd)
		return thrd_error;

	return (pthread_cond_broadcast(cnd) == 0) ? thrd_success : thrd_error;
}


int cnd_wait(cnd_t *cnd, mtx_t *mtx)
{
	if (!cnd || !mtx)
		return thrd_error;

	return (pthread_cond_wait(cnd, mtx) == 0) ? thrd_success : thrd_error;
}


void cnd_destroy(cnd_t *cnd)
{
	if (!cnd)
		return;

	pthread_cond_destroy(cnd);
}


int mtx_init(mtx_t *mtx, int type)
{
	pthread_mutexattr_t attr;

	if (!mtx)
		return thrd_error;

	if ((type & mtx_recursive) == 0) {
		pthread_mutex_init(mtx, NULL);
		return thrd_success;
	}

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(mtx, &attr);
	pthread_mutexattr_destroy(&attr);

	return thrd_success;
}


int mtx_lock(mtx_t *mtx)
{
	if (!mtx)
		return thrd_error;

	return (pthread_mutex_lock(mtx) == 0) ? thrd_success : thrd_error;
}


int mtx_trylock(mtx_t *mtx)
{
	if (!mtx)
		return thrd_error;

	return (pthread_mutex_trylock(mtx) == 0) ? thrd_success : thrd_busy;
}


int mtx_unlock(mtx_t *mtx)
{
	if (!mtx)
		return thrd_error;

	return (pthread_mutex_unlock(mtx) == 0) ? thrd_success : thrd_error;
}


void mtx_destroy(mtx_t *mtx)
{
	if (!mtx)
		return;

	pthread_mutex_destroy(mtx);
}


int tss_create(tss_t *key, tss_dtor_t destructor)
{
	if (!key)
		return thrd_error;

	return (pthread_key_create(key, destructor) == 0) ? thrd_success
							  : thrd_error;
}


void *tss_get(tss_t key)
{
	return pthread_getspecific(key);
}


int tss_set(tss_t key, void *val)
{
	return (pthread_setspecific(key, val) == 0) ? thrd_success
						    : thrd_error;
}


void tss_delete(tss_t key)
{
	pthread_key_delete(key);
}
