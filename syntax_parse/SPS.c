/*
 * Copyright (c) 2016 Dmitry Osipenko <digetx@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"

void scaling_list(bitstream_reader *reader, int8_t *scalingList,
		  unsigned sizeOfScalingList, unsigned *useDefaultScalingMatrixFlag)
{
	int32_t lastScale = 8;
	int32_t nextScale = 8;
	int j;

	for (j = 0; j < sizeOfScalingList * sizeOfScalingList; j++) {
		if (nextScale != 0) {
			int32_t delta_scale = bitstream_read_se(reader);

			nextScale = (lastScale + delta_scale + 256) % 256;
			*useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
		}

		scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
		lastScale = scalingList[j];

		SYNTAX_IPRINT("%d\t", scalingList[j]);

		if (j > 0 && j % sizeOfScalingList == 0) {
			SYNTAX_IPRINT("\n");
		}
	}
}

void parse_SPS(decoder_context *decoder)
{
	bitstream_reader *reader = &decoder->reader;
	decoder_context_sps *sps;
	uint32_t sps_id;
	unsigned profile_idc;
	unsigned constraint_set0_flag;
	unsigned constraint_set1_flag;
	unsigned constraint_set2_flag;
	unsigned constraint_set3_flag;
	unsigned constraint_set4_flag;
	unsigned constraint_set5_flag;
	unsigned level_idc;
	int i;

	profile_idc	     = bitstream_read_u(reader, 8);
	constraint_set0_flag = bitstream_read_u(reader, 1);
	constraint_set1_flag = bitstream_read_u(reader, 1);
	constraint_set2_flag = bitstream_read_u(reader, 1);
	constraint_set3_flag = bitstream_read_u(reader, 1);
	constraint_set4_flag = bitstream_read_u(reader, 1);
	constraint_set5_flag = bitstream_read_u(reader, 1);

	/* reserved_zero_2bits */
	if (bitstream_read_u(reader, 2) != 0) {
		SYNTAX_ERR("SPS is malformed\n");
	}

	level_idc	= bitstream_read_u(reader, 8);
	sps_id		= bitstream_read_ue(reader);

	SYNTAX_IPRINT("seq_parameter_set_id = %u\n", sps_id);

	if (sps_id > 31) {
		SYNTAX_ERR("SPS is malformed, sps_id overflow\n");
	}

	sps = &decoder->sps[sps_id];

	decoder_reset_SPS(sps);

	SYNTAX_IPRINT("profile_idc = %u\n", profile_idc);
	SYNTAX_IPRINT("level_idc = %u\n", level_idc);
	SYNTAX_IPRINT("constraint_set0_flag = %u\n", constraint_set0_flag);
	SYNTAX_IPRINT("constraint_set1_flag = %u\n", constraint_set1_flag);
	SYNTAX_IPRINT("constraint_set2_flag = %u\n", constraint_set2_flag);
	SYNTAX_IPRINT("constraint_set3_flag = %u\n", constraint_set3_flag);
	SYNTAX_IPRINT("constraint_set4_flag = %u\n", constraint_set4_flag);
	SYNTAX_IPRINT("constraint_set5_flag = %u\n", constraint_set5_flag);

	sps->profile_idc	  = profile_idc;
	sps->level_idc		  = level_idc;
	sps->constraint_set0_flag = constraint_set0_flag;
	sps->constraint_set1_flag = constraint_set1_flag;
	sps->constraint_set2_flag = constraint_set2_flag;
	sps->constraint_set3_flag = constraint_set3_flag;
	sps->constraint_set4_flag = constraint_set4_flag;
	sps->constraint_set5_flag = constraint_set5_flag;

	switch (sps->profile_idc) {
	case 100:
	case 110:
	case 122:
	case 244:
	case 44:
	case 83:
	case 86:
	case 118:
	case 128:
	case 138:
	case 139:
	case 134:
		sps->chroma_format_idc = bitstream_read_ue(reader);

		if (sps->chroma_format_idc == 3) {
			sps->separate_colour_plane_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("separate_colour_plane_flag = %u\n",
			       sps->separate_colour_plane_flag);
		}

		sps->bit_depth_luma_minus8		  = bitstream_read_ue(reader);
		sps->bit_depth_chroma_minus8		  = bitstream_read_ue(reader);
		sps->qpprime_y_zero_transform_bypass_flag = bitstream_read_u(reader, 1);
		sps->seq_scaling_matrix_present_flag	  = bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("bit_depth_luma_minus8 = %u\n",
			      sps->bit_depth_luma_minus8);
		SYNTAX_IPRINT("bit_depth_chroma_minus8 = %u\n",
			      sps->bit_depth_chroma_minus8);
		SYNTAX_IPRINT("qpprime_y_zero_transform_bypass_flag = %u\n",
			      sps->qpprime_y_zero_transform_bypass_flag);
		SYNTAX_IPRINT("seq_scaling_matrix_present_flag = %u\n",
			      sps->seq_scaling_matrix_present_flag);

		if (!sps->seq_scaling_matrix_present_flag) {
			break;
		}

		sps->seq_scaling_list_present_flag = 0;

		for (i = 0; i < (sps->chroma_format_idc != 3) ? 8 : 12; i++) {
			unsigned present_flag = bitstream_read_u(reader, 1);

			sps->seq_scaling_list_present_flag |= present_flag << i;

			SYNTAX_IPRINT("scaling list %s[%d]: %s",
				      i < 6 ? "4x4" : "8x8", i,
				      present_flag ? "\n" : "not present\n");

			if (!present_flag) {
				continue;
			}

			if (i < 6) {
				scaling_list(reader, sps->scalingList_4x4[i], 4,
					     &sps->UseDefaultScalingMatrix4x4Flag[i]);
			} else {
				scaling_list(reader, sps->scalingList_8x8[i - 6], 8,
					     &sps->UseDefaultScalingMatrix8x8Flag[i - 6]);
			}
		}
		break;
	default:
		sps->chroma_format_idc = 1;
		break;
	}

	SYNTAX_IPRINT("chroma_format_idc = %u\n", sps->chroma_format_idc);
	SYNTAX_IPRINT("ChromaArrayType = %u\n", ChromaArrayType());

	if (sps->chroma_format_idc != 1) {
		SYNTAX_ERR("Only YUV4:2:0 supported");
	}

	if (sps->bit_depth_luma_minus8 != 0) {
		SYNTAX_ERR("Luma depth > 8bit not supported");
	}

	if (sps->bit_depth_chroma_minus8 != 0) {
		SYNTAX_ERR("Chroma depth > 8bit not supported");
	}

	sps->log2_max_frame_num_minus4	= bitstream_read_ue(reader);
	sps->pic_order_cnt_type		= bitstream_read_ue(reader);

	SYNTAX_IPRINT("log2_max_frame_num_minus4 = %u\n",
		      sps->log2_max_frame_num_minus4);
	SYNTAX_IPRINT("pic_order_cnt_type = %u\n",
		      sps->pic_order_cnt_type);

	switch (sps->pic_order_cnt_type) {
	case 0:
		sps->log2_max_pic_order_cnt_lsb_minus4 = bitstream_read_ue(reader);

		SYNTAX_IPRINT("log2_max_pic_order_cnt_lsb_minus4 = %u\n",
			      sps->log2_max_pic_order_cnt_lsb_minus4);
		break;
	case 1:
		sps->delta_pic_order_always_zero_flag      = bitstream_read_u(reader, 1);
		sps->offset_for_non_ref_pic		   = bitstream_read_se(reader);
		sps->offset_for_top_to_bottom_field	   = bitstream_read_se(reader);
		sps->num_ref_frames_in_pic_order_cnt_cycle = bitstream_read_ue(reader);

		SYNTAX_IPRINT("delta_pic_order_always_zero_flag = %u\n",
			      sps->delta_pic_order_always_zero_flag);
		SYNTAX_IPRINT("offset_for_non_ref_pic = %u\n",
			      sps->offset_for_non_ref_pic);
		SYNTAX_IPRINT("offset_for_top_to_bottom_field = %u\n",
			      sps->offset_for_top_to_bottom_field);
		SYNTAX_IPRINT("num_ref_frames_in_pic_order_cnt_cycle = %u\n",
			      sps->num_ref_frames_in_pic_order_cnt_cycle);

		sps->offset_for_ref_frame =
			realloc(sps->offset_for_ref_frame,
				sizeof(signed) * sps->num_ref_frames_in_pic_order_cnt_cycle);

		assert(sps->offset_for_ref_frame != NULL);

		for (i = 0; i < sps->num_ref_frames_in_pic_order_cnt_cycle; i++) {
			sps->offset_for_ref_frame[i] = bitstream_read_se(reader);

			SYNTAX_IPRINT("offset_for_ref_frame[%d] = %d\n", i,
				      sps->offset_for_ref_frame[i]);
		}
		break;
	default:
		break;
	}

	sps->max_num_ref_frames			  = bitstream_read_ue(reader);
	sps->gaps_in_frame_num_value_allowed_flag = bitstream_read_u(reader, 1);
	sps->pic_width_in_mbs_minus1		  = bitstream_read_ue(reader);
	sps->pic_height_in_map_units_minus1	  = bitstream_read_ue(reader);
	sps->frame_mbs_only_flag		  = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("max_num_ref_frames = %u\n",
		      sps->max_num_ref_frames);
	SYNTAX_IPRINT("gaps_in_frame_num_value_allowed_flag = %u\n",
		      sps->gaps_in_frame_num_value_allowed_flag);
	SYNTAX_IPRINT("pic_width_in_mbs_minus1 = %u (%upx)\n",
		      sps->pic_width_in_mbs_minus1,
		      (sps->pic_width_in_mbs_minus1 + 1) * 16);
	SYNTAX_IPRINT("pic_height_in_map_units_minus1 = %u (%upx)\n",
		      sps->pic_height_in_map_units_minus1,
		      (sps->pic_height_in_map_units_minus1 + 1) * 16);
	SYNTAX_IPRINT("frame_mbs_only_flag = %u\n",
		      sps->frame_mbs_only_flag);

	if (!sps->frame_mbs_only_flag) {
		sps->mb_adaptive_frame_field_flag = bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("mb_adaptive_frame_field_flag = %u\n",
			      sps->mb_adaptive_frame_field_flag);

		SYNTAX_ERR("Interlace not supported\n");
	}

	sps->direct_8x8_inference_flag	  = bitstream_read_u(reader, 1);
	sps->frame_cropping_flag	  = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("direct_8x8_inference_flag = %u\n",
		      sps->direct_8x8_inference_flag);
	SYNTAX_IPRINT("frame_cropping_flag = %u\n",
		      sps->frame_cropping_flag);

	if (sps->frame_cropping_flag) {
		sps->frame_crop_left_offset   = bitstream_read_ue(reader);
		sps->frame_crop_right_offset  = bitstream_read_ue(reader);
		sps->frame_crop_top_offset    = bitstream_read_ue(reader);
		sps->frame_crop_bottom_offset = bitstream_read_ue(reader);

		SYNTAX_IPRINT("frame_crop_left_offset = 0x%X\n",
			      sps->frame_crop_left_offset);
		SYNTAX_IPRINT("frame_crop_right_offset = 0x%X\n",
			      sps->frame_crop_right_offset);
		SYNTAX_IPRINT("frame_crop_top_offset = 0x%X\n",
			      sps->frame_crop_top_offset);
		SYNTAX_IPRINT("frame_crop_bottom_offset = 0x%X\n",
			      sps->frame_crop_bottom_offset);
	}

	sps->vui_parameters_present_flag = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("vui_parameters_present_flag = %u\n",
		      sps->vui_parameters_present_flag);

	if (sps->vui_parameters_present_flag) {
		SPS_vui_parameters(decoder);
	}

	if (more_rbsp_data(decoder)) {
		SYNTAX_ERR("SPS is malformed\n");
	}

	sps->valid = 1;
}
