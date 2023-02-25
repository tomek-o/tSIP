/**
 * @file sdl/sdl.c  Simple DirectMedia Layer module for SDL v2.0
 *
 * Copyright (C) 2010 Alfred E. Heggestad
 */

#define SDL_endian_h_
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <re.h>
#include <rem.h>
#include <baresip.h>

#define DEBUG_MODULE "sdl"
#define DEBUG_LEVEL 5
#include <re_dbg.h>

/**
 * @defgroup sdl sdl
 *
 * Video display using Simple DirectMedia Layer version 2 (SDL2)
 */


struct vidisp_st {
	struct vidisp *vd;              /**< Inheritance (1st)     */
	SDL_Window *window;             /**< SDL Window            */
	SDL_Renderer *renderer;         /**< SDL Renderer          */
	SDL_Texture *texture;           /**< Texture for pixels    */
	struct vidsz size;              /**< Current size          */
	enum vidfmt fmt;                /**< Current pixel format  */
	bool fullscreen;                /**< Fullscreen flag       */
	void *parent_handle;
	struct mqueue *mq;
	Uint32 flags;
	bool quit;
};


static struct vidisp *vid = NULL;


static uint32_t match_fmt(enum vidfmt fmt)
{
	switch (fmt) {

	case VID_FMT_YUV420P:	return SDL_PIXELFORMAT_IYUV;
	case VID_FMT_YUYV422:   return SDL_PIXELFORMAT_YUY2;
	case VID_FMT_UYVY422:   return SDL_PIXELFORMAT_UYVY;
	case VID_FMT_NV12:	return SDL_PIXELFORMAT_NV12;
	case VID_FMT_NV21:	return SDL_PIXELFORMAT_NV21;
	case VID_FMT_RGB32:     return SDL_PIXELFORMAT_ARGB8888;
	default:		return SDL_PIXELFORMAT_UNKNOWN;
	}
}


static uint32_t chroma_step(enum vidfmt fmt)
{
	switch (fmt) {

	case VID_FMT_YUV420P:	return 2;
	case VID_FMT_NV12:	return 1;
	case VID_FMT_NV21:	return 1;
	case VID_FMT_RGB32:     return 0;
	default:		return 0;
	}
}


static void sdl_reset(struct vidisp_st *st)
{
	if (st->texture) {
		SDL_DestroyTexture(st->texture);
		st->texture = NULL;
	}

	if (st->renderer) {
		SDL_DestroyRenderer(st->renderer);
		st->renderer = NULL;
	}

	if (st->window) {
		SDL_DestroyWindow(st->window);
		st->window = NULL;
	}
}


static void poll_events(struct vidisp_st *st)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_KEYDOWN) {

			switch (event.key.keysym.sym) {

			case SDLK_f:
				/* press key 'f' to toggle fullscreen */
				st->fullscreen = !st->fullscreen;
				DEBUG_INFO("sdl: %able fullscreen mode\n",
				     st->fullscreen ? "en" : "dis");

				if (st->fullscreen)
					st->flags |=
						SDL_WINDOW_FULLSCREEN_DESKTOP;
				else
					st->flags &=
						~SDL_WINDOW_FULLSCREEN_DESKTOP;

				SDL_SetWindowFullscreen(st->window, st->flags);
				break;

			case SDLK_q:
				mqueue_push(st->mq, 'q', NULL);
				break;

			default:
				break;
			}
		}
		else if (event.type == SDL_QUIT) {
			st->quit = true;
			break;
		}
	}
}


static void destructor(void *arg)
{
	struct vidisp_st *st = arg;

	mem_deref(st->mq);
	sdl_reset(st);

	mem_deref(st->vd);

	/* needed to close the window */
	SDL_PumpEvents();

	if (st->parent_handle) {
		// get rid of the last image from parent component
		// if SDL display is using SDL_CreateWindowFrom
		InvalidateRect(st->parent_handle, NULL, TRUE);
		UpdateWindow(st->parent_handle);
	}
}


/* called in the context of the main thread */
static void mqueue_handler(int id, void *data, void *arg)
{
	(void)data;
	(void)arg;

	DEBUG_INFO("sdl: mqueue event: id=%d\n", id);

#if 0
	ui_input_key(baresip_uis(), id, NULL);
#else
	{
		int TODO__SDL_UI_INPUT_KEY;
	}
#endif
}


/* NOTE: should be called from the main thread */
static int alloc(struct vidisp_st **stp, const struct vidisp *vd,
		 struct vidisp_prm *prm, const char *dev,
		 vidisp_resize_h *resizeh, void *arg)
{
	struct vidisp_st *st;
	int err;

	/* Not used by SDL */
	(void)dev;
	(void)resizeh;
	(void)arg;

	if (!stp || !vd)
		return EINVAL;

	st = mem_zalloc(sizeof(*st), destructor);
	if (!st)
		return ENOMEM;

	st->vd = mem_ref((void*)vd);
	st->fullscreen = prm ? prm->fullscreen : false;
	st->parent_handle = prm ? prm->parent_handle : NULL;

	err = mqueue_alloc(&st->mq, mqueue_handler, st);
	if (err)
		goto out;

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}


static int display(struct vidisp_st *st, const char *title,
		   const struct vidframe *frame, uint64_t timestamp)
{
	void *pixels;
	uint8_t *d;
	int dpitch, ret;
	unsigned i, h;
	uint32_t format;
	(void)timestamp;

	if (!st || !frame)
		return EINVAL;

	if (st->quit)
		return ENODEV;

	//DEBUG_WARNING("frame %p fmt = %d\n", frame, (int)frame->fmt);
	format = match_fmt(frame->fmt);
	if (format == SDL_PIXELFORMAT_UNKNOWN) {
		DEBUG_WARNING("sdl: pixel format not supported (%d = %s)\n",
			(int)frame->fmt, vidfmt_name(frame->fmt));
		return ENOTSUP;
	}

	if (!vidsz_cmp(&st->size, &frame->size) || frame->fmt != st->fmt) {
		if (st->size.w && st->size.h) {
			DEBUG_INFO("sdl: reset size:"
			     " %s %u x %u ---> %s %u x %u\n",
			     vidfmt_name(st->fmt), st->size.w, st->size.h,
			     vidfmt_name(frame->fmt),
			     frame->size.w, frame->size.h);
		}
		sdl_reset(st);
	}

	if (!st->window) {
		char capt[256];
		int pos;

		st->flags  = 0; //SDL_WINDOW_INPUT_FOCUS;

		if (title) {
			re_snprintf(capt, sizeof(capt), "%s - %u x %u",
				    title, frame->size.w, frame->size.h);
		}
		else {
			re_snprintf(capt, sizeof(capt), "%u x %u",
				    frame->size.w, frame->size.h);
		}

		if (st->parent_handle) {
			st->flags |= SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS;
			pos = 0; //SDL_WINDOWPOS_UNDEFINED;
			st->window = SDL_CreateWindowFrom(st->parent_handle);
		} else {
			st->flags |= SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
			if (st->fullscreen)
				st->flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
			pos = SDL_WINDOWPOS_CENTERED;
			st->window = SDL_CreateWindow(capt,
						  pos, pos,
					      frame->size.w, frame->size.h,
						  st->flags);
		}

		if (!st->window) {
			DEBUG_WARNING("sdl: unable to create sdl window: %s\n",
				SDL_GetError());
			return ENODEV;
		}

		if (st->parent_handle) {
		#if 0
			HWND hwnd;
			SDL_SysWMinfo wmInfo;
			SDL_VERSION(&wmInfo.version);
			SDL_GetWindowWMInfo(st->window, &wmInfo);
			hwnd = wmInfo.info.win.window;
			::SetParent(hwnd, (HWND)st->parent_handle);
			SDL_ShowWindow(st->window);
		#endif
		} else {
			SDL_SetWindowBordered(st->window, true);
			SDL_RaiseWindow(st->window);
		}

		st->size = frame->size;
		st->fmt = frame->fmt;
	}

	if (!st->renderer) {

		SDL_RendererInfo rend_info;
		Uint32 flags = 0;

		flags |= SDL_RENDERER_ACCELERATED;
		flags |= SDL_RENDERER_PRESENTVSYNC;

		st->renderer = SDL_CreateRenderer(st->window, -1, flags);
		if (!st->renderer) {
			DEBUG_WARNING("sdl: unable to create renderer: %s\n",
				SDL_GetError());
			return ENOMEM;
		}

		if (!SDL_GetRendererInfo(st->renderer, &rend_info)) {
			DEBUG_INFO("sdl: created renderer '%s'\n", rend_info.name);
		}

		SDL_RenderSetLogicalSize(st->renderer,
					 frame->size.w, frame->size.h);
	}

	if (!st->texture) {

		st->texture = SDL_CreateTexture(st->renderer,
						format,
						SDL_TEXTUREACCESS_STREAMING,
						frame->size.w, frame->size.h);
		if (!st->texture) {
			DEBUG_WARNING("sdl: unable to create texture: %s\n",
				SDL_GetError());
			return ENODEV;
		}
	}

	/* NOTE: poll events first */
	poll_events(st);

	ret = SDL_LockTexture(st->texture, NULL, &pixels, &dpitch);
	if (ret != 0) {
		DEBUG_WARNING("sdl: unable to lock texture (ret=%d)\n", ret);
		return ENODEV;
	}

	d = pixels;
	for (i=0; i<3; i++) {

		const uint8_t *s = frame->data[i];
		unsigned sz, dsz, hstep, wstep;

		if (!frame->data[i] || !frame->linesize[i])
			break;

		hstep = i==0 ? 1 : 2;
		wstep = i==0 ? 1 : chroma_step(frame->fmt);

		if (wstep == 0)
			continue;

		dsz = dpitch / wstep;
		sz  = min(frame->linesize[i], dsz);

		for (h = 0; h < frame->size.h; h += hstep) {

			memcpy(d, s, sz);

			s += frame->linesize[i];
			d += dsz;
		}
	}

	SDL_UnlockTexture(st->texture);

	/* Clear screen (avoid artifacts) */
	SDL_RenderClear(st->renderer);

	/* Blit the sprite onto the screen */
	SDL_RenderCopy(st->renderer, st->texture, NULL, NULL);

	/* Update the screen! */
	SDL_RenderPresent(st->renderer);

	return 0;
}


static void hide(struct vidisp_st *st)
{
	if (!st || !st->window)
		return;

	SDL_HideWindow(st->window);
}


static int module_init(void)
{
	int err;

	if (SDL_Init(0) != 0) {
		DEBUG_WARNING("sdl: unable to init SDL: %s\n", SDL_GetError());
		return ENODEV;
	}

	if (SDL_VideoInit(NULL) != 0) {
		DEBUG_WARNING("sdl: unable to init Video: %s\n", SDL_GetError());
		return ENODEV;
	}

	err = vidisp_register(&vid, /*baresip_vidispl(),*/
			      "sdl", alloc, NULL, display, hide);
	if (err)
		return err;

	return 0;
}


static int module_close(void)
{
	if (vid) {
		vid = mem_deref(vid);
		SDL_VideoQuit();
	}

	SDL_Quit();

	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(sdl) = {
	"sdl",
	"vidisp",
	module_init,
	module_close,
};
