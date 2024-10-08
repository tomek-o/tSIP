/**
 * @file selfview.c  Selfview Video-Filter
 *
 * Copyright (C) 2010 Alfred E. Heggestad
 */
#include <string.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>

#ifdef USE_VIDEO

#define DEBUG_MODULE "selfview"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

/**
 * @defgroup selfview selfview
 *
 * Show a selfview of the captured video stream
 *
 * Example config:
 \verbatim
  video_selfview          pip # {window,pip}
  selfview_size           64x64
 \endverbatim
 */


/* shared state */
struct selfview {
	mtx_t lock;                 /**< Protect frame         */
	struct vidframe *frame;     /**< Copy of encoded frame */
};

struct selfview_enc {
	struct vidfilt_enc_st vf;   /**< Inheritance           */
	struct selfview *selfview;  /**< Ref. to shared state  */
	const struct vidisp *vd;
	struct vidisp_st *disp;     /**< Selfview display      */
};

struct selfview_dec {
	struct vidfilt_dec_st vf;   /**< Inheritance           */
	struct selfview *selfview;  /**< Ref. to shared state  */
};


static struct vidsz selfview_size = {0, 0};


static void destructor(void *arg)
{
	struct selfview *st = arg;

	mtx_lock(&st->lock);
	mem_deref(st->frame);
	mtx_unlock(&st->lock);
	mtx_destroy(&st->lock);
}


static void encode_destructor(void *arg)
{
	struct selfview_enc *st = arg;

	list_unlink(&st->vf.le);
	mem_deref(st->selfview);
	mem_deref(st->disp);
}


static void decode_destructor(void *arg)
{
	struct selfview_dec *st = arg;

	list_unlink(&st->vf.le);
	mem_deref(st->selfview);
}


static int selfview_alloc(struct selfview **selfviewp, void **ctx)
{
	struct selfview *selfview;
	int err;

	if (!selfviewp || !ctx)
		return EINVAL;

	if (*ctx) {
		*selfviewp = mem_ref(*ctx);
	}
	else {
		selfview = mem_zalloc(sizeof(*selfview), destructor);
		if (!selfview)
			return ENOMEM;

		err = mtx_init(&selfview->lock, mtx_plain) != thrd_success;
		if (err)
			return ENOMEM;

		*ctx = selfview;
		*selfviewp = selfview;
	}

	return 0;
}


static int encode_update(struct vidfilt_enc_st **stp, void **ctx,
			 const struct vidfilt *vf)
{
	struct selfview_enc *st;
	int err;

	if (!stp || !ctx || !vf)
		return EINVAL;

	if (*stp)
		return 0;

	st = mem_zalloc(sizeof(*st), encode_destructor);
	if (!st)
		return ENOMEM;

	err = selfview_alloc(&st->selfview, ctx);
	if (err)
		goto out;

	if (0 == str_casecmp(vf->name, "selfview_window")) {

		err = vidisp_alloc(&st->disp, NULL, NULL,
				   NULL, NULL, NULL);
		if (err)
			goto out;

		st->vd = vidisp_find(NULL);
		if (!st->vd) {
			err = ENOENT;
			goto out;
		}

		DEBUG_INFO("selfview: created video display (%s)\n",
		     st->vd->name);
	}

 out:
	if (err)
		mem_deref(st);
	else
		*stp = (struct vidfilt_enc_st *)st;

	return err;
}


static int decode_update(struct vidfilt_dec_st **stp, void **ctx,
			 const struct vidfilt *vf)
{
	struct selfview_dec *st;
	int err;

	if (!stp || !ctx || !vf)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), decode_destructor);
	if (!st)
		return ENOMEM;

	err = selfview_alloc(&st->selfview, ctx);

	if (err)
		mem_deref(st);
	else
		*stp = (struct vidfilt_dec_st *)st;

	return err;
}


static int encode_win(struct vidfilt_enc_st *st, struct vidframe *frame)
{
	struct selfview_enc *enc = (struct selfview_enc *)st;
	int err = 0;

	if (!frame)
		return 0;

	if (!enc->disp) {

		err = vidisp_alloc(&enc->disp, NULL, NULL, NULL, NULL, NULL);
		if (err)
			return err;
	}

	return vidisp_display(enc->disp, "Selfview", frame, 0);
}

/** Capture TX image */
static int encode_pip(struct vidfilt_enc_st *st, struct vidframe *frame)
{
	struct selfview_enc *enc = (struct selfview_enc *)st;
	struct selfview *selfview = enc->selfview;
	int err = 0;

	if (!frame)
		return 0;

	mtx_lock(&selfview->lock);
	if (!selfview->frame) {
		struct vidsz sz;

		/* Use size if configured, or else 20% of main window */
		if (selfview_size.w && selfview_size.h) {
			sz = selfview_size;
		}
		else {
			sz.w = frame->size.w / 5;
			sz.h = frame->size.h / 5;
		}

		err = vidframe_alloc(&selfview->frame, VID_FMT_YUV420P, &sz);
	}
	if (!err)
		vidconv(selfview->frame, frame, NULL);
	mtx_unlock(&selfview->lock);

	return err;
}

/** Draw captured TX image on RX canvas */
static int decode_pip(struct vidfilt_dec_st *st, struct vidframe *frame)
{
	struct selfview_dec *dec = (struct selfview_dec *)st;
	struct selfview *sv = dec->selfview;

	if (!frame)
		return 0;

	mtx_lock(&sv->lock);
	if (sv->frame) {
		struct vidrect rect;

		rect.w = min(sv->frame->size.w, frame->size.w/2);
		rect.h = min(sv->frame->size.h, frame->size.h/2);
		if (rect.w <= (frame->size.w - 10))
			rect.x = frame->size.w - rect.w - 10;
		else
			rect.x = frame->size.w/2;
		if (rect.h <= (frame->size.h - 10))
			rect.y = frame->size.h - rect.h - 10;
		else
			rect.y = frame->size.h/2;

		/* 	This generates artifacts (looking like kind of echo of selfview) on Windows.
			Frame is not a deep copy, *data[4] pointers are supplied by ffmpeg and
			apparently this memory is reused by ffmpeg when video is moving.
		*/
		vidconv(frame, sv->frame, &rect);

		vidframe_draw_rect(frame, rect.x, rect.y, rect.w, rect.h,
				   127, 127, 127);
	}
	mtx_unlock(&sv->lock);

	return 0;
}

static struct vidfilt selfview_win = {
	LE_INIT,
	"selfview_window",
	encode_update,
	encode_win,
	NULL,
	NULL
};

static struct vidfilt selfview_pip = {
	LE_INIT,
	"selfview_pip",
	encode_update,
	encode_pip,
	decode_update,
	decode_pip
};


static int module_init(void)
{
	struct config * cfg = conf_config();
	
	if (cfg->video.selfview.pip) {
		vidfilt_register(&selfview_pip);
	} else {
		vidfilt_register(&selfview_win);
	}

#if 0
	(void)conf_get_vidsz(conf_cur(), "selfview_size", &selfview_size);
#endif
	return 0;
}


static int module_close(void)
{
	vidfilt_unregister(&selfview_win);
	vidfilt_unregister(&selfview_pip);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(selfview) = {
	"selfview",
	"vidfilt",
	module_init,
	module_close
};

#endif /* USE_VIDEO */
