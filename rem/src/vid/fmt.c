/**
 * @file fmt.c Video Formats
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <re.h>
#include <rem_vid.h>


/** Video format description table */
const struct vidfmt_desc vidfmt_descv[VID_FMT_N] = {

	{"yuv420p", 3, 3, { {0, 1}, {1, 1}, {2, 1}, {0, 0} } },
	{"yuyv422", 1, 3, { {0, 2}, {0, 4}, {0, 4}, {0, 0} } },
	{"uyvy422", 1, 3, { {0, 2}, {0, 4}, {0, 4}, {0, 0} } },
	{"rgb32",   1, 4, { {0, 4}, {0, 4}, {0, 4}, {0, 4} } },
	{"argb",    1, 4, { {0, 4}, {0, 4}, {0, 4}, {0, 4} } },
	{"rgb565",  1, 3, { {0, 2}, {0, 2}, {0, 2}, {0, 0} } },
	{"rgb555",  1, 3, { {0, 2}, {0, 2}, {0, 2}, {0, 0} } },
	{"nv12",    3, 2, { {0, 1}, {1, 2}, {1, 2}, {0, 0} } },
	{"nv21",    3, 2, { {0, 1}, {1, 2}, {1, 2}, {0, 0} } },
};


/**
 * Get the name of a video format
 *
 * @param fmt Video format
 *
 * @return Name of the video format
 */
const char *vidfmt_name(enum vidfmt fmt)
{
	if (fmt >= VID_FMT_N)
		return "???";

	return vidfmt_descv[fmt].name;
}
