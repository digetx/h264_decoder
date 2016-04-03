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
	int i;

	decoder_reset_SPS(decoder);

	decoder->sps.profile_idc		= bitstream_read_u(reader, 8);
	decoder->sps.constraint_set0_flag	= bitstream_read_u(reader, 1);
	decoder->sps.constraint_set1_flag	= bitstream_read_u(reader, 1);
	decoder->sps.constraint_set2_flag 	= bitstream_read_u(reader, 1);
	decoder->sps.constraint_set3_flag	= bitstream_read_u(reader, 1);
	decoder->sps.constraint_set4_flag	= bitstream_read_u(reader, 1);
	decoder->sps.constraint_set5_flag	= bitstream_read_u(reader, 1);

	/* reserved_zero_2bits */
	if (bitstream_read_u(reader, 2) != 0) {
		SYNTAX_ERR("SPS is malformed\n");
	}

	decoder->sps.level_idc			= bitstream_read_u(reader, 8);
	decoder->sps.seq_parameter_set_id	= bitstream_read_ue(reader);

	SYNTAX_IPRINT("profile_idc = %u\n", decoder->sps.profile_idc);
	SYNTAX_IPRINT("constraint_set0_flag = %u\n", decoder->sps.constraint_set0_flag);
	SYNTAX_IPRINT("constraint_set1_flag = %u\n", decoder->sps.constraint_set1_flag);
	SYNTAX_IPRINT("constraint_set2_flag = %u\n", decoder->sps.constraint_set2_flag);
	SYNTAX_IPRINT("constraint_set3_flag = %u\n", decoder->sps.constraint_set3_flag);
	SYNTAX_IPRINT("constraint_set4_flag = %u\n", decoder->sps.constraint_set4_flag);
	SYNTAX_IPRINT("constraint_set5_flag = %u\n", decoder->sps.constraint_set5_flag);
	SYNTAX_IPRINT("level_idc = %u\n", decoder->sps.level_idc);
	SYNTAX_IPRINT("seq_parameter_set_id = %u\n", decoder->sps.seq_parameter_set_id);

	switch (decoder->sps.profile_idc) {
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
		decoder->sps.chroma_format_idc = bitstream_read_ue(reader);

		if (decoder->sps.chroma_format_idc == 3) {
			decoder->sps.separate_colour_plane_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("separate_colour_plane_flag = %u\n",
			       decoder->sps.separate_colour_plane_flag);
		}

		decoder->sps.bit_depth_luma_minus8		  = bitstream_read_ue(reader);
		decoder->sps.bit_depth_chroma_minus8		  = bitstream_read_ue(reader);
		decoder->sps.qpprime_y_zero_transform_bypass_flag = bitstream_read_u(reader, 1);
		decoder->sps.seq_scaling_matrix_present_flag	  = bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("bit_depth_luma_minus8 = %u\n",
			      decoder->sps.bit_depth_luma_minus8);
		SYNTAX_IPRINT("bit_depth_chroma_minus8 = %u\n",
			      decoder->sps.bit_depth_chroma_minus8);
		SYNTAX_IPRINT("qpprime_y_zero_transform_bypass_flag = %u\n",
			      decoder->sps.qpprime_y_zero_transform_bypass_flag);
		SYNTAX_IPRINT("seq_scaling_matrix_present_flag = %u\n",
			      decoder->sps.seq_scaling_matrix_present_flag);

		if (!decoder->sps.seq_scaling_matrix_present_flag) {
			break;
		}

		decoder->sps.seq_scaling_list_present_flag = 0;

		for (i = 0; i < (decoder->sps.chroma_format_idc != 3) ? 8 : 12; i++) {
			unsigned present_flag = bitstream_read_u(reader, 1);

			decoder->sps.seq_scaling_list_present_flag |= present_flag << i;

			SYNTAX_IPRINT("scaling list %s[%d]: %s",
				      i < 6 ? "4x4" : "8x8", i,
				      present_flag ? "\n" : "not present\n");

			if (!present_flag) {
				continue;
			}

			if (i < 6) {
				scaling_list(reader, decoder->sps.scalingList_4x4[i], 4,
					     &decoder->sps.UseDefaultScalingMatrix4x4Flag[i]);
			} else {
				scaling_list(reader, decoder->sps.scalingList_8x8[i - 6], 8,
					     &decoder->sps.UseDefaultScalingMatrix8x8Flag[i - 6]);
			}
		}
		break;
	default:
		decoder->sps.chroma_format_idc = 1;
		break;
	}

	SYNTAX_IPRINT("chroma_format_idc = %u\n", decoder->sps.chroma_format_idc);
	SYNTAX_IPRINT("ChromaArrayType = %u\n", ChromaArrayType());

	if (decoder->sps.chroma_format_idc != 1) {
		SYNTAX_ERR("Only YUV4:2:0 supported");
	}

	if (decoder->sps.bit_depth_luma_minus8 != 0) {
		SYNTAX_ERR("Luma depth > 8bit not supported");
	}

	if (decoder->sps.bit_depth_chroma_minus8 != 0) {
		SYNTAX_ERR("Chroma depth > 8bit not supported");
	}

	decoder->sps.log2_max_frame_num_minus4	= bitstream_read_ue(reader);
	decoder->sps.pic_order_cnt_type		= bitstream_read_ue(reader);

	SYNTAX_IPRINT("log2_max_frame_num_minus4 = %u\n",
		      decoder->sps.log2_max_frame_num_minus4);
	SYNTAX_IPRINT("pic_order_cnt_type = %u\n",
		      decoder->sps.pic_order_cnt_type);

	switch (decoder->sps.pic_order_cnt_type) {
	case 0:
		decoder->sps.log2_max_pic_order_cnt_lsb_minus4 = bitstream_read_ue(reader);

		SYNTAX_IPRINT("log2_max_pic_order_cnt_lsb_minus4 = %u\n",
			      decoder->sps.log2_max_pic_order_cnt_lsb_minus4);
		break;
	case 1:
		decoder->sps.delta_pic_order_always_zero_flag      = bitstream_read_u(reader, 1);
		decoder->sps.offset_for_non_ref_pic		   = bitstream_read_se(reader);
		decoder->sps.offset_for_top_to_bottom_field	   = bitstream_read_se(reader);
		decoder->sps.num_ref_frames_in_pic_order_cnt_cycle = bitstream_read_ue(reader);

		SYNTAX_IPRINT("delta_pic_order_always_zero_flag = %u\n",
			      decoder->sps.delta_pic_order_always_zero_flag);
		SYNTAX_IPRINT("offset_for_non_ref_pic = %u\n",
			      decoder->sps.offset_for_non_ref_pic);
		SYNTAX_IPRINT("offset_for_top_to_bottom_field = %u\n",
			      decoder->sps.offset_for_top_to_bottom_field);
		SYNTAX_IPRINT("num_ref_frames_in_pic_order_cnt_cycle = %u\n",
			      decoder->sps.num_ref_frames_in_pic_order_cnt_cycle);

		decoder->sps.offset_for_ref_frame =
			realloc(decoder->sps.offset_for_ref_frame,
				sizeof(signed) * decoder->sps.num_ref_frames_in_pic_order_cnt_cycle);

		assert(decoder->sps.offset_for_ref_frame != NULL);

		for (i = 0; i < decoder->sps.num_ref_frames_in_pic_order_cnt_cycle; i++) {
			decoder->sps.offset_for_ref_frame[i] = bitstream_read_se(reader);

			SYNTAX_IPRINT("offset_for_ref_frame[%d] = %d\n", i,
				      decoder->sps.offset_for_ref_frame[i]);
		}
		break;
	default:
		break;
	}

	decoder->sps.max_num_ref_frames			  = bitstream_read_ue(reader);
	decoder->sps.gaps_in_frame_num_value_allowed_flag = bitstream_read_u(reader, 1);
	decoder->sps.pic_width_in_mbs_minus1		  = bitstream_read_ue(reader);
	decoder->sps.pic_height_in_map_units_minus1	  = bitstream_read_ue(reader);
	decoder->sps.frame_mbs_only_flag		  = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("max_num_ref_frames = %u\n",
		      decoder->sps.max_num_ref_frames);
	SYNTAX_IPRINT("gaps_in_frame_num_value_allowed_flag = %u\n",
		      decoder->sps.gaps_in_frame_num_value_allowed_flag);
	SYNTAX_IPRINT("pic_width_in_mbs_minus1 = %u (%upx)\n",
		      decoder->sps.pic_width_in_mbs_minus1,
		      (decoder->sps.pic_width_in_mbs_minus1 + 1) * 16);
	SYNTAX_IPRINT("pic_height_in_map_units_minus1 = %u (%upx)\n",
		      decoder->sps.pic_height_in_map_units_minus1,
		      (decoder->sps.pic_height_in_map_units_minus1 + 1) * 16);
	SYNTAX_IPRINT("frame_mbs_only_flag = %u\n",
		      decoder->sps.frame_mbs_only_flag);

	if (!decoder->sps.frame_mbs_only_flag) {
		decoder->sps.mb_adaptive_frame_field_flag = bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("mb_adaptive_frame_field_flag = %u\n",
			      decoder->sps.mb_adaptive_frame_field_flag);

		SYNTAX_ERR("Interlace not supported\n");
	}

	decoder->sps.direct_8x8_inference_flag	  = bitstream_read_u(reader, 1);
	decoder->sps.frame_cropping_flag	  = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("direct_8x8_inference_flag = %u\n",
		      decoder->sps.direct_8x8_inference_flag);
	SYNTAX_IPRINT("frame_cropping_flag = %u\n",
		      decoder->sps.frame_cropping_flag);

	if (decoder->sps.frame_cropping_flag) {
		decoder->sps.frame_crop_left_offset   = bitstream_read_ue(reader);
		decoder->sps.frame_crop_right_offset  = bitstream_read_ue(reader);
		decoder->sps.frame_crop_top_offset    = bitstream_read_ue(reader);
		decoder->sps.frame_crop_bottom_offset = bitstream_read_ue(reader);

		SYNTAX_IPRINT("frame_crop_left_offset = 0x%X\n",
			      decoder->sps.frame_crop_left_offset);
		SYNTAX_IPRINT("frame_crop_right_offset = 0x%X\n",
			      decoder->sps.frame_crop_right_offset);
		SYNTAX_IPRINT("frame_crop_top_offset = 0x%X\n",
			      decoder->sps.frame_crop_top_offset);
		SYNTAX_IPRINT("frame_crop_bottom_offset = 0x%X\n",
			      decoder->sps.frame_crop_bottom_offset);
	}

	decoder->sps.vui_parameters_present_flag = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("vui_parameters_present_flag = %u\n",
		      decoder->sps.vui_parameters_present_flag);

	if (decoder->sps.vui_parameters_present_flag) {
		SPS_vui_parameters(decoder);
	}

	if (more_rbsp_data(decoder)) {
		SYNTAX_ERR("SPS is malformed\n");
	}

	decoder->sps.valid = 1;
}
