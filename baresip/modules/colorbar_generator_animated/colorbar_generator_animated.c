/**
 * @file colorbar_generator_animated.c Video source: colorbar generator, animated
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

enum { SQUARE_SIZE = 30 };
enum { STEP_X = 5 };
enum { STEP_Y = 10 };

struct vidsrc_st {
	struct vidframe *frame;
	thrd_t thread;
	mtx_t mutex;
	bool run;
	uint64_t ts;
	double fps;
	vidsrc_frame_h *frameh;
	void *arg;
	struct vidsz size;
	unsigned int square_x, square_y;
	bool reverse;
};


static struct vidsrc *vidsrc;


static void process_frame(struct vidsrc_st *st)
{
	unsigned y;
    struct vidsz *size = &st->size;

	st->ts += (VIDEO_TIMEBASE / st->fps);

	/* Pattern of three horizontal bars in RGB */
	for (y=0; y<size->h; y++) {

		uint8_t r=0, g=0, b=0;

		if (y < size->h/3)
			r = 255;
		else if (y < size->h*2/3)
			g = 255;
		else
			b = 255;

		vidframe_draw_hline(st->frame, 0, y, size->w, r, g, b);
	}

	if (size->w >= SQUARE_SIZE && size->h >= SQUARE_SIZE) {
		bool next_line = false;
		vidframe_draw_filled_rect(st->frame, st->square_x, st->square_y, SQUARE_SIZE, SQUARE_SIZE, 0, 0, 0);
		if (st->reverse == false) {
			if (st->square_x + SQUARE_SIZE + STEP_X <= size->w) {
				st->square_x += STEP_X;
			} else {
				next_line = true;
			}
		} else {
			if (st->square_x >= STEP_X) {
				st->square_x -= STEP_X;
			} else {
				next_line = true;
			}
		}
		if (next_line) {
			st->reverse = !st->reverse;
			st->square_y += STEP_Y;
			if (st->square_y + SQUARE_SIZE >= size->h) {
				st->square_y = 0;
			}
		}
	}

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
			sys_usleep(4000);
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
	st->size   = *size;

	err = vidframe_alloc(&st->frame, VID_FMT_YUV420P, size);
	if (err)
		goto out;

	err = mtx_init(&st->mutex, mtx_plain) != thrd_success;
	if (err) {
		err = ENOMEM;
		goto out;
	}

	st->run = true;
	err = thread_create_name(&st->thread, "colorbar_generator", read_thread, st);
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



static int module_init(void)
{
	int err = 0;
	err |= vidsrc_register(&vidsrc, "colorbar_generator_animated", src_alloc, NULL);
	return err;
}


static int module_close(void)
{
	vidsrc = mem_deref(vidsrc);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(colorbar_generator_animated) = {
	"colorbar_generator_animated",
	"colorbar_generator_animated",
	module_init,
	module_close
};
