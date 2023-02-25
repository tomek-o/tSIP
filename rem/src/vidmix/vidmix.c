/**
 * @file vidmix.c Video Mixer
 *
 * Copyright (C) 2010 Creytiv.com
 */

#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#define __USE_UNIX98 1
#include <string.h>
#include <re.h>
#include <rem_vid.h>
#include <rem_vidconv.h>
#include <rem_vidmix.h>


struct vidmix {
	mtx_t rwlock;
	struct list srcl;
	bool initialized;
};

struct vidmix_source {
	struct le le;
	thrd_t thread;
	mtx_t mutex;
	struct vidframe *frame_tx;
	struct vidframe *frame_rx;
	struct vidmix *mix;
	vidmix_frame_h *fh;
	void *arg;
	void *focus;
	bool content_hide;
	bool focus_full;
	unsigned fint;
	bool selfview;
	bool content;
	bool clear;
	bool run;
};


static inline void source_mix_full(struct vidframe *mframe,
				   const struct vidframe *frame_src);


static inline void clear_frame(struct vidframe *vf)
{
	vidframe_fill(vf, 0, 0, 0);
}


static void clear_all(struct vidmix *mix)
{
	struct le *le;

	for (le=mix->srcl.head; le; le=le->next) {

		struct vidmix_source *src = le->data;

		src->clear = true;
	}
}


static void destructor(void *arg)
{
	struct vidmix *mix = arg;

	if (mix->initialized)
		mtx_destroy(&mix->rwlock);
}


static void source_destructor(void *arg)
{
	struct vidmix_source *src = arg;

	vidmix_source_stop(src);

	if (src->le.list) {
		mtx_lock(&src->mix->rwlock);
		list_unlink(&src->le);
		clear_all(src->mix);
		mtx_unlock(&src->mix->rwlock);
	}

	mem_deref(src->frame_tx);
	mem_deref(src->frame_rx);
	mem_deref(src->mix);
}


static inline void source_mix(struct vidframe *mframe,
			      const struct vidframe *frame_src,
			      unsigned n, unsigned rows, unsigned idx,
			      bool focus, bool focus_this, bool focus_full)
{
	struct vidrect rect;

	if (!frame_src)
		return;

	if (focus) {

		const unsigned nmin = focus_full ? 12 : 6;

		n = max((n+1), nmin)/2;

		if (focus_this) {
			rect.w = mframe->size.w * (n-1) / n;
			rect.h = mframe->size.h * (n-1) / n;
			rect.x = 0;
			rect.y = 0;
		}
		else {
			rect.w = mframe->size.w / n;
			rect.h = mframe->size.h / n;

			if (idx < n) {
				rect.x = mframe->size.w - rect.w;
				rect.y = rect.h * idx;
			}
			else if (idx < (n*2 - 1)) {
				rect.x = rect.w * (n*2 - 2 - idx);
				rect.y = mframe->size.h - rect.h;
			}
			else {
				return;
			}
		}
	}
	else if (rows == 1) {

		source_mix_full(mframe, frame_src);
		return;
	}
	else {
		rect.w = mframe->size.w / rows;
		rect.h = mframe->size.h / rows;
		rect.x = rect.w * (idx % rows);
		rect.y = rect.h * (idx / rows);
	}

	vidconv_aspect(mframe, frame_src, &rect);
}


static inline void source_mix_full(struct vidframe *mframe,
				   const struct vidframe *frame_src)
{
	if (!frame_src)
		return;

	if (vidsz_cmp(&mframe->size, &frame_src->size)) {

		vidframe_copy(mframe, frame_src);
	}
	else {
		struct vidrect rect;

		rect.w = mframe->size.w;
		rect.h = mframe->size.h;
		rect.x = 0;
		rect.y = 0;

		vidconv_aspect(mframe, frame_src, &rect);
	}
}


static inline unsigned calc_rows(unsigned n)
{
	unsigned rows;

	for (rows=1;; rows++)
		if (n <= (rows * rows))
			return rows;
}


static int vidmix_thread(void *arg)
{
	struct vidmix_source *src = arg;
	struct vidmix *mix = src->mix;
	uint64_t ts = tmr_jiffies();

	mtx_lock(&src->mutex);

	while (src->run) {

		unsigned n, rows, idx;
		struct le *le;
		uint64_t now;

		mtx_unlock(&src->mutex);
		sys_usleep(4000);
		mtx_lock(&src->mutex);

		now = tmr_jiffies();

		if (ts > now)
			continue;

		if (!src->frame_tx) {
			ts += src->fint;
			continue;
		}

		mtx_lock(&mix->rwlock);

		if (src->clear) {
			clear_frame(src->frame_tx);
			src->clear = false;
		}

		for (le=mix->srcl.head, n=0; le; le=le->next) {

			const struct vidmix_source *lsrc = le->data;

			if (lsrc == src && !src->selfview)
				continue;

			if (lsrc->content && src->content_hide)
				continue;

			if (lsrc == src->focus && src->focus_full)
				source_mix_full(src->frame_tx, lsrc->frame_rx);

			++n;
		}

		rows = calc_rows(n);

		for (le=mix->srcl.head, idx=0; le; le=le->next) {

			const struct vidmix_source *lsrc = le->data;

			if (lsrc == src && !src->selfview)
				continue;

			if (lsrc->content && src->content_hide)
				continue;

			if (lsrc == src->focus && src->focus_full)
				continue;

			source_mix(src->frame_tx, lsrc->frame_rx, n, rows, idx,
				   src->focus != NULL, src->focus == lsrc,
				   src->focus_full);

			if (src->focus != lsrc)
				++idx;
		}

		mtx_unlock(&mix->rwlock);

		src->fh((uint32_t)ts * 90, src->frame_tx, src->arg);

		ts += src->fint;
	}

	mtx_unlock(&src->mutex);

	return 0;
}


static int content_thread(void *arg)
{
	struct vidmix_source *src = arg;
	struct vidmix *mix = src->mix;
	uint64_t ts = tmr_jiffies();

	mtx_lock(&src->mutex);

	while (src->run) {

		struct le *le;
		uint64_t now;

		mtx_unlock(&src->mutex);
		sys_usleep(4000);
		mtx_lock(&src->mutex);

		now = tmr_jiffies();

		if (ts > now)
			continue;

		mtx_lock(&mix->rwlock);

		for (le=mix->srcl.head; le; le=le->next) {

			const struct vidmix_source *lsrc = le->data;

			if (!lsrc->content || !lsrc->frame_rx || lsrc == src)
				continue;

			src->fh((uint32_t)ts * 90, lsrc->frame_rx, src->arg);
			break;
		}

		mtx_unlock(&mix->rwlock);

		ts += src->fint;
	}

	mtx_unlock(&src->mutex);

	return 0;
}


/**
 * Allocate a new Video mixer
 *
 * @param mixp Pointer to allocated video mixer
 *
 * @return 0 for success, otherwise error code
 */
int vidmix_alloc(struct vidmix **mixp)
{
	struct vidmix *mix;
	int err;

	if (!mixp)
		return EINVAL;

	mix = mem_zalloc(sizeof(*mix), destructor);
	if (!mix)
		return ENOMEM;

	err = mtx_init(&mix->rwlock, mtx_plain) != thrd_success;
	if (err) {
		err = ENOMEM;
		goto out;
	}

	mix->initialized = true;

 out:
	if (err)
		mem_deref(mix);
	else
		*mixp = mix;

	return err;
}


/**
 * Allocate a video mixer source
 *
 * @param srcp    Pointer to allocated video source
 * @param mix     Video mixer
 * @param sz      Size of output video frame (optional)
 * @param fps     Output frame rate (frames per second)
 * @param content True if source is of type content
 * @param fh      Mixer frame handler
 * @param arg     Handler argument
 *
 * @return 0 for success, otherwise error code
 */
int vidmix_source_alloc(struct vidmix_source **srcp, struct vidmix *mix,
			const struct vidsz *sz, unsigned fps, bool content,
			vidmix_frame_h *fh, void *arg)
{
	struct vidmix_source *src;
	int err;

	if (!srcp || !mix || !fps || !fh)
		return EINVAL;

	src = mem_zalloc(sizeof(*src), source_destructor);
	if (!src)
		return ENOMEM;

	src->mix     = mem_ref(mix);
	src->fint    = 1000/fps;
	src->content = content;
	src->fh      = fh;
	src->arg     = arg;

	err = mtx_init(&src->mutex, mtx_plain) != thrd_success;
	if (err) {
		err = ENOMEM;
		goto out;
	}

	if (sz) {
		err = vidframe_alloc(&src->frame_tx, VID_FMT_YUV420P, sz);
		if (err)
			goto out;

		clear_frame(src->frame_tx);
	}

 out:
	if (err)
		mem_deref(src);
	else
		*srcp = src;

	return err;
}


/**
 * Check if vidmix source is enabled
 *
 * @param src Video mixer source
 *
 * @return true if enabled, otherwise false
 */
bool vidmix_source_isenabled(const struct vidmix_source *src)
{
	return src ? (src->le.list != NULL) : false;
}


/**
 * Check if vidmix source is running
 *
 * @param src Video mixer source
 *
 * @return true if running, otherwise false
 */
bool vidmix_source_isrunning(const struct vidmix_source *src)
{
	return src ? src->run : false;
}


/**
 * Get focus source
 *
 * @param src Video mixer source
 *
 * @return pointer of focused source or NULL if focus is not set
 */
void *vidmix_source_get_focus(const struct vidmix_source *src)
{
	return src ? src->focus : NULL;
}


/**
 * Enable/disable vidmix source
 *
 * @param src    Video mixer source
 * @param enable True to enable, false to disable
 */
void vidmix_source_enable(struct vidmix_source *src, bool enable)
{
	if (!src)
		return;

	if (src->le.list && enable)
		return;

	if (!src->le.list && !enable)
		return;

	mtx_lock(&src->mix->rwlock);

	if (enable) {
		if (src->frame_rx)
			clear_frame(src->frame_rx);

		list_append(&src->mix->srcl, &src->le, src);
	}
	else {
		list_unlink(&src->le);
	}

	clear_all(src->mix);

	mtx_unlock(&src->mix->rwlock);
}


/**
 * Start vidmix source thread
 *
 * @param src    Video mixer source
 *
 * @return 0 for success, otherwise error code
 */
int vidmix_source_start(struct vidmix_source *src)
{
	int err;

	if (!src)
		return EINVAL;

	if (src->run)
		return EALREADY;

	src->run = true;

	err = thread_create_name(&src->thread, "vidmix",
				 src->content ? content_thread : vidmix_thread,
				 src);
	if (err)
		src->run = false;

	return err;
}


/**
 * Stop vidmix source thread
 *
 * @param src    Video mixer source
 */
void vidmix_source_stop(struct vidmix_source *src)
{
	if (!src)
		return;

	if (src->run) {
		mtx_lock(&src->mutex);
		src->run = false;
		mtx_unlock(&src->mutex);
		thrd_join(src->thread, NULL);
	}
}


/**
 * Set video mixer output frame size
 *
 * @param src  Video mixer source
 * @param sz   Size of output video frame
 *
 * @return 0 for success, otherwise error code
 */
int vidmix_source_set_size(struct vidmix_source *src, const struct vidsz *sz)
{
	struct vidframe *frame;
	int err;

	if (!src || !sz)
		return EINVAL;

	if (src->frame_tx && vidsz_cmp(&src->frame_tx->size, sz))
		return 0;

	err = vidframe_alloc(&frame, VID_FMT_YUV420P, sz);
	if (err)
		return err;

	clear_frame(frame);

	mtx_lock(&src->mutex);
	mem_deref(src->frame_tx);
	src->frame_tx = frame;
	mtx_unlock(&src->mutex);

	return 0;
}


/**
 * Set video mixer output frame rate
 *
 * @param src  Video mixer source
 * @param fps  Output frame rate (frames per second)
 */
void vidmix_source_set_rate(struct vidmix_source *src, unsigned fps)
{
	if (!src || !fps)
		return;

	mtx_lock(&src->mutex);
	src->fint = 1000/fps;
	mtx_unlock(&src->mutex);
}


/**
 * Set video mixer content hide
 *
 * @param src    Video mixer source
 * @param hide   True to hide content, false to show
 */
void vidmix_source_set_content_hide(struct vidmix_source *src, bool hide)
{
	if (!src)
		return;

	mtx_lock(&src->mutex);
	src->content_hide = hide;
	src->clear = true;
	mtx_unlock(&src->mutex);
}


/**
 * Toggle vidmix source selfview
 *
 * @param src    Video mixer source
 */
void vidmix_source_toggle_selfview(struct vidmix_source *src)
{
	if (!src)
		return;

	mtx_lock(&src->mutex);
	src->selfview = !src->selfview;
	src->clear = true;
	mtx_unlock(&src->mutex);
}


/**
 * Set focus on selected participant source
 *
 * @param src        Video mixer source
 * @param focus_src  Video mixer source to focus, NULL to clear focus state
 * @param focus_full Full focus
 */
void vidmix_source_set_focus(struct vidmix_source *src,
			     const struct vidmix_source *focus_src,
			     bool focus_full)
{
	if (!src)
		return;

	mtx_lock(&src->mutex);
	src->focus_full = focus_full;
	src->focus = (void *)focus_src;
	src->clear = true;
	mtx_unlock(&src->mutex);
}


/**
 * Set focus on selected participant
 *
 * @param src    Video mixer source
 * @param pidx   Participant to focus, 0 to disable
 */
void vidmix_source_set_focus_idx(struct vidmix_source *src, unsigned pidx)
{
	bool focus_full = false;
	void *focus = NULL;

	if (!src)
		return;

	if (pidx > 0) {

		struct le *le;
		unsigned i;

		mtx_lock(&src->mix->rwlock);

		for (le=src->mix->srcl.head, i=1; le; le=le->next) {

			const struct vidmix_source *lsrc = le->data;

			if (lsrc == src && !src->selfview)
				continue;

			if (lsrc->content && src->content_hide)
				continue;

			if (i++ == pidx) {
				focus = (void *)lsrc;
				break;
			}
		}

		mtx_unlock(&src->mix->rwlock);
	}

	if (focus && focus == src->focus)
		focus_full = !src->focus_full;

	mtx_lock(&src->mutex);
	src->focus_full = focus_full;
	src->focus = focus;
	src->clear = true;
	mtx_unlock(&src->mutex);
}


/**
 * Put a video frame into the video mixer
 *
 * @param src   Video source
 * @param frame Video frame
 */
void vidmix_source_put(struct vidmix_source *src, const struct vidframe *frame)
{
	if (!src || !frame || frame->fmt != VID_FMT_YUV420P)
		return;

	if (!src->frame_rx || !vidsz_cmp(&src->frame_rx->size, &frame->size)) {

		struct vidframe *frm;
		int err;

		err = vidframe_alloc(&frm, VID_FMT_YUV420P, &frame->size);
		if (err)
			return;

		mtx_lock(&src->mix->rwlock);

		mem_deref(src->frame_rx);
		src->frame_rx = frm;

		clear_all(src->mix);

		mtx_unlock(&src->mix->rwlock);
	}

	mtx_lock(&src->mix->rwlock);
	vidframe_copy(src->frame_rx, frame);
	mtx_unlock(&src->mix->rwlock);
}
