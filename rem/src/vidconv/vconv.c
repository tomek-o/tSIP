/**
 * @file vconv.c Video Conversion
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <string.h>
#include <re.h>
#include <rem_vid.h>
#include <rem_dsp.h>
#include <rem_vidconv.h>


#if 0

/*
 * The lookup tables are generated with the following code:
 */

#define P 14

#define COEF_RV ((int32_t) (1.370705f * (float)(1 << P)))
#define COEF_GU ((int32_t) (-0.337633f * (float)(1 << P)))
#define COEF_GV ((int32_t) (-0.698001f * (float)(1 << P)))
#define COEF_BU ((int32_t) (1.732446f * (float)(1 << P)))

#define ERV(a) (COEF_RV * ((a) - 128))
#define EGU(a) (COEF_GU * ((a) - 128))
#define EGV(a) (COEF_GV * ((a) - 128))
#define EBU(a) (COEF_BU * ((a) - 128))


int16_t CRV[256];
int16_t CGU[256];
int16_t CGV[256];
int16_t CBU[256];


static void init_table(void)
{
	int i;

	for (i = 0; i < 256; ++i) {
		CRV[i] = ERV(i) >> P;
		CGU[i] = EGU(i) >> P;
		CGV[i] = EGV(i) >> P;
		CBU[i] = EBU(i) >> P;
	}
}
#endif


static const int16_t CRV[256] = {
	-176,-175,-173,-172,-170,-169,-168,-166,-165,-164,-162,-161,
	-159,-158,-157,-155,-154,-153,-151,-150,-149,-147,-146,-144,
	-143,-142,-140,-139,-138,-136,-135,-133,-132,-131,-129,-128,
	-127,-125,-124,-122,-121,-120,-118,-117,-116,-114,-113,-112,
	-110,-109,-107,-106,-105,-103,-102,-101, -99, -98, -96, -95,
	 -94, -92, -91, -90, -88, -87, -85, -84, -83, -81, -80, -79,
	 -77, -76, -75, -73, -72, -70, -69, -68, -66, -65, -64, -62,
	 -61, -59, -58, -57, -55, -54, -53, -51, -50, -48, -47, -46,
	 -44, -43, -42, -40, -39, -38, -36, -35, -33, -32, -31, -29,
	 -28, -27, -25, -24, -22, -21, -20, -18, -17, -16, -14, -13,
	 -11, -10,  -9,  -7,  -6,  -5,  -3,  -2,   0,   1,   2,   4,
	   5,   6,   8,   9,  10,  12,  13,  15,  16,  17,  19,  20,
	  21,  23,  24,  26,  27,  28,  30,  31,  32,  34,  35,  37,
	  38,  39,  41,  42,  43,  45,  46,  47,  49,  50,  52,  53,
	  54,  56,  57,  58,  60,  61,  63,  64,  65,  67,  68,  69,
	  71,  72,  74,  75,  76,  78,  79,  80,  82,  83,  84,  86,
	  87,  89,  90,  91,  93,  94,  95,  97,  98, 100, 101, 102,
	 104, 105, 106, 108, 109, 111, 112, 113, 115, 116, 117, 119,
	 120, 121, 123, 124, 126, 127, 128, 130, 131, 132, 134, 135,
	 137, 138, 139, 141, 142, 143, 145, 146, 148, 149, 150, 152,
	 153, 154, 156, 157, 158, 160, 161, 163, 164, 165, 167, 168,
	 169, 171, 172, 174};

static const int16_t CGU[256] = {
	  43,  42,  42,  42,  41,  41,  41,  40,  40,  40,  39,  39,
	  39,  38,  38,  38,  37,  37,  37,  36,  36,  36,  35,  35,
	  35,  34,  34,  34,  33,  33,  33,  32,  32,  32,  31,  31,
	  31,  30,  30,  30,  29,  29,  29,  28,  28,  28,  27,  27,
	  27,  26,  26,  25,  25,  25,  24,  24,  24,  23,  23,  23,
	  22,  22,  22,  21,  21,  21,  20,  20,  20,  19,  19,  19,
	  18,  18,  18,  17,  17,  17,  16,  16,  16,  15,  15,  15,
	  14,  14,  14,  13,  13,  13,  12,  12,  12,  11,  11,  11,
	  10,  10,  10,   9,   9,   9,   8,   8,   8,   7,   7,   7,
	   6,   6,   6,   5,   5,   5,   4,   4,   4,   3,   3,   3,
	   2,   2,   2,   1,   1,   1,   0,   0,   0,  -1,  -1,  -2,
	  -2,  -2,  -3,  -3,  -3,  -4,  -4,  -4,  -5,  -5,  -5,  -6,
	  -6,  -6,  -7,  -7,  -7,  -8,  -8,  -8,  -9,  -9,  -9, -10,
	 -10, -10, -11, -11, -11, -12, -12, -12, -13, -13, -13, -14,
	 -14, -14, -15, -15, -15, -16, -16, -16, -17, -17, -17, -18,
	 -18, -18, -19, -19, -19, -20, -20, -20, -21, -21, -21, -22,
	 -22, -22, -23, -23, -23, -24, -24, -24, -25, -25, -25, -26,
	 -26, -26, -27, -27, -28, -28, -28, -29, -29, -29, -30, -30,
	 -30, -31, -31, -31, -32, -32, -32, -33, -33, -33, -34, -34,
	 -34, -35, -35, -35, -36, -36, -36, -37, -37, -37, -38, -38,
	 -38, -39, -39, -39, -40, -40, -40, -41, -41, -41, -42, -42,
	 -42, -43, -43, -43};

static const int16_t CGV[256] = {
	  89,  88,  87,  87,  86,  85,  85,  84,  83,  83,  82,  81,
	  80,  80,  79,  78,  78,  77,  76,  76,  75,  74,  73,  73,
	  72,  71,  71,  70,  69,  69,  68,  67,  67,  66,  65,  64,
	  64,  63,  62,  62,  61,  60,  60,  59,  58,  57,  57,  56,
	  55,  55,  54,  53,  53,  52,  51,  50,  50,  49,  48,  48,
	  47,  46,  46,  45,  44,  43,  43,  42,  41,  41,  40,  39,
	  39,  38,  37,  36,  36,  35,  34,  34,  33,  32,  32,  31,
	  30,  30,  29,  28,  27,  27,  26,  25,  25,  24,  23,  23,
	  22,  21,  20,  20,  19,  18,  18,  17,  16,  16,  15,  14,
	  13,  13,  12,  11,  11,  10,   9,   9,   8,   7,   6,   6,
	   5,   4,   4,   3,   2,   2,   1,   0,   0,  -1,  -2,  -3,
	  -3,  -4,  -5,  -5,  -6,  -7,  -7,  -8,  -9, -10, -10, -11,
	 -12, -12, -13, -14, -14, -15, -16, -17, -17, -18, -19, -19,
	 -20, -21, -21, -22, -23, -24, -24, -25, -26, -26, -27, -28,
	 -28, -29, -30, -31, -31, -32, -33, -33, -34, -35, -35, -36,
	 -37, -37, -38, -39, -40, -40, -41, -42, -42, -43, -44, -44,
	 -45, -46, -47, -47, -48, -49, -49, -50, -51, -51, -52, -53,
	 -54, -54, -55, -56, -56, -57, -58, -58, -59, -60, -61, -61,
	 -62, -63, -63, -64, -65, -65, -66, -67, -68, -68, -69, -70,
	 -70, -71, -72, -72, -73, -74, -74, -75, -76, -77, -77, -78,
	 -79, -79, -80, -81, -81, -82, -83, -84, -84, -85, -86, -86,
	 -87, -88, -88, -89};

static const int16_t CBU[256] = {
	-222,-221,-219,-217,-215,-214,-212,-210,-208,-207,-205,-203,
	-201,-200,-198,-196,-195,-193,-191,-189,-188,-186,-184,-182,
	-181,-179,-177,-175,-174,-172,-170,-169,-167,-165,-163,-162,
	-160,-158,-156,-155,-153,-151,-149,-148,-146,-144,-143,-141,
	-139,-137,-136,-134,-132,-130,-129,-127,-125,-124,-122,-120,
	-118,-117,-115,-113,-111,-110,-108,-106,-104,-103,-101, -99,
	 -98, -96, -94, -92, -91, -89, -87, -85, -84, -82, -80, -78,
	 -77, -75, -73, -72, -70, -68, -66, -65, -63, -61, -59, -58,
	 -56, -54, -52, -51, -49, -47, -46, -44, -42, -40, -39, -37,
	 -35, -33, -32, -30, -28, -26, -25, -23, -21, -20, -18, -16,
	 -14, -13, -11,  -9,  -7,  -6,  -4,  -2,   0,   1,   3,   5,
	   6,   8,  10,  12,  13,  15,  17,  19,  20,  22,  24,  25,
	  27,  29,  31,  32,  34,  36,  38,  39,  41,  43,  45,  46,
	  48,  50,  51,  53,  55,  57,  58,  60,  62,  64,  65,  67,
	  69,  71,  72,  74,  76,  77,  79,  81,  83,  84,  86,  88,
	  90,  91,  93,  95,  97,  98, 100, 102, 103, 105, 107, 109,
	 110, 112, 114, 116, 117, 119, 121, 123, 124, 126, 128, 129,
	 131, 133, 135, 136, 138, 140, 142, 143, 145, 147, 148, 150,
	 152, 154, 155, 157, 159, 161, 162, 164, 166, 168, 169, 171,
	 173, 174, 176, 178, 180, 181, 183, 185, 187, 188, 190, 192,
	 194, 195, 197, 199, 200, 202, 204, 206, 207, 209, 211, 213,
	 214, 216, 218, 220};


static inline void yuv2rgb(uint8_t *rgb, uint8_t y, int ruv, int guv, int buv)
{
	*rgb++ = saturate_u8(y + buv);
	*rgb++ = saturate_u8(y + guv);
	*rgb++ = saturate_u8(y + ruv);
	*rgb   = 0;
}


static inline void yuv2rgb565(uint8_t *rgb, uint8_t y,
			      int ruv, int guv, int buv)
{
	int r = saturate_u8(y + ruv) >> 3;
	int g = saturate_u8(y + guv) >> 2;
	int b = saturate_u8(y + buv) >> 3;

	rgb[1] = r << 3 | g >> 3;
	rgb[0] = g << 5 | b;
}


static inline void yuv2rgb555(uint8_t *rgb, uint8_t y,
			      int ruv, int guv, int buv)
{
	uint8_t r = saturate_u8(y + ruv) >> 3;
	uint8_t g = saturate_u8(y + guv) >> 3;
	uint8_t b = saturate_u8(y + buv) >> 3;

	rgb[1] = r << 2 | g >> 3;
	rgb[0] = g << 5 | b;
}


typedef void (line_h)(unsigned xoffs, unsigned width, double rw,
		      unsigned yd, unsigned ys, unsigned ys2,
		      uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
		      unsigned lsd,
		      const uint8_t *sd0, const uint8_t *sd1,
		      const uint8_t *sd2, unsigned lss);


static void yuv420p_to_yuv420p(unsigned xoffs, unsigned width, double rw,
			       unsigned yd, unsigned ys, unsigned ys2,
			       uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			       unsigned lsd,
			       const uint8_t *ds0, const uint8_t *ds1,
			       const uint8_t *ds2, unsigned lss
			       )
{
	unsigned x, xd, xs, xs2;
	unsigned id, is;

	for (x=0; x<width; x+=2) {

		xd  = x + xoffs;

		xs  = (unsigned)(x * rw);
		xs2 = (unsigned)((x+1) * rw);

		id = xd + yd*lsd;

		dd0[id]         = ds0[xs  + ys*lss];
		dd0[id+1]       = ds0[xs2 + ys*lss];
		dd0[id + lsd]   = ds0[xs  + ys2*lss];
		dd0[id+1 + lsd] = ds0[xs2 + ys2*lss];

		id = xd/2    + yd*lsd/4;
		is = (xs>>1) + (ys>>1)*lss/2;

		dd1[id] = ds1[is];
		dd2[id] = ds2[is];
	}
}


static void yuyv422_to_yuv420p(unsigned xoffs, unsigned width, double rw,
			       unsigned yd, unsigned ys, unsigned ys2,
			       uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			       unsigned lsd,
			       const uint8_t *sd0, const uint8_t *sd1,
			       const uint8_t *sd2, unsigned lss
			       )
{
	unsigned x, xd, xs;
	unsigned id, is, is2;

	(void)sd1;
	(void)sd2;

	for (x=0; x<width; x+=2) {

		xd  = x + xoffs;

		xs  = ((unsigned)(x * rw * 2)) & ~3;

		id  = xd + yd*lsd;
		is  = xs + ys*lss;
		is2 = xs  + ys2*lss;

		dd0[id]         = sd0[is];
		dd0[id+1]       = sd0[is + 2];
		dd0[id + lsd]   = sd0[is2];
		dd0[id+1 + lsd] = sd0[is2 + 2];

		id = xd/2 + yd*lsd/4;

		dd1[id] = sd0[is + 1];
		dd2[id] = sd0[is + 3];
	}
}


static void uyvy422_to_yuv420p(unsigned xoffs, unsigned width, double rw,
			       unsigned yd, unsigned ys, unsigned ys2,
			       uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			       unsigned lsd,
			       const uint8_t *sd0, const uint8_t *sd1,
			       const uint8_t *sd2, unsigned lss
			       )
{
	unsigned x, xd, xs;
	unsigned id, is, is2;

	(void)sd1;
	(void)sd2;

	for (x=0; x<width; x+=2) {

		xd  = x + xoffs;

		xs  = ((unsigned)(x * rw * 2)) & ~3;

		id  = xd + yd*lsd;
		is  = xs + ys*lss;
		is2 = xs  + ys2*lss;

		dd0[id]         = sd0[is + 1];
		dd0[id+1]       = sd0[is + 3];
		dd0[id + lsd]   = sd0[is2 + 1];
		dd0[id+1 + lsd] = sd0[is2 + 3];

		id = xd/2 + yd*lsd/4;

		dd1[id] = sd0[is + 0];
		dd2[id] = sd0[is + 2];
	}
}


static void rgb32_to_yuv420p(unsigned xoffs, unsigned width, double rw,
			     unsigned yd, unsigned ys, unsigned ys2,
			     uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			     unsigned lsd,
			     const uint8_t *ds0, const uint8_t *ds1,
			     const uint8_t *ds2, unsigned lss
			     )
{
	unsigned x, xd, xs, xs2;
	unsigned id;

	(void)ds1;
	(void)ds2;

	for (x=0; x<width; x+=2) {

		uint32_t x0;
		uint32_t x1;
		uint32_t x2;
		uint32_t x3;

		xd  = x + xoffs;

		xs  = 4 * ((unsigned)( x    * rw));
		xs2 = 4 * ((unsigned)((x+1) * rw));

		id = xd + yd*lsd;

		x0 = *(uint32_t *)(void *)&ds0[xs  + ys*lss];
		x1 = *(uint32_t *)(void *)&ds0[xs2 + ys*lss];
		x2 = *(uint32_t *)(void *)&ds0[xs  + ys2*lss];
		x3 = *(uint32_t *)(void *)&ds0[xs2 + ys2*lss];

		dd0[id]         = rgb2y(x0 >> 16, x0 >> 8, x0);
		dd0[id+1]       = rgb2y(x1 >> 16, x1 >> 8, x1);
		dd0[id + lsd]   = rgb2y(x2 >> 16, x2 >> 8, x2);
		dd0[id+1 + lsd] = rgb2y(x3 >> 16, x3 >> 8, x3);

		id = xd/2 + yd*lsd/4;

		dd1[id] = rgb2u(x0 >> 16, x0 >> 8, x0);
		dd2[id] = rgb2v(x0 >> 16, x0 >> 8, x0);
	}
}


static void yuv420p_to_rgb32(unsigned xoffs, unsigned width, double rw,
			     unsigned yd, unsigned ys, unsigned ys2,
			     uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			     unsigned lsd,
			     const uint8_t *ds0, const uint8_t *ds1,
			     const uint8_t *ds2, unsigned lss)
{
	unsigned x, xd, xs, xs2;
	unsigned id, is;

	(void)dd1;
	(void)dd2;

	for (x=0; x<width; x+=2) {

		int ruv, guv, buv;
		uint8_t u, v;

		xd  = (x + xoffs) * 4;

		xs  = (unsigned)(x * rw);
		xs2 = (unsigned)((x+1) * rw);

		id = (xd + yd*lsd);
		is  = (xs>>1) + (ys>>1)*lss/2;

		u = ds1[is];
		v = ds2[is];

		ruv = CRV[v];
		guv = CGV[v] + CGU[u];
		buv = CBU[u];

		yuv2rgb(&dd0[id],         ds0[xs  + ys*lss],  ruv, guv, buv);
		yuv2rgb(&dd0[id+4],       ds0[xs2 + ys*lss],  ruv, guv, buv);
		yuv2rgb(&dd0[id   + lsd], ds0[xs  + ys2*lss], ruv, guv, buv);
		yuv2rgb(&dd0[id+4 + lsd], ds0[xs2 + ys2*lss], ruv, guv, buv);
	}
}


static void yuv420p_to_rgb565(unsigned xoffs, unsigned width, double rw,
			      unsigned yd, unsigned ys, unsigned ys2,
			      uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			      unsigned lsd,
			      const uint8_t *ds0, const uint8_t *ds1,
			      const uint8_t *ds2, unsigned lss)
{
	unsigned x, xd, xs, xs2;
	unsigned id, is;

	(void)dd1;
	(void)dd2;

	for (x=0; x<width; x+=2) {

		int ruv, guv, buv;
		uint8_t u, v;

		xd  = (x + xoffs) * 2;

		xs  = (unsigned)(x * rw);
		xs2 = (unsigned)((x+1) * rw);

		id = (xd + yd*lsd);
		is  = (xs>>1) + (ys>>1)*lss/2;

		u = ds1[is];
		v = ds2[is];

		ruv = CRV[v];
		guv = CGV[v] + CGU[u];
		buv = CBU[u];

		yuv2rgb565(&dd0[id],         ds0[xs  + ys*lss],  ruv, guv,buv);
		yuv2rgb565(&dd0[id+2],       ds0[xs2 + ys*lss],  ruv, guv,buv);
		yuv2rgb565(&dd0[id   + lsd], ds0[xs  + ys2*lss], ruv, guv,buv);
		yuv2rgb565(&dd0[id+2 + lsd], ds0[xs2 + ys2*lss], ruv, guv,buv);
	}
}


static void yuv420p_to_rgb555(unsigned xoffs, unsigned width, double rw,
			      unsigned yd, unsigned ys, unsigned ys2,
			      uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			      unsigned lsd,
			      const uint8_t *ds0, const uint8_t *ds1,
			      const uint8_t *ds2, unsigned lss)
{
	unsigned x, xd, xs, xs2;
	unsigned id, is;

	(void)dd1;
	(void)dd2;

	for (x=0; x<width; x+=2) {

		int ruv, guv, buv;
		uint8_t u, v;

		xd  = (x + xoffs) * 2;

		xs  = (unsigned)(x * rw);
		xs2 = (unsigned)((x+1) * rw);

		id = (xd + yd*lsd);
		is  = (xs>>1) + (ys>>1)*lss/2;

		u = ds1[is];
		v = ds2[is];

		ruv = CRV[v];
		guv = CGV[v] + CGU[u];
		buv = CBU[u];

		yuv2rgb555(&dd0[id],         ds0[xs  + ys*lss],  ruv, guv,buv);
		yuv2rgb555(&dd0[id+2],       ds0[xs2 + ys*lss],  ruv, guv,buv);
		yuv2rgb555(&dd0[id   + lsd], ds0[xs  + ys2*lss], ruv, guv,buv);
		yuv2rgb555(&dd0[id+2 + lsd], ds0[xs2 + ys2*lss], ruv, guv,buv);
	}
}


static void nv12_to_yuv420p(unsigned xoffs, unsigned width, double rw,
			    unsigned yd, unsigned ys, unsigned ys2,
			    uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			    unsigned lsd,
			    const uint8_t *ds0, const uint8_t *ds1,
			    const uint8_t *ds2, unsigned lss
			    )
{
	unsigned x, xd, xs, xs2;
	unsigned id, is;

	(void)ds2;

	for (x=0; x<width; x+=2) {

		xd  = x + xoffs;

		xs  = (unsigned)(x * rw);
		xs2 = (unsigned)((x+1) * rw);

		id = xd + yd*lsd;

		dd0[id]         = ds0[xs  + ys*lss];
		dd0[id+1]       = ds0[xs2 + ys*lss];
		dd0[id + lsd]   = ds0[xs  + ys2*lss];
		dd0[id+1 + lsd] = ds0[xs2 + ys2*lss];

		id = xd/2    + yd*lsd/4;
		is = ((xs>>1) + (ys>>1)*lss/2) & ~1;

		dd1[id] = ds1[2*is];
		dd2[id] = ds1[2*is+1];
	}
}

static void nv21_to_yuv420p(unsigned xoffs, unsigned width, double rw,
			    unsigned yd, unsigned ys, unsigned ys2,
			    uint8_t *dd0, uint8_t *dd1, uint8_t *dd2,
			    unsigned lsd,
			    const uint8_t *ds0, const uint8_t *ds1,
			    const uint8_t *ds2, unsigned lss
			    )
{
	unsigned x, xd, xs, xs2;
	unsigned id, is;

	(void)ds2;

	for (x=0; x<width; x+=2) {

		xd  = x + xoffs;

		xs  = (unsigned)(x * rw);
		xs2 = (unsigned)((x+1) * rw);

		id = xd + yd*lsd;

		dd0[id]         = ds0[xs  + ys*lss];
		dd0[id+1]       = ds0[xs2 + ys*lss];
		dd0[id + lsd]   = ds0[xs  + ys2*lss];
		dd0[id+1 + lsd] = ds0[xs2 + ys2*lss];

		id = xd/2    + yd*lsd/4;
		is = ((xs>>1) + (ys>>1)*lss/2) & ~1;

		dd2[id] = ds1[2*is];
		dd1[id] = ds1[2*is+1];
	}
}

#define MAX_SRC 9
#define MAX_DST 7

/**
 * Pixel conversion table:  [src][dst]
 *
 * @note Index must be aligned to values in enum vidfmt
 */
static line_h *conv_table[MAX_SRC][MAX_DST] = {

/*
 * Dst:  YUV420P              YUYV422   UYVY422   RGB32
 */
	{yuv420p_to_yuv420p,  NULL,     NULL,     yuv420p_to_rgb32, NULL,
	 yuv420p_to_rgb565, yuv420p_to_rgb555},
	{yuyv422_to_yuv420p,  NULL,     NULL,     NULL, NULL, NULL, NULL},
	{uyvy422_to_yuv420p,  NULL,     NULL,     NULL, NULL, NULL, NULL},
	{rgb32_to_yuv420p,    NULL,     NULL,     NULL, NULL, NULL, NULL},
	{rgb32_to_yuv420p,    NULL,     NULL,     NULL, NULL, NULL, NULL},
	{NULL,                NULL,     NULL,     NULL, NULL, NULL, NULL},
	{NULL,                NULL,     NULL,     NULL, NULL, NULL, NULL},
	{nv12_to_yuv420p,     NULL,     NULL,     NULL, NULL, NULL, NULL},
	{nv21_to_yuv420p,     NULL,     NULL,     NULL, NULL, NULL, NULL},
};


/**
 * Convert a video frame from one pixel format to another pixel format
 *
 * Speed matches swscale: SWS_BILINEAR
 *
 * todo: optimize (check out SWS_FAST_BILINEAR)
 *
 * @param dst  Destination video frame
 * @param src  Source video frame
 * @param r    Drawing area in destination frame, NULL means whole frame
 */
void vidconv(struct vidframe *dst, const struct vidframe *src,
	     struct vidrect *r)
{
	struct vidrect rdst;
	unsigned yd, ys, ys2, lsd, lss, y;
	const uint8_t *ds0, *ds1, *ds2;
	uint8_t *dd0, *dd1, *dd2;
	double rw, rh;
	line_h *lineh = NULL;

	if (!vidframe_isvalid(dst) || !vidframe_isvalid(src))
		return;

	if (src->fmt < MAX_SRC && dst->fmt < MAX_DST) {

		/* Lookup conversion function */
		lineh = conv_table[src->fmt][dst->fmt];
	}
	if (!lineh) {
		(void)re_printf("vidconv: no pixel converter found for"
				" %s -> %s\n", vidfmt_name(src->fmt),
				vidfmt_name(dst->fmt));
		return;
	}

	if (r) {
		r->x &= ~1;
		r->y &= ~1;
		r->w &= ~1;
		r->h &= ~1;

		if ((r->x + r->w) > dst->size.w ||
		    (r->y + r->h) > dst->size.h) {
			(void)re_printf("vidconv: out of bounds (%u x %u)\n",
					dst->size.w, dst->size.h);
			return;
		}
	}
	else {
		rdst.x = rdst.y = 0;
		rdst.w = dst->size.w & ~1;
		rdst.h = dst->size.h & ~1;
		r = &rdst;
	}

	rw = (double)src->size.w / (double)r->w;
	rh = (double)src->size.h / (double)r->h;

	lsd = dst->linesize[0];
	lss = src->linesize[0];

	dd0 = dst->data[0];
	dd1 = dst->data[1];
	dd2 = dst->data[2];

	ds0 = src->data[0];
	ds1 = src->data[1];
	ds2 = src->data[2];

	for (y=0; y<r->h; y+=2) {

		yd  = y + r->y;

		ys  = (unsigned)(y * rh);
		ys2 = (unsigned)((y+1) * rh);

		lineh(r->x, r->w, rw, yd, ys, ys2,
		      dd0, dd1, dd2, lsd,
		      ds0, ds1, ds2, lss);
	}
}


/**
 * Same as vidconv(), but maintain source aspect ratio within bounds of r
 *
 * @param dst  Destination video frame
 * @param src  Source video frame
 * @param r    Drawing area in destination frame
 */
void vidconv_aspect(struct vidframe *dst, const struct vidframe *src,
		    struct vidrect *r)
{
	struct vidsz asz;
	double ar;

	ar = (double)src->size.w / (double)src->size.h;

	asz.w = r->w;
	asz.h = r->h;

	r->w = (unsigned)min((double)asz.w, (double)asz.h * ar);
	r->h = (unsigned)min((double)asz.h, (double)asz.w / ar);
	r->x = r->x + (asz.w - r->w) / 2;
	r->y = r->y + (asz.h - r->h) / 2;

	vidconv(dst, src, r);
}
