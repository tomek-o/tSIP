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

	case VID_FMT_YUV420P: return sz->w * sz->h * 3 / 2;
	case VID_FMT_YUYV422: return sz->w * sz->h * 2;
	case VID_FMT_UYVY422: return sz->w * sz->h * 2;
	case VID_FMT_RGB32:   return sz->w * sz->h * 4;
	case VID_FMT_RGB565:  return sz->w * sz->h * 2;
	case VID_FMT_RGB555:  return sz->w * sz->h * 2;
	case VID_FMT_NV12:    return sz->w * sz->h * 3 / 2;
	case VID_FMT_NV21:    return sz->w * sz->h * 3 / 2;
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

	if (!vf || !sz || !buf)
		return;

	w = (sz->w + 1) >> 1;
	h = (sz->h + 1) >> 1;

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
		vf->linesize[0] = sz->w * 4;
		vf->data[0] = buf;
		break;

	case VID_FMT_RGB565:
	case VID_FMT_RGB555:
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
	unsigned h, i;

	if (!vf)
		return;

	switch (vf->fmt) {

	case VID_FMT_YUV420P:
		h = vf->size.h;

		memset(vf->data[0], rgb2y(r, g, b), h * vf->linesize[0]);
		memset(vf->data[1], rgb2u(r, g, b), h/2 * vf->linesize[1]);
		memset(vf->data[2], rgb2v(r, g, b), h/2 * vf->linesize[2]);
		break;

	case VID_FMT_RGB32:
		p = vf->data[0];
		for (i=0; i<vf->linesize[0] * vf->size.h; i+=4) {
			*p++ = r;
			*p++ = g;
			*p++ = b;
			*p++ = 0;
		}
		break;

	default:
		(void)re_printf("vidfill: no fmt %s\n", vidfmt_name(vf->fmt));
		break;
	}
}
