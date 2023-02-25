/**
 * @file frame.c Video Frame
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <string.h>
#include <re.h>
#include <rem_vid.h>


/**
 * Get video frame buffer size
 *
 * @param fmt Video pixel format
 * @param sz  Size of video frame
 *
 * @return Number of bytes
 */
size_t vidframe_size(enum vidfmt fmt, const struct vidsz *sz)
{
	if (!sz)
		return 0;

	switch (fmt) {

	case VID_FMT_YUV420P: return (size_t)sz->w * sz->h * 3 / 2;
	case VID_FMT_YUYV422: return (size_t)sz->w * sz->h * 2;
	case VID_FMT_UYVY422: return (size_t)sz->w * sz->h * 2;
	case VID_FMT_RGB32:   return (size_t)sz->w * sz->h * 4;
	case VID_FMT_ARGB:    return (size_t)sz->w * sz->h * 4;
	case VID_FMT_RGB565:  return (size_t)sz->w * sz->h * 2;
	case VID_FMT_NV12:    return (size_t)sz->w * sz->h * 3 / 2;
	case VID_FMT_NV21:    return (size_t)sz->w * sz->h * 3 / 2;
	case VID_FMT_YUV444P: return (size_t)sz->w * sz->h * 3;
	case VID_FMT_YUV422P: return (size_t)sz->w * sz->h * 2;
	default:
		return 0;
	}
}


/**
 * Initialize a video frame
 *
 * @param vf       Video frame
 * @param fmt      Video pixel format
 * @param sz       Size of video frame
 * @param data     Pointer to video planes
 * @param linesize Pointer to linesizes
 */
void vidframe_init(struct vidframe *vf, enum vidfmt fmt,
		   const struct vidsz *sz, void *data[4], unsigned linesize[4])
{
	int i;

	if (!vf || !sz || !data || !linesize)
		return;

	for (i=0; i<4; i++) {
		vf->data[i]     = data[i];
		vf->linesize[i] = linesize[i];
	}

	vf->size = *sz;
	vf->fmt = fmt;
}


/**
 * Initialize a video frame from a buffer
 *
 * @param vf  Video frame
 * @param fmt Video pixel format
 * @param sz  Size of video frame
 * @param buf Frame buffer
 */
void vidframe_init_buf(struct vidframe *vf, enum vidfmt fmt,
		       const struct vidsz *sz, uint8_t *buf)
{
	unsigned w, h;
	unsigned w2;	

	if (!vf || !sz || !buf)
		return;

	w = (sz->w + 1) >> 1;
	h = (sz->h + 1) >> 1;

	w2 = (sz->w + 1) >> 1;

	memset(vf->linesize, 0, sizeof(vf->linesize));
	memset(vf->data, 0, sizeof(vf->data));

	switch (fmt) {

	case VID_FMT_YUV420P:
		vf->linesize[0] = sz->w;
		vf->linesize[1] = w;
		vf->linesize[2] = w;

		vf->data[0] = buf;
		vf->data[1] = vf->data[0] + vf->linesize[0] * sz->h;
		vf->data[2] = vf->data[1] + vf->linesize[1] * h;
		break;

	case VID_FMT_YUYV422:
	case VID_FMT_UYVY422:
		vf->linesize[0] = sz->w * 2;
		vf->data[0] = buf;
		break;

	case VID_FMT_RGB32:
	case VID_FMT_ARGB:
		vf->linesize[0] = sz->w * 4;
		vf->data[0] = buf;
		break;

	case VID_FMT_RGB565:
		vf->linesize[0] = sz->w * 2;
		vf->data[0] = buf;
		break;

	case VID_FMT_NV12:
	case VID_FMT_NV21:
		vf->linesize[0] = sz->w;
		vf->linesize[1] = w*2;

		vf->data[0] = buf;
		vf->data[1] = vf->data[0] + vf->linesize[0] * sz->h;
		break;

	case VID_FMT_YUV444P:
		vf->linesize[0] = sz->w;
		vf->linesize[1] = sz->w;
		vf->linesize[2] = sz->w;

		vf->data[0] = buf;
		vf->data[1] = vf->data[0] + vf->linesize[0] * sz->h;
		vf->data[2] = vf->data[1] + vf->linesize[1] * sz->h;
		break;

	case VID_FMT_YUV422P:
		vf->linesize[0] = sz->w;
		vf->linesize[1] = w2;
		vf->linesize[2] = w2;

		vf->data[0] = buf;
		vf->data[1] = vf->data[0] + vf->linesize[0] * sz->h;
		vf->data[2] = vf->data[1] + vf->linesize[1] * sz->h;
		break;

	default:
		(void)re_printf("vidframe: no fmt %s\n", vidfmt_name(fmt));
		return;
	}

	vf->size = *sz;
	vf->fmt = fmt;
}


/**
 * Allocate an empty video frame
 *
 * @param vfp Pointer to allocated video frame
 * @param fmt Video pixel format
 * @param sz  Size of video frame
 *
 * @return 0 for success, otherwise error code
 */
int vidframe_alloc(struct vidframe **vfp, enum vidfmt fmt,
		   const struct vidsz *sz)
{
	struct vidframe *vf;

	if (!sz || !sz->w || !sz->h)
		return EINVAL;

	vf = mem_zalloc(sizeof(*vf) + vidframe_size(fmt, sz), NULL);
	if (!vf)
		return ENOMEM;

	vidframe_init_buf(vf, fmt, sz, (uint8_t *)(vf + 1));

	*vfp = vf;

	//re_printf("vidframe_alloc: %p\n", vf);

	return 0;
}


/**
 * Fill a video frame with a nice color
 *
 * @param vf Video frame
 * @param r  Red color component
 * @param g  Green color component
 * @param b  Blue color component
 */
void vidframe_fill(struct vidframe *vf, uint32_t r, uint32_t g, uint32_t b)
{
	uint8_t *p;
	size_t h;
	unsigned i, x;
	int u, v;

	if (!vf)
		return;

	switch (vf->fmt) {

	case VID_FMT_YUV420P:
		h = vf->size.h;

		memset(vf->data[0], rgb2y(r, g, b), h * vf->linesize[0]);
		memset(vf->data[1], rgb2u(r, g, b), h/2 * vf->linesize[1]);
		memset(vf->data[2], rgb2v(r, g, b), h/2 * vf->linesize[2]);
		break;

	case VID_FMT_YUV444P:
		h = vf->size.h;

		memset(vf->data[0], rgb2y(r, g, b), h * vf->linesize[0]);
		memset(vf->data[1], rgb2u(r, g, b), h * vf->linesize[1]);
		memset(vf->data[2], rgb2v(r, g, b), h * vf->linesize[2]);
		break;

	case VID_FMT_RGB32:
		p = vf->data[0];
		for (i=0; i<vf->linesize[0] * vf->size.h; i+=4) {
			*p++ = b;
			*p++ = g;
			*p++ = r;
			*p++ = 0;
		}
		break;

	case VID_FMT_NV12:
	case VID_FMT_NV21:
		h = vf->size.h;

		if (vf->fmt == VID_FMT_NV12) {
			u = rgb2u(r, g, b);
			v = rgb2v(r, g, b);
		}
		else {
			v = rgb2u(r, g, b);
			u = rgb2v(r, g, b);
		}

		memset(vf->data[0], rgb2y(r, g, b), h * vf->linesize[0]);

		p = vf->data[1];

		for (h=0; h<vf->size.h; h+=2) {

			for (x=0; x<vf->size.w; x+=2) {
				p[x  ] = u;
				p[x+1] = v;
			}

			p += vf->linesize[1];
		}
		break;

	case VID_FMT_YUV422P:
		h = vf->size.h;

		memset(vf->data[0], rgb2y(r, g, b), h * vf->linesize[0]);
		memset(vf->data[1], rgb2u(r, g, b), h * vf->linesize[1]);
		memset(vf->data[2], rgb2v(r, g, b), h * vf->linesize[2]);
		break;

	default:
		(void)re_printf("vidfill: no fmt %s\n", vidfmt_name(vf->fmt));
		break;
	}
}


/**
 * Copy content between to equally sized video frames of same pixel format
 *
 * @param dst Destination frame
 * @param src Source frame
 */
void vidframe_copy(struct vidframe *dst, const struct vidframe *src)
{
	const uint8_t *ds0, *ds1, *ds2;
	unsigned lsd, lss, w, h, y;
	unsigned lsd1, lss1;
	unsigned lsd2, lss2;
	uint8_t *dd0, *dd1, *dd2;

	if (!dst || !src)
		return;

	if (!vidsz_cmp(&dst->size, &src->size))
		return;

	if (dst->fmt != src->fmt)
		return;

	switch (dst->fmt) {

	case VID_FMT_YUV420P:
		lsd = dst->linesize[0];
		lss = src->linesize[0];

		dd0 = dst->data[0];
		dd1 = dst->data[1];
		dd2 = dst->data[2];

		ds0 = src->data[0];
		ds1 = src->data[1];
		ds2 = src->data[2];

		w  = dst->size.w & ~1;
		h  = dst->size.h & ~1;

		for (y=0; y<h; y+=2) {

			memcpy(dd0, ds0, w);
			dd0 += lsd;
			ds0 += lss;

			memcpy(dd0, ds0, w);
			dd0 += lsd;
			ds0 += lss;

			memcpy(dd1, ds1, w/2);
			dd1 += lsd/2;
			ds1 += lss/2;

			memcpy(dd2, ds2, w/2);
			dd2 += lsd/2;
			ds2 += lss/2;
		}
		break;

	case VID_FMT_YUV444P:
		lsd = dst->linesize[0];
		lss = src->linesize[0];

		dd0 = dst->data[0];
		dd1 = dst->data[1];
		dd2 = dst->data[2];

		ds0 = src->data[0];
		ds1 = src->data[1];
		ds2 = src->data[2];

		w  = dst->size.w;
		h  = dst->size.h;

		for (y=0; y<h; y++) {

			/* Y */
			memcpy(dd0, ds0, w);
			dd0 += lsd;
			ds0 += lss;

			/* U */
			memcpy(dd1, ds1, w);
			dd1 += lsd;
			ds1 += lss;

			/* V */
			memcpy(dd2, ds2, w);
			dd2 += lsd;
			ds2 += lss;
		}
		break;

	case VID_FMT_NV12:
	case VID_FMT_NV21:
		lsd = dst->linesize[0];
		lss = src->linesize[0];

		dd0 = dst->data[0];
		dd1 = dst->data[1];

		ds0 = src->data[0];
		ds1 = src->data[1];

		w  = dst->size.w & ~1;
		h  = dst->size.h & ~1;

		for (y=0; y<h; y+=2) {

			memcpy(dd0, ds0, w);
			dd0 += lsd;
			ds0 += lss;

			memcpy(dd0, ds0, w);
			dd0 += lsd;
			ds0 += lss;

			memcpy(dd1, ds1, w);
			dd1 += lsd;
			ds1 += lss;
		}
		break;

	case VID_FMT_YUV422P:
		lsd  = dst->linesize[0];
		lss  = src->linesize[0];
		lsd1 = dst->linesize[1];
		lss1 = src->linesize[1];
		lsd2 = dst->linesize[2];
		lss2 = src->linesize[2];

		dd0 = dst->data[0];
		dd1 = dst->data[1];
		dd2 = dst->data[2];

		ds0 = src->data[0];
		ds1 = src->data[1];
		ds2 = src->data[2];

		w  = dst->size.w & ~1;
		h  = dst->size.h & ~1;

		for (y=0; y<h; y+=1) {

			memcpy(dd0, ds0, w);
			dd0 += lsd;
			ds0 += lss;

			memcpy(dd1, ds1, w/2);
			dd1 += lsd1;
			ds1 += lss1;

			memcpy(dd2, ds2, w/2);
			dd2 += lsd2;
			ds2 += lss2;
		}
		break;

	case VID_FMT_YUYV422:
		lsd = dst->linesize[0];
		lss = src->linesize[0];

		dd0 = dst->data[0];

		ds0 = src->data[0];

		w  = dst->size.w & ~1;
		h  = dst->size.h & ~1;

		for (y=0; y<h; y+=2) {

			memcpy(dd0, ds0, w*2);
			dd0 += lsd;
			ds0 += lss;

			memcpy(dd0, ds0, w*2);
			dd0 += lsd;
			ds0 += lss;
		}
		break;

	default:
		(void)re_printf("vidframe_copy(): unsupported format:"
				" %s\n", vidfmt_name(dst->fmt));
		break;
	}
}
