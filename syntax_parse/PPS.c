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
	decoder_context_sps *sps;
	decoder_context_pps *pps;
	uint32_t pps_id;
	uint32_t sps_id;
	int i;

	pps_id = bitstream_read_ue(reader);

	SYNTAX_IPRINT("pic_parameter_set_id = %u\n", pps_id);

	if (pps_id > 255) {
		SYNTAX_ERR("PPS is malformed, pps_id overflow\n");
	}

	pps = &decoder->pps[pps_id];

	decoder_reset_PPS(pps);

	sps_id = bitstream_read_ue(reader);

	SYNTAX_IPRINT("seq_parameter_set_id = %u\n", sps_id);

	if (sps_id > 31) {
		SYNTAX_ERR("PPS is malformed, sps_id overflow\n");
	}

	sps = &decoder->sps[sps_id];

	if (!sps->valid) {
		SYNTAX_ERR("PPS is malformed, SPS is invalid\n");
	}

	pps->seq_parameter_set_id = sps_id;
	pps->entropy_coding_mode_flag = bitstream_read_u(reader, 1);
	pps->bottom_field_pic_order_in_frame_present_flag = bitstream_read_u(reader, 1);
	pps->num_slice_groups_minus1 = bitstream_read_ue(reader);

	SYNTAX_IPRINT("entropy_coding_mode_flag (CABAC) = %u\n",
		      pps->entropy_coding_mode_flag);
	SYNTAX_IPRINT("bottom_field_pic_order_in_frame_present_flag = %u\n",
		      pps->bottom_field_pic_order_in_frame_present_flag);
	SYNTAX_IPRINT("num_slice_groups_minus1 = %u\n",
		      pps->num_slice_groups_minus1);

	if (pps->num_slice_groups_minus1 > 0) {
		pps->slice_group_map_type = bitstream_read_ue(reader);

		SYNTAX_IPRINT("slice_group_map_type = %u\n",
			      pps->slice_group_map_type);

		switch (pps->slice_group_map_type) {
		case 0:
			pps->run_length_minus1 =
				realloc(pps->run_length_minus1,
					pps->num_slice_groups_minus1 + 1);

			assert(pps->run_length_minus1 != NULL);

			for (i = 0; i <= pps->num_slice_groups_minus1; i++) {
				pps->run_length_minus1[i] =
							bitstream_read_ue(reader);

				SYNTAX_IPRINT("run_length_minus1[%d] = %u\n",
					      i, pps->run_length_minus1[i]);
			}
			break;
		case 2:
			pps->top_left =
				realloc(pps->top_left,
					pps->num_slice_groups_minus1 + 1);

			assert(pps->top_left != NULL);

			pps->bottom_right =
				realloc(pps->bottom_right,
					pps->num_slice_groups_minus1 + 1);

			assert(pps->bottom_right != NULL);

			for (i = 0; i < pps->num_slice_groups_minus1; i++) {
				pps->top_left[i] = bitstream_read_ue(reader);
				pps->bottom_right[i] = bitstream_read_ue(reader);

				SYNTAX_IPRINT("top_left[%d] = %u\n",
					      i, pps->top_left[i]);
				SYNTAX_IPRINT("bottom_right[%d] = %u\n",
					      i, pps->bottom_right[i]);
			}
			break;
		case 3 ... 5:
			pps->slice_group_change_direction_flag =
						bitstream_read_u(reader, 1);
			pps->slice_group_change_rate_minus1 =
						bitstream_read_ue(reader);

			SYNTAX_IPRINT("slice_group_change_direction_flag = %u\n",
				      pps->slice_group_change_direction_flag);
			SYNTAX_IPRINT("slice_group_change_rate_minus1 = %u\n",
				      pps->slice_group_change_rate_minus1);
			break;
		case 6:
			pps->pic_size_in_map_units_minus1 =
						bitstream_read_ue(reader);

			SYNTAX_IPRINT("pic_size_in_map_units_minus1 = %u\n",
				      pps->pic_size_in_map_units_minus1);

			pps->slice_group_id =
				realloc(pps->slice_group_id,
					pps->pic_size_in_map_units_minus1 + 1);

			assert(pps->slice_group_id != NULL);

			for (i = 0; i <= pps->pic_size_in_map_units_minus1; i++) {
				int bits_nb =
					32 - clz(pps->pic_size_in_map_units_minus1 + 1);

				pps->slice_group_id[i] = bitstream_read_u(reader, bits_nb);

				SYNTAX_IPRINT("slice_group_id[%d] = %u\n",
					      i, pps->slice_group_id[i]);
			}
			break;
		default:
			break;
		}
	}

	pps->num_ref_idx_l0_default_active_minus1   = bitstream_read_ue(reader);
	pps->num_ref_idx_l1_default_active_minus1   = bitstream_read_ue(reader);
	pps->weighted_pred_flag                     = bitstream_read_u(reader, 1);
	pps->weighted_bipred_idc                    = bitstream_read_u(reader, 2);
	pps->pic_init_qp_minus26                    = bitstream_read_se(reader);
	pps->pic_init_qs_minus26                    = bitstream_read_se(reader);
	pps->chroma_qp_index_offset                 = bitstream_read_se(reader);
	pps->deblocking_filter_control_present_flag = bitstream_read_u(reader, 1);
	pps->constrained_intra_pred_flag            = bitstream_read_u(reader, 1);
	pps->redundant_pic_cnt_present_flag         = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("num_ref_idx_l0_default_active_minus1 = %u\n",
		      pps->num_ref_idx_l0_default_active_minus1);
	SYNTAX_IPRINT("num_ref_idx_l1_default_active_minus1 = %u\n",
		      pps->num_ref_idx_l1_default_active_minus1);
	SYNTAX_IPRINT("weighted_pred_flag = %u\n",
		      pps->weighted_pred_flag);
	SYNTAX_IPRINT("weighted_bipred_idc = %u\n",
		      pps->weighted_bipred_idc);
	SYNTAX_IPRINT("pic_init_qp_minus26 = %d\n",
		      pps->pic_init_qp_minus26);
	SYNTAX_IPRINT("pic_init_qs_minus26 = %d\n",
		      pps->pic_init_qs_minus26);
	SYNTAX_IPRINT("chroma_qp_index_offset = %d\n",
		      pps->chroma_qp_index_offset);
	SYNTAX_IPRINT("deblocking_filter_control_present_flag = %u\n",
		      pps->deblocking_filter_control_present_flag);
	SYNTAX_IPRINT("constrained_intra_pred_flag = %u\n",
		      pps->constrained_intra_pred_flag);
	SYNTAX_IPRINT("redundant_pic_cnt_present_flag = %u\n",
		      pps->redundant_pic_cnt_present_flag);

	if (!more_rbsp_data(decoder)) {
		goto end;
	}

	pps->transform_8x8_mode_flag	     = bitstream_read_u(reader, 1);
	pps->pic_scaling_matrix_present_flag = bitstream_read_u(reader, 1);

	SYNTAX_IPRINT("transform_8x8_mode_flag = %u\n",
		      pps->transform_8x8_mode_flag);
	SYNTAX_IPRINT("pic_scaling_matrix_present_flag = %u\n",
		      pps->pic_scaling_matrix_present_flag);

	if (pps->transform_8x8_mode_flag) {
		SYNTAX_ERR("transform_8x8 unsupported\n");
	}

	if (pps->pic_scaling_matrix_present_flag) {
		for (i = 0; i < 6 + ( ( sps->chroma_format_idc != 3 ) ? 2 : 6 ) * pps->transform_8x8_mode_flag; i++) {
			unsigned present_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("scaling list %s[%d]: %s",
				      i < 6 ? "4x4" : "8x8", i,
				      present_flag ? "\n" : "not present\n");

			if (!present_flag) {
				continue;
			}

			if (i < 6) {
				scaling_list(reader, pps->scalingList_4x4[i], 4,
					     &pps->UseDefaultScalingMatrix4x4Flag[i]);
			} else {
				scaling_list(reader, pps->scalingList_8x8[i - 6], 8,
					     &pps->UseDefaultScalingMatrix8x8Flag[i - 6]);
			}
		}
	}

	pps->second_chroma_qp_index_offset =
					bitstream_read_se(reader);

	SYNTAX_IPRINT("second_chroma_qp_index_offset = %d\n",
		      pps->second_chroma_qp_index_offset);

	if (more_rbsp_data(decoder)) {
		SYNTAX_ERR("PPS is malformed\n");
	}

end:
	pps->valid = 1;
}
