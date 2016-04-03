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

void parse_PPS(decoder_context *decoder)
{
	bitstream_reader *reader = &decoder->reader;
	int i;

	decoder->pps.valid = 0;

	decoder->pps.pic_parameter_set_id = bitstream_read_ue(reader);
	decoder->pps.seq_parameter_set_id = bitstream_read_ue(reader);
	decoder->pps.entropy_coding_mode_flag = bitstream_read_u(reader, 1);
	decoder->pps.bottom_field_pic_order_in_frame_present_flag = bitstream_read_u(reader, 1);
	decoder->pps.num_slice_groups_minus1 = bitstream_read_ue(reader);

	SYNTAX_IPRINT("pic_parameter_set_id = %u\n",
		      decoder->pps.pic_parameter_set_id);
	SYNTAX_IPRINT("seq_parameter_set_id = %u\n",
		      decoder->pps.seq_parameter_set_id);
	SYNTAX_IPRINT("entropy_coding_mode_flag (CABAC) = %u\n",
		      decoder->pps.entropy_coding_mode_flag);
	SYNTAX_IPRINT("bottom_field_pic_order_in_frame_present_flag = %u\n",
		      decoder->pps.bottom_field_pic_order_in_frame_present_flag);
	SYNTAX_IPRINT("num_slice_groups_minus1 = %u\n",
		      decoder->pps.num_slice_groups_minus1);

	if (decoder->pps.num_slice_groups_minus1 > 0) {
		decoder->pps.slice_group_map_type = bitstream_read_ue(reader);

		SYNTAX_IPRINT("slice_group_map_type = %u\n",
			      decoder->pps.slice_group_map_type);

		switch (decoder->pps.slice_group_map_type) {
		case 0:
			decoder->pps.run_length_minus1 =
				realloc(decoder->pps.run_length_minus1,
					decoder->pps.num_slice_groups_minus1 + 1);

			assert(decoder->pps.run_length_minus1 != NULL);

			for (i = 0; i <= decoder->pps.num_slice_groups_minus1; i++) {
				decoder->pps.run_length_minus1[i] =
							bitstream_read_ue(reader);

				SYNTAX_IPRINT("run_length_minus1[%d] = %u\n",
					      i, decoder->pps.run_length_minus1[i]);
			}
			break;
		case 2:
			decoder->pps.top_left =
				realloc(decoder->pps.top_left,
					decoder->pps.num_slice_groups_minus1 + 1);

			assert(decoder->pps.top_left != NULL);

			decoder->pps.bottom_right =
				realloc(decoder->pps.bottom_right,
					decoder->pps.num_slice_groups_minus1 + 1);

			assert(decoder->pps.bottom_right != NULL);

			for (i = 0; i < decoder->pps.num_slice_groups_minus1; i++) {
				decoder->pps.top_left[i] = bitstream_read_ue(reader);
				decoder->pps.bottom_right[i] = bitstream_read_ue(reader);

				SYNTAX_IPRINT("top_left[%d] = %u\n",
					      i, decoder->pps.top_left[i]);
				SYNTAX_IPRINT("bottom_right[%d] = %u\n",
					      i, decoder->pps.bottom_right[i]);
			}
			break;
		case 3 ... 5:
			decoder->pps.slice_group_change_direction_flag =
						bitstream_read_u(reader, 1);
			decoder->pps.slice_group_change_rate_minus1 =
						bitstream_read_ue(reader);

			SYNTAX_IPRINT("slice_group_change_direction_flag = %u\n",
				      decoder->pps.slice_group_change_direction_flag);
			SYNTAX_IPRINT("slice_group_change_rate_minus1 = %u\n",
				      decoder->pps.slice_group_change_rate_minus1);
			break;
		case 6:
			decoder->pps.pic_size_in_map_units_minus1 =
						bitstream_read_ue(reader);

			SYNTAX_IPRINT("pic_size_in_map_units_minus1 = %u\n",
				      decoder->pps.pic_size_in_map_units_minus1);

			decoder->pps.slice_group_id =
				realloc(decoder->pps.slice_group_id,
					decoder->pps.pic_size_in_map_units_minus1 + 1);

			assert(decoder->pps.slice_group_id != NULL);

			for (i = 0; i <= decoder->pps.pic_size_in_map_units_minus1; i++) {
				int bits_nb =
					32 - clz(decoder->pps.pic_size_in_map_units_minus1 + 1);

				decoder->pps.slice_group_id[i] = bitstream_read_u(reader, bits_nb);

				SYNTAX_IPRINT("slice_group_id[%d] = %u\n",
					      i, decoder->pps.slice_group_id[i]);
			}
			break;
		default:
			break;
		}
	}

	decoder->pps.num_ref_idx_l0_default_active_minus1   = bitstream_read_ue(reader);
	decoder->pps.num_ref_idx_l1_default_active_minus1   = bitstream_read_ue(reader);
	decoder->pps.weighted_pred_flag                     = bitstream_read_u(reader, 1);
	decoder->pps.weighted_bipred_idc                    = bitstream_read_u(reader, 2);
	decoder->pps.pic_init_qp_minus26                    = bitstream_read_se(reader);
	decoder->pps.pic_init_qs_minus26                    = bitstream_read_se(reader);
	decoder->pps.chroma_qp_index_offset                 = bitstream_read_se(reader);
	decoder->pps.deblocking_filter_control_present_flag = bitstream_read_u(reader, 1);
	decoder->pps.constrained_intra_pred_flag            = bitstream_read_u(reader, 1);
	decoder->pps.redundant_pic_cnt_present_flag         = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("num_ref_idx_l0_default_active_minus1 = %u\n",
		      decoder->pps.num_ref_idx_l0_default_active_minus1);
	SYNTAX_IPRINT("num_ref_idx_l1_default_active_minus1 = %u\n",
		      decoder->pps.num_ref_idx_l1_default_active_minus1);
	SYNTAX_IPRINT("weighted_pred_flag = %u\n",
		      decoder->pps.weighted_pred_flag);
	SYNTAX_IPRINT("weighted_bipred_idc = %u\n",
		      decoder->pps.weighted_bipred_idc);
	SYNTAX_IPRINT("pic_init_qp_minus26 = %d\n",
		      decoder->pps.pic_init_qp_minus26);
	SYNTAX_IPRINT("pic_init_qs_minus26 = %d\n",
		      decoder->pps.pic_init_qs_minus26);
	SYNTAX_IPRINT("chroma_qp_index_offset = %d\n",
		      decoder->pps.chroma_qp_index_offset);
	SYNTAX_IPRINT("deblocking_filter_control_present_flag = %u\n",
		      decoder->pps.deblocking_filter_control_present_flag);
	SYNTAX_IPRINT("constrained_intra_pred_flag = %u\n",
		      decoder->pps.constrained_intra_pred_flag);
	SYNTAX_IPRINT("redundant_pic_cnt_present_flag = %u\n",
		      decoder->pps.redundant_pic_cnt_present_flag);

	if (!more_rbsp_data(decoder)) {
		goto end;
	}

	decoder->pps.transform_8x8_mode_flag =
					bitstream_read_u(reader, 1);
	decoder->pps.pic_scaling_matrix_present_flag =
					bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("transform_8x8_mode_flag = %u\n",
		      decoder->pps.transform_8x8_mode_flag);
	SYNTAX_IPRINT("pic_scaling_matrix_present_flag = %u\n",
		      decoder->pps.pic_scaling_matrix_present_flag);

	if (decoder->pps.transform_8x8_mode_flag) {
		SYNTAX_ERR("transform_8x8 unsupported\n");
	}

	if (decoder->pps.pic_scaling_matrix_present_flag) {
		for (i = 0; i < 6 + ( ( decoder->sps.chroma_format_idc != 3 ) ? 2 : 6 ) * decoder->pps.transform_8x8_mode_flag; i++) {
			unsigned present_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("scaling list %s[%d]: %s",
				      i < 6 ? "4x4" : "8x8", i,
				      present_flag ? "\n" : "not present\n");

			if (!present_flag) {
				continue;
			}

			if (i < 6) {
				scaling_list(reader, decoder->pps.scalingList_4x4[i], 4,
					     &decoder->pps.UseDefaultScalingMatrix4x4Flag[i]);
			} else {
				scaling_list(reader, decoder->pps.scalingList_8x8[i - 6], 8,
					     &decoder->pps.UseDefaultScalingMatrix8x8Flag[i - 6]);
			}
		}
	}

	decoder->pps.second_chroma_qp_index_offset =
					bitstream_read_se(reader);

	SYNTAX_IPRINT("second_chroma_qp_index_offset = %d\n",
		      decoder->pps.second_chroma_qp_index_offset);

	if (more_rbsp_data(decoder)) {
		SYNTAX_ERR("PPS is malformed\n");
	}

end:
	decoder->pps.valid = 1;
}
