/**
 * @file draw.c Video Frame primitive drawing routines
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <string.h>
#include <re.h>
#include <rem_vid.h>


/**
 * Draw a pixel to a video frame
 *
 * @param f   Video frame
 * @param x   Pixel X-position
 * @param y   Pixel Y-position
 * @param cy  Color component Y
 * @param cu  Color component U
 * @param cv  Color component V
 */
void vidframe_draw_point(struct vidframe *f, unsigned x, unsigned y,
			 uint8_t cy, uint8_t cu, uint8_t cv)
{
	uint8_t *yp, *up, *vp;

	if (!f || f->fmt != VID_FMT_YUV420P)
		return;

	if (x >= f->size.w || y >= f->size.h)
		return;

	yp = f->data[0] + f->linesize[0] * y     + x;
	up = f->data[1] + f->linesize[1] * (y/2) + x/2;
	vp = f->data[2] + f->linesize[2] * (y/2) + x/2;

	yp[0] = cy;
	up[0] = cu;
	vp[0] = cv;
}


/**
 * Draw a horizontal line
 *
 * @param f   Video frame
 * @param x0  Origin X-position
 * @param y0  Origin Y-position
 * @param w   Line width
 * @param r   Red color component
 * @param g   Green color component
 * @param b   Blue color component
 */
void vidframe_draw_hline(struct vidframe *f,
			 unsigned x0, unsigned y0, unsigned w,
			 uint8_t r, uint8_t g, uint8_t b)
{
	if (!f || f->fmt != VID_FMT_YUV420P)
		return;

	if (x0 >= f->size.w || y0 >= f->size.h)
		return;

	w = min(w, f->size.w-x0);

	memset(f->data[0] +  y0   *f->linesize[0] + x0,   rgb2y(r, g, b), w);
	memset(f->data[1] + (y0/2)*f->linesize[1] + x0/2, rgb2u(r, g, b), w/2);
	memset(f->data[2] + (y0/2)*f->linesize[2] + x0/2, rgb2v(r, g, b), w/2);
}


/**
 * Draw a vertical line
 *
 * @param f   Video frame
 * @param x0  Origin X-position
 * @param y0  Origin Y-position
 * @param h   Line height
 * @param r   Red color component
 * @param g   Green color component
 * @param b   Blue color component
 */
void vidframe_draw_vline(struct vidframe *f,
			 unsigned x0, unsigned y0, unsigned h,
			 uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t cy, cu, cv;

	if (!f || f->fmt != VID_FMT_YUV420P)
		return;

	cy = rgb2y(r, g, b);
	cu = rgb2u(r, g, b);
	cv = rgb2v(r, g, b);

	while (h--) {
		vidframe_draw_point(f, x0, y0++, cy, cu, cv);
	}
}


/**
 * Draw a rectangle
 *
 * @param f   Video frame
 * @param x0  Origin X-position
 * @param y0  Origin Y-position
 * @param w   Rectangle width
 * @param h   Rectangle height
 * @param r   Red color component
 * @param g   Green color component
 * @param b   Blue color component
 */
void vidframe_draw_rect(struct vidframe *f, unsigned x0, unsigned y0,
			unsigned w, unsigned h,
			uint8_t r, uint8_t g, uint8_t b)
{
	if (!f)
		return;

	vidframe_draw_hline(f, x0,     y0,     w, r, g, b);
	vidframe_draw_hline(f, x0,     y0+h-1, w, r, g, b);
	vidframe_draw_vline(f, x0,     y0,     h, r, g, b);
	vidframe_draw_vline(f, x0+w-1, y0,     h, r, g, b);
}
