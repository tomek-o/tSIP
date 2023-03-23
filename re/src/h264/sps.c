/**
 * @file h264/sps.c H.264 SPS parser
 *
 * Copyright (C) 2010 Creytiv.com
 */

#include <string.h>
#include <re_types.h>
#include <re_fmt.h>
#include <re_mbuf.h>
#include <re_h264.h>
#include "h264.h"

#define DEBUG_MODULE "h264_sps"
#define DEBUG_LEVEL 5
#include <re_dbg.h>


enum {
	MAX_SPS_COUNT          = 32,
	MAX_LOG2_MAX_FRAME_NUM = 16,
	MACROBLOCK_SIZE        = 16,
};

#define MAX_MACROBLOCKS 1048576u


static int scaling_list(struct getbit *gb,
			unsigned *scalingl, size_t sizeofscalinglist,
			bool *usedefaultscalingmatrix)
{
	unsigned lastscale = 8;
	unsigned nextscale = 8;
	size_t j;
	int err;

	for (j = 0; j < sizeofscalinglist; j++) {

		if (nextscale != 0) {

			unsigned delta_scale;

			err = get_ue_golomb(gb, &delta_scale);
			if (err)
				return err;

			nextscale = (lastscale + delta_scale + 256) % 256;

			*usedefaultscalingmatrix = (j==0 && nextscale==0);
		}

		scalingl[j] = (nextscale==0) ? lastscale : nextscale;

		lastscale = scalingl[j];
	}

	return 0;
}


static int decode_scaling_matrix(struct getbit *gb, unsigned chroma_format_idc)
{
	unsigned scalinglist4x4[16];
	unsigned scalinglist8x8[64];
	bool usedefaultscalingmatrix[12];
	unsigned i;
	int err;

	for (i = 0; i < ((chroma_format_idc != 3) ? 8u : 12u); i++) {

		unsigned seq_scaling_list_present_flag;

		if (getbit_get_left(gb) < 1)
			return EBADMSG;

		seq_scaling_list_present_flag = get_bit(gb);

		if (seq_scaling_list_present_flag) {

			if (i < 6) {
				err = scaling_list(gb, scalinglist4x4, 16,
					   &usedefaultscalingmatrix[i]);
			}
			else {
				err = scaling_list(gb, scalinglist8x8, 64,
					   &usedefaultscalingmatrix[i]);
			}

			if (err)
				return err;
		}
	}

	return 0;
}


/**
 * Decode a Sequence Parameter Set (SPS) bitstream
 *
 * @param sps  Decoded H.264 SPS
 * @param p    SPS bitstream to decode, excluding NAL header
 * @param len  Number of bytes
 *
 * @return 0 if success, otherwise errorcode
 */
int h264_sps_decode(struct h264_sps *sps, const uint8_t *p, size_t len)
{
	struct getbit gb;
	uint8_t profile_idc;
	unsigned seq_parameter_set_id;
	unsigned frame_mbs_only_flag;
	unsigned chroma_format_idc = 1;
	bool frame_cropping_flag;
	unsigned mb_w_m1;
	unsigned mb_h_m1;
	int err;

	if (!sps || !p || len < 3)
		return EINVAL;

	memset(sps, 0, sizeof(*sps));

	profile_idc    = p[0];
	sps->level_idc = p[2];

	getbit_init(&gb, p+3, (len-3)*8);

	err = get_ue_golomb(&gb, &seq_parameter_set_id);
	if (err)
		return err;

	if (seq_parameter_set_id >= MAX_SPS_COUNT) {
		DEBUG_WARNING("h264: sps: sps_id %u out of range\n",
			   seq_parameter_set_id);
		return EBADMSG;
	}

	if (profile_idc == 100 ||
	    profile_idc == 110 ||
	    profile_idc == 122 ||
	    profile_idc == 244 ||
	    profile_idc ==  44 ||
	    profile_idc ==  83 ||
	    profile_idc ==  86 ||
	    profile_idc == 118 ||
	    profile_idc == 128 ||
	    profile_idc == 138 ||
	    profile_idc == 144) {

		unsigned seq_scaling_matrix_present_flag;

		err = get_ue_golomb(&gb, &chroma_format_idc);
		if (err)
			return err;

		if (chroma_format_idc > 3U) {
			return EBADMSG;
		}
		else if (chroma_format_idc == 3) {

			if (getbit_get_left(&gb) < 1)
				return EBADMSG;

			/* separate_colour_plane_flag */
			(void)get_bit(&gb);
		}

		/* bit_depth_luma/chroma */
		err  = get_ue_golomb(&gb, NULL);
		err |= get_ue_golomb(&gb, NULL);
		if (err)
			return err;

		if (getbit_get_left(&gb) < 2)
			return EBADMSG;

		/* qpprime_y_zero_transform_bypass_flag */
		get_bit(&gb);

		seq_scaling_matrix_present_flag = get_bit(&gb);
		if (seq_scaling_matrix_present_flag) {

			err = decode_scaling_matrix(&gb, chroma_format_idc);
			if (err)
				return err;
		}
	}

	err = get_ue_golomb(&gb, &sps->log2_max_frame_num);
	if (err)
		return err;

	sps->log2_max_frame_num += 4;

	if (sps->log2_max_frame_num > MAX_LOG2_MAX_FRAME_NUM) {
		DEBUG_WARNING("h264: sps: log2_max_frame_num"
			   " out of range: %u\n", sps->log2_max_frame_num);
		return EBADMSG;
	}

	err = get_ue_golomb(&gb, &sps->pic_order_cnt_type);
	if (err)
		return err;

	if (sps->pic_order_cnt_type == 0) {

		/* log2_max_pic_order_cnt_lsb */
		err = get_ue_golomb(&gb, NULL);
		if (err)
			return err;
	}
	else if (sps->pic_order_cnt_type == 2) {
	}
	else {
		DEBUG_WARNING("h264: sps: WARNING:"
			   " unknown pic_order_cnt_type (%u)\n",
			   sps->pic_order_cnt_type);
		return ENOTSUP;
	}

	err = get_ue_golomb(&gb, &sps->max_num_ref_frames);
	if (err)
		return err;

	if (getbit_get_left(&gb) < 1)
		return EBADMSG;

	/* gaps_in_frame_num_value_allowed_flag */
	(void)get_bit(&gb);

	err  = get_ue_golomb(&gb, &mb_w_m1);
	err |= get_ue_golomb(&gb, &mb_h_m1);
	if (err)
		return err;

	if (getbit_get_left(&gb) < 1)
		return EBADMSG;
	frame_mbs_only_flag = get_bit(&gb);

	sps->pic_width_in_mbs        = mb_w_m1 + 1;
	sps->pic_height_in_map_units = mb_h_m1 + 1;

	sps->pic_height_in_map_units *= 2 - frame_mbs_only_flag;

	if (sps->pic_width_in_mbs >= MAX_MACROBLOCKS ||
	    sps->pic_height_in_map_units >= MAX_MACROBLOCKS) {
		DEBUG_WARNING("h264: sps: width/height overflow\n");
		return EBADMSG;
	}

	if (!frame_mbs_only_flag) {

		if (getbit_get_left(&gb) < 1)
			return EBADMSG;

		/* mb_adaptive_frame_field_flag */
		(void)get_bit(&gb);
	}

	if (getbit_get_left(&gb) < 1)
		return EBADMSG;

	/* direct_8x8_inference_flag */
	(void)get_bit(&gb);

	if (getbit_get_left(&gb) < 1)
		return EBADMSG;

	frame_cropping_flag = get_bit(&gb);

	if (frame_cropping_flag) {

		unsigned crop_left;
		unsigned crop_right;
		unsigned crop_top;
		unsigned crop_bottom;

		unsigned vsub   = (chroma_format_idc == 1) ? 1 : 0;
		unsigned hsub   = (chroma_format_idc == 1 ||
			      chroma_format_idc == 2) ? 1 : 0;
		unsigned sx = 1u << hsub;
		unsigned sy = (2u - frame_mbs_only_flag) << vsub;

		unsigned w = MACROBLOCK_SIZE * sps->pic_width_in_mbs;
		unsigned h = MACROBLOCK_SIZE * sps->pic_height_in_map_units;

		err  = get_ue_golomb(&gb, &crop_left);
		err |= get_ue_golomb(&gb, &crop_right);
		err |= get_ue_golomb(&gb, &crop_top);
		err |= get_ue_golomb(&gb, &crop_bottom);
		if (err)
			return err;

		if ((crop_left + crop_right ) * sx >= w ||
		    (crop_top  + crop_bottom) * sy >= h) {
			DEBUG_WARNING("h264: sps: crop values invalid"
				   " %u %u %u %u / %u %u\n",
				   crop_left, crop_right, crop_top,
				   crop_bottom, w, h);
			return EBADMSG;
		}

		sps->frame_crop_left_offset   = sx * crop_left;
		sps->frame_crop_right_offset  = sx * crop_right;
		sps->frame_crop_top_offset    = sy * crop_top;
		sps->frame_crop_bottom_offset = sy * crop_bottom;
	}

	/* success */
	sps->profile_idc = profile_idc;
	sps->seq_parameter_set_id = (uint8_t)seq_parameter_set_id;
	sps->chroma_format_idc = chroma_format_idc;

	return 0;
}


void h264_sps_resolution(const struct h264_sps *sps,
		unsigned *width, unsigned *height)
{
	if (!sps || !width || !height)
		return;

	*width = MACROBLOCK_SIZE * sps->pic_width_in_mbs
		- sps->frame_crop_left_offset
		- sps->frame_crop_right_offset;

	*height = MACROBLOCK_SIZE * sps->pic_height_in_map_units
		- sps->frame_crop_top_offset
		- sps->frame_crop_bottom_offset;
}


const char *h264_sps_chroma_format_name(uint8_t chroma_format_idc)
{
	switch (chroma_format_idc) {

	case 0: return "monochrome";
	case 1: return "YUV420";
	case 2: return "YUV422";
	case 3: return "YUV444";
	default: return "???";
	}
}
