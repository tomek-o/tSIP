/**
 * @file nullvideo.c Video source for black screen and video output discarding data
 *
 * Copyright (C) 2010 Alfred E. Heggestad
 */
#define _DEFAULT_SOURCE 1
#define _BSD_SOURCE 1
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <re.h>
#include <rem.h>
#include <baresip.h>


struct vidsrc_st {
	struct vidframe *frame;
	thrd_t thread;
	mtx_t mutex;
	bool run;
	uint64_t ts;
	double fps;
	vidsrc_frame_h *frameh;
	void *arg;
};

struct vidisp_st {
	struct vidisp *vd;              /**< Inheritance (1st)     */
	int dummy;
};


static struct vidsrc *vidsrc;
static struct vidisp *vidisp;


static void process_frame(struct vidsrc_st *st)
{
	st->ts += (uint64_t)(VIDEO_TIMEBASE / st->fps);

	st->frameh(st->frame, /*st->ts, */ st->arg);
}


static int read_thread(void *arg)
{
	struct vidsrc_st *st = arg;

	st->ts = tmr_jiffies_usec();

	while (1) {
		bool run;

		mtx_lock(&st->mutex);
		run = st->run;
		mtx_unlock(&st->mutex);

		if (!run)
			break;

		if (tmr_jiffies_usec() < st->ts) {
			sys_msleep(4);
			continue;
		}

		process_frame(st);
	}

	return 0;
}


static void src_destructor(void *arg)
{
	struct vidsrc_st *st = arg;
	bool run;

	mtx_lock(&st->mutex);
	run = st->run;
	mtx_unlock(&st->mutex);

	if (run) {
		mtx_lock(&st->mutex);
		st->run = false;
		mtx_unlock(&st->mutex);

		thrd_join(st->thread, NULL);
	}

	mtx_destroy(&st->mutex);

	mem_deref(st->frame);
}


static void disp_destructor(void *arg)
{
	struct vidisp_st *st = arg;
	(void)st;
}


static int src_alloc(struct vidsrc_st **stp, struct vidsrc *vs,
		 struct media_ctx **ctx, struct vidsrc_prm *prm,
		 const struct vidsz *size,
		 const char *fmt, const char *dev,
		 vidsrc_frame_h *frameh,
		 vidsrc_error_h *errorh, void *arg)
{
	struct vidsrc_st *st;
	unsigned x;
	int err;

	(void)fmt;
	(void)dev;
	(void)errorh;
	(void)vs;

	if (!stp || !prm || !size || !frameh)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), src_destructor);
	if (!st)
		return ENOMEM;

	st->fps    = prm->fps;
	st->frameh = frameh;
	st->arg    = arg;

	err = vidframe_alloc(&st->frame, VID_FMT_YUV420P, size);
	if (err)
		goto out;

	/* Black screen */
	for (x=0; x<size->w; x++) {
		uint8_t r=0, g=0, b=0;
		vidframe_draw_vline(st->frame, x, 0, size->h, r, g, b);
	}

	err = mtx_init(&st->mutex, mtx_plain) != thrd_success;
	if (err) {
		err = ENOMEM;
		goto out;
	}

	st->run = true;
	err = thread_create_name(&st->thread, "nullvideo", read_thread, st);
	if (err) {
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


static int disp_alloc(struct vidisp_st **stp, const struct vidisp *vd,
		      struct vidisp_prm *prm, const char *dev,
		      vidisp_resize_h *resizeh, void *arg)
{
	struct vidisp_st *st;
	(void)prm;
	(void)dev;
	(void)resizeh;
	(void)arg;

	if (!stp || !vd)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), disp_destructor);
	if (!st)
		return ENOMEM;

	st->vd = mem_ref((void*)vd);

	*stp = st;

	return 0;
}


static int display(struct vidisp_st *st, const char *title,
		   const struct vidframe *frame, uint64_t timestamp)
{
	(void)st;
	(void)title;
	(void)frame;
	(void)timestamp;

	return 0;
}


static int module_init(void)
{
	int err = 0;
	err |= vidsrc_register(&vidsrc, "nullvideo", src_alloc, NULL);
	err |= vidisp_register(&vidisp, "nullvideo", disp_alloc, NULL, display, NULL);
	return err;
}


static int module_close(void)
{
	vidsrc = mem_deref(vidsrc);
	vidisp = mem_deref(vidisp);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(nullvideo) = {
	"nullvideo",
	"nullvideo",
	module_init,
	module_close
};
