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

#define IdrPicFlag	(decoder->nal.unit_type == 5)

static const char * SLICE_TYPE(int type)
{
	switch (type) {
	case P:		return "P";
	case B:		return "B";
	case I:		return "I";
	case SP:	return "SP";
	case SI:	return "SI";
	case P_ONLY:	return "P_ONLY";
	case B_ONLY:	return "B_ONLY";
	case I_ONLY:	return "I_ONLY";
	case SP_ONLY:	return "SP_ONLY";
	case SI_ONLY:	return "SI_ONLY";
	default:
		break;
	}

	SYNTAX_ERR("slice_type is malformed\n");

	return "Bad value";
}

void parse_slice_header(decoder_context *decoder)
{
	bitstream_reader *reader = &decoder->reader;
	decoder_context_sps *sps;
	decoder_context_pps *pps;
	unsigned pps_id;
	int i;

	decoder_reset_SH(decoder);

	decoder->sh.first_mb_in_slice = bitstream_read_ue(reader);
	decoder->sh.slice_type = bitstream_read_ue(reader);
	pps_id = bitstream_read_ue(reader);

	SYNTAX_IPRINT("first_mb_in_slice = %u\n", decoder->sh.first_mb_in_slice);
	SYNTAX_IPRINT("slice_type %u = \"%s\"\n", decoder->sh.slice_type, SLICE_TYPE(decoder->sh.slice_type));
	SYNTAX_IPRINT("pic_parameter_set_id = %u\n", pps_id);

	if (pps_id > 255) {
		SYNTAX_ERR("Slice header is malformed, pps_id overflow\n");
	}

	if (!decoder->pps[pps_id].valid) {
		SYNTAX_ERR("Cannot parse slice while PPS is invalid\n");
	}

	decoder->active_pps = &decoder->pps[pps_id];
	pps = decoder->active_pps;

	decoder->active_sps = &decoder->sps[pps->seq_parameter_set_id];
	sps = decoder->active_sps;

	decoder->sh.slice_type %= 5;

	if (sps->separate_colour_plane_flag) {
		decoder->sh.colour_plane_id = bitstream_read_u(reader, 2);

		SYNTAX_IPRINT("colour_plane_id = %u\n", decoder->sh.colour_plane_id);
	}

	decoder->sh.frame_num =
		bitstream_read_u(reader, sps->log2_max_frame_num_minus4 + 4);

	SYNTAX_IPRINT("frame_num = %u\n", decoder->sh.frame_num);

	if (!sps->frame_mbs_only_flag) {
		decoder->sh.field_pic_flag = bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("field_pic_flag = %u\n", decoder->sh.field_pic_flag);

		if (decoder->sh.field_pic_flag) {
			decoder->sh.bottom_field_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("bottom_field_flag = %u\n",
				      decoder->sh.bottom_field_flag);
		}
	}

	if (IdrPicFlag) {
		decoder->sh.idr_pic_id = bitstream_read_ue(reader);

		SYNTAX_IPRINT("idr_pic_id = %u\n", decoder->sh.idr_pic_id);
	}

	if (sps->pic_order_cnt_type == 0) {
		decoder->sh.pic_order_cnt_lsb =
			bitstream_read_u(reader, sps->log2_max_pic_order_cnt_lsb_minus4 + 4 );

		SYNTAX_IPRINT("pic_order_cnt_lsb = %u\n",
			      decoder->sh.pic_order_cnt_lsb);

		if (pps->bottom_field_pic_order_in_frame_present_flag &&
			!decoder->sh.field_pic_flag)
		{
			decoder->sh.delta_pic_order_cnt_bottom = bitstream_read_se(reader);

			SYNTAX_IPRINT("delta_pic_order_cnt_bottom = %d\n",
				      decoder->sh.delta_pic_order_cnt_bottom);
		}
	}

	if (sps->pic_order_cnt_type == 1 &&
		!sps->delta_pic_order_always_zero_flag)
	{
		decoder->sh.delta_pic_order_cnt[0] = bitstream_read_se(reader);

		SYNTAX_IPRINT("delta_pic_order_cnt[0] = %d\n",
			      decoder->sh.delta_pic_order_cnt[0]);

		if (pps->bottom_field_pic_order_in_frame_present_flag &&
			!decoder->sh.field_pic_flag)
		{
			decoder->sh.delta_pic_order_cnt[1] = bitstream_read_se(reader);

			SYNTAX_IPRINT("delta_pic_order_cnt[1] = %d\n",
				      decoder->sh.delta_pic_order_cnt[1]);
		}
	}

	if (pps->redundant_pic_cnt_present_flag) {
		decoder->sh.redundant_pic_cnt = bitstream_read_ue(reader);

		SYNTAX_IPRINT("redundant_pic_cnt = %u\n",
			      decoder->sh.redundant_pic_cnt);
	}

	switch (decoder->sh.slice_type) {
	case B:
		decoder->sh.direct_spatial_mv_pred_flag =
						bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("direct_spatial_mv_pred_flag = %u\n",
			      decoder->sh.direct_spatial_mv_pred_flag);
	case P:
	case SP:
		decoder->sh.num_ref_idx_active_override_flag =
						bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("num_ref_idx_active_override_flag = %u\n",
			      decoder->sh.num_ref_idx_active_override_flag);

		if (!decoder->sh.num_ref_idx_active_override_flag) {
			break;
		}

		decoder->sh.num_ref_idx_l0_active_minus1 =
						bitstream_read_ue(reader);

		SYNTAX_IPRINT("num_ref_idx_l0_active_minus1 = %u\n",
			      decoder->sh.num_ref_idx_l0_active_minus1);

		if (decoder->sh.slice_type != B) {
			break;
		}

		decoder->sh.num_ref_idx_l1_active_minus1 =
						bitstream_read_ue(reader);

		SYNTAX_IPRINT("num_ref_idx_l1_active_minus1 = %u\n",
			      decoder->sh.num_ref_idx_l1_active_minus1);
		break;
	default:
		break;
	}

	// ref_pic_list_modification( )
	{
		unsigned ref_pic_list_modification_flag_l0;
		unsigned ref_pic_list_modification_flag_l1;
		uint32_t modification_of_pic_nums_idc;
		uint32_t abs_diff_pic_num_minus1;
		uint32_t long_term_pic_num;
		int predicted_picture = decoder->sh.frame_num;
		int remapped_picture;
		int refIdxL = 0;
		int l1 = 0;

		switch (decoder->sh.slice_type) {
		case P:
		case B:
		case SP:
			ref_pic_list_modification_flag_l0 = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("ref_pic_list_modification_flag_l0 = %u\n",
				      ref_pic_list_modification_flag_l0);

			if (!ref_pic_list_modification_flag_l0) {
				goto flag_l1;
			}
pics_num:
			modification_of_pic_nums_idc = bitstream_read_ue(reader);

			SYNTAX_IPRINT("modification_of_pic_nums_idc = %u\n",
				      modification_of_pic_nums_idc);

			switch (modification_of_pic_nums_idc) {
			case 0 ... 1:
				abs_diff_pic_num_minus1 = bitstream_read_ue(reader);

				SYNTAX_IPRINT("abs_diff_pic_num_minus1 = %u\n",
					      abs_diff_pic_num_minus1);

				if (modification_of_pic_nums_idc) {
					remapped_picture = predicted_picture + abs_diff_pic_num_minus1;

					SYNTAX_IPRINT("refIdxL%d <- %d\n", l1, remapped_picture);

					predicted_picture = remapped_picture;
				}
				goto pics_num;
			case 2:
				long_term_pic_num = bitstream_read_ue(reader);

				SYNTAX_IPRINT("long_term_pic_num = %u\n",
					      long_term_pic_num);
				goto pics_num;
			case 3:
				if (l1) {
					break;
				}
flag_l1:
				switch (decoder->sh.slice_type) {
				case B:
				case B_ONLY:
					ref_pic_list_modification_flag_l1 =
							bitstream_read_u(reader, 1);

					SYNTAX_IPRINT("ref_pic_list_modification_flag_l1 = %u\n",
						      ref_pic_list_modification_flag_l1);

					if (!ref_pic_list_modification_flag_l1) {
						break;
					}

					l1 = 1;
					goto pics_num;
				}
				break;
			default:
				SYNTAX_ERR("slice header is malformed\n");
			}
			break;
		default:
			break;
		}
	}

	switch (decoder->sh.slice_type) {
	case P_ONLY:
	case SP_ONLY:
		if (!pps->weighted_pred_flag) {
			break;
		}
		goto pred_weight_table;
	case B:
		if (!pps->weighted_bipred_idc) {
			break;
		}
pred_weight_table:
{
		pred_weight **pw;
		int sz, l = 0;

		decoder->sh.luma_log2_weight_denom = bitstream_read_ue(reader);

		SYNTAX_IPRINT("luma_log2_weight_denom = %u\n",
			      decoder->sh.luma_log2_weight_denom);

		if (ChromaArrayType() != 0) {
			decoder->sh.chroma_log2_weight_denom = bitstream_read_ue(reader);

			SYNTAX_IPRINT("chroma_log2_weight_denom = %u\n",
				      decoder->sh.chroma_log2_weight_denom);
		}
table_fill:
		pw = l ? &decoder->sh.pred_weight_l1 : &decoder->sh.pred_weight_l0;
		sz = (l ? decoder->sh.num_ref_idx_l1_active_minus1 : decoder->sh.num_ref_idx_l0_active_minus1) + 1;
		*pw = realloc(*pw, sizeof(pred_weight) * sz);

		assert(*pw != NULL);

		for (i = 0; i < sz; i++) {
			(*pw)[i].luma_weight_l_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("luma_weight_l%d_flag[%d] = %u\n",
				      i, l, (*pw)[i].luma_weight_l_flag);

			if ((*pw)[i].luma_weight_l_flag) {
				(*pw)[i].luma_weight_l = bitstream_read_se(reader);

				SYNTAX_IPRINT("luma_weight_l%d[%d] = %u\n",
					      i, l, (*pw)[i].luma_weight_l);
			}
		}

		if (decoder->sh.slice_type != B || l == 1) {
			break;
		}

		l = 1;
		goto table_fill;
}
	default:
		break;
	}

	if (decoder->nal.ref_idc != 0) {
		if (IdrPicFlag) {
			decoder->sh.no_output_of_prior_pics_flag =
						bitstream_read_u(reader, 1);
			decoder->sh.long_term_reference_flag =
						bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("no_output_of_prior_pics_flag = %u\n",
				      decoder->sh.no_output_of_prior_pics_flag);
			SYNTAX_IPRINT("long_term_reference_flag = %u\n",
				      decoder->sh.long_term_reference_flag);
		} else {
			unsigned adaptive_ref_pic_marking_mode_flag;
			uint32_t memory_management_control_operation;
			uint32_t difference_of_pic_nums_minus1;
			uint32_t long_term_pic_num;
			uint32_t long_term_frame_idx;
			uint32_t max_long_term_frame_idx_plus1;

			adaptive_ref_pic_marking_mode_flag =
						bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("adaptive_ref_pic_marking_mode_flag = %u\n",
				      adaptive_ref_pic_marking_mode_flag);

			if (adaptive_ref_pic_marking_mode_flag) {
				do {
					memory_management_control_operation =
								bitstream_read_ue(reader);

					SYNTAX_IPRINT("memory_management_control_operation = %u\n",
						      memory_management_control_operation);

					switch (memory_management_control_operation) {
					case 1:
					case 3:
						difference_of_pic_nums_minus1 =
								bitstream_read_ue(reader);

						SYNTAX_IPRINT("difference_of_pic_nums_minus1 = %u\n",
							      difference_of_pic_nums_minus1);

						if (memory_management_control_operation == 3) {
							goto long_term_frame_idx__;
						}
						break;
					case 2:
						long_term_pic_num =
								bitstream_read_ue(reader);

						SYNTAX_IPRINT("long_term_pic_num = %u\n",
							      long_term_pic_num);
						break;
long_term_frame_idx__:			case 6:
						long_term_frame_idx =
								bitstream_read_ue(reader);

						SYNTAX_IPRINT("long_term_frame_idx = %u\n",
							      long_term_frame_idx);
						break;
					case 4:
						max_long_term_frame_idx_plus1 =
								bitstream_read_ue(reader);

						SYNTAX_IPRINT("max_long_term_frame_idx_plus1 = %u\n",
							      max_long_term_frame_idx_plus1);
						break;
					case 0:
						break;
					default:
						SYNTAX_ERR("memory_management_control_operation is malformed\n");
						break;
					}
				} while (memory_management_control_operation != 0);
			}
		}
	}

	switch (decoder->sh.slice_type) {
	case I:
	case SI:
		break;
	default:
		if (!CABAC_MODE) {
			break;
		}

		decoder->sh.cabac_init_idc = bitstream_read_ue(reader);

		SYNTAX_IPRINT("cabac_init_idc = %u\n", decoder->sh.cabac_init_idc);
	}

	decoder->sh.slice_qp_delta = bitstream_read_se(reader);

	SYNTAX_IPRINT("slice_qp_delta = %d\n", decoder->sh.slice_qp_delta);

	switch (decoder->sh.slice_type) {
	case SP:
		decoder->sh.sp_for_switch_flag = bitstream_read_u(reader, 1);

		SYNTAX_IPRINT("sp_for_switch_flag = %u\n",
			      decoder->sh.sp_for_switch_flag);
	case SI:
		decoder->sh.slice_qs_delta = bitstream_read_se(reader);

		SYNTAX_IPRINT("slice_qs_delta = %d\n",
			      decoder->sh.slice_qs_delta);
		break;
	default:
		break;
	}

	if (pps->deblocking_filter_control_present_flag) {
		decoder->sh.disable_deblocking_filter_idc = bitstream_read_ue(reader);

		SYNTAX_IPRINT("disable_deblocking_filter_idc = %u\n",
			      decoder->sh.disable_deblocking_filter_idc);

		if (decoder->sh.disable_deblocking_filter_idc != 1) {
			decoder->sh.slice_alpha_c0_offset_div2 =
						bitstream_read_se(reader);
			decoder->sh.slice_beta_offset_div2 =
						bitstream_read_se(reader);

			SYNTAX_IPRINT("slice_alpha_c0_offset_div2 = %d\n",
				      decoder->sh.slice_alpha_c0_offset_div2);
			SYNTAX_IPRINT("slice_beta_offset_div2 = %d\n",
				      decoder->sh.slice_beta_offset_div2);
		}
	}

	if (pps->num_slice_groups_minus1 > 0 &&
		pps->slice_group_map_type >= 3 && pps->slice_group_map_type <= 5)
	{
		int bits_nb = 32 - clz(pps->num_slice_groups_minus1 + 1);

		decoder->sh.slice_group_change_cycle =
						bitstream_read_u(reader, bits_nb);

		SYNTAX_IPRINT("slice_group_change_cycle = %u\n",
			decoder->sh.slice_group_change_cycle);
	}
}
