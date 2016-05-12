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

#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include <sys/types.h>

#include "bitstream.h"

#define P	0
#define B	1
#define I	2
#define SP	3
#define SI	4
#define P_ONLY	5
#define B_ONLY	6
#define I_ONLY	7
#define SP_ONLY	8
#define SI_ONLY	9

#define DECODE_DPRINT(f, ...)	printf(f, ## __VA_ARGS__)

#define DECODE_ERR(f, ...)				\
{							\
	fprintf(stderr, "%s:%d:\n", __FILE__, __LINE__);\
	fprintf(stderr, "error! decode: %s: "		\
		f, __func__, ## __VA_ARGS__);		\
	exit(EXIT_FAILURE);				\
}

typedef struct decoder_context_sps {
	unsigned valid:1;

	unsigned profile_idc:8;
	unsigned constraint_set0_flag:1;
	unsigned constraint_set1_flag:1;
	unsigned constraint_set2_flag:1;
	unsigned constraint_set3_flag:1;
	unsigned constraint_set4_flag:1;
	unsigned constraint_set5_flag:1;
	unsigned level_idc:8;
	uint32_t chroma_format_idc;
	unsigned separate_colour_plane_flag:1;
	uint32_t bit_depth_luma_minus8;
	uint32_t bit_depth_chroma_minus8;
	unsigned qpprime_y_zero_transform_bypass_flag:1;
	unsigned seq_scaling_matrix_present_flag:1;
	unsigned seq_scaling_list_present_flag:12;
	uint32_t log2_max_frame_num_minus4;
	uint32_t pic_order_cnt_type;
	uint32_t log2_max_pic_order_cnt_lsb_minus4;
	unsigned delta_pic_order_always_zero_flag:1;
	int32_t  offset_for_non_ref_pic;
	int32_t  offset_for_top_to_bottom_field;
	uint32_t num_ref_frames_in_pic_order_cnt_cycle;
	int32_t  *offset_for_ref_frame;
	uint32_t max_num_ref_frames;
	unsigned gaps_in_frame_num_value_allowed_flag:1;
	uint32_t pic_width_in_mbs_minus1;
	uint32_t pic_height_in_map_units_minus1;
	unsigned frame_mbs_only_flag:1;
	unsigned mb_adaptive_frame_field_flag:1;
	unsigned direct_8x8_inference_flag:1;
	unsigned frame_cropping_flag:1;
	uint32_t frame_crop_left_offset;
	uint32_t frame_crop_right_offset;
	uint32_t frame_crop_top_offset;
	uint32_t frame_crop_bottom_offset;
	unsigned vui_parameters_present_flag:1;
	unsigned UseDefaultScalingMatrix4x4Flag[6];
	unsigned UseDefaultScalingMatrix8x8Flag[6];
	int8_t scalingList_4x4[6][16];
	int8_t scalingList_8x8[6][64];
} decoder_context_sps;

typedef struct decoder_context_pps {
	unsigned valid:1;

	uint8_t seq_parameter_set_id;
	unsigned entropy_coding_mode_flag:1;
	unsigned bottom_field_pic_order_in_frame_present_flag:1;
	uint32_t num_slice_groups_minus1;
	uint32_t slice_group_map_type;
	uint32_t *run_length_minus1;
	uint32_t *top_left;
	uint32_t *bottom_right;
	unsigned slice_group_change_direction_flag:1;
	uint32_t slice_group_change_rate_minus1;
	uint32_t pic_size_in_map_units_minus1;
	uint32_t *slice_group_id;
	uint32_t num_ref_idx_l0_default_active_minus1;
	uint32_t num_ref_idx_l1_default_active_minus1;
	unsigned weighted_pred_flag:1;
	unsigned weighted_bipred_idc:2;
	int32_t  pic_init_qp_minus26;
	int32_t  pic_init_qs_minus26;
	int32_t  chroma_qp_index_offset;
	unsigned deblocking_filter_control_present_flag:1;
	unsigned constrained_intra_pred_flag:1;
	unsigned redundant_pic_cnt_present_flag:1;
	unsigned transform_8x8_mode_flag:1;
	unsigned pic_scaling_matrix_present_flag:1;
	unsigned UseDefaultScalingMatrix4x4Flag[6];
	unsigned UseDefaultScalingMatrix8x8Flag[6];
	int8_t   scalingList_4x4[6][16];
	int8_t   scalingList_8x8[6][64];
	int32_t  second_chroma_qp_index_offset;
} decoder_context_pps;

typedef struct pred_weight {
	unsigned luma_weight_l_flag:1;
	int32_t  luma_weight_l;
	int32_t  luma_offset_l;
	unsigned chroma_weight_l_flag:1;
	int32_t  chroma_weight_l[2];
	int32_t  chroma_offset_l[2];
} pred_weight;

typedef struct slice_header {
	uint32_t first_mb_in_slice;
	uint32_t slice_type;
	unsigned colour_plane_id:2;
	uint32_t frame_num;
	unsigned field_pic_flag:1;
	unsigned bottom_field_flag:1;
	uint32_t idr_pic_id;
	uint32_t pic_order_cnt_lsb;
	int32_t  delta_pic_order_cnt_bottom;
	int32_t  delta_pic_order_cnt[2];
	uint32_t redundant_pic_cnt;
	unsigned direct_spatial_mv_pred_flag:1;
	unsigned num_ref_idx_active_override_flag:1;
	uint32_t num_ref_idx_l0_active_minus1;
	uint32_t num_ref_idx_l1_active_minus1;
	uint32_t cabac_init_idc;
	int32_t  slice_qp_delta;
	unsigned sp_for_switch_flag:1;
	int32_t  slice_qs_delta;
	uint32_t disable_deblocking_filter_idc;
	int32_t  slice_alpha_c0_offset_div2;
	int32_t  slice_beta_offset_div2;
	uint32_t slice_group_change_cycle;
	uint32_t luma_log2_weight_denom;
	uint32_t chroma_log2_weight_denom;
	pred_weight *pred_weight_l0;
	pred_weight *pred_weight_l1;
	unsigned no_output_of_prior_pics_flag:1;
	unsigned long_term_reference_flag:1;
} slice_header;

typedef struct macro_sub_block {
	uint8_t totalcoeff;
	int16_t coeffs[16];
} macro_sub_block;

typedef struct macroblock {
	unsigned mb_type:5;

	unsigned slice_type:4;

	unsigned transform_size_8x8_flag:1;

	signed   mb_qp_delta:6;

	uint8_t luma_has_transform_coeffs[16];

	int QP_Y, QPcb, QPcr;

	union {
		struct {
			unsigned luma_pred_mode[16];
			macro_sub_block luma_DC;
			macro_sub_block luma_AC[16];
		};

		uint8_t luma_decoded[16][16];
	};

	union {
		struct {
			unsigned intra_chroma_pred_mode:2;
			macro_sub_block chroma_U_DC;
			macro_sub_block chroma_U_AC[4];
		};

		uint8_t chroma_U_decoded[4][16];
	};

	union {
		struct {
			macro_sub_block chroma_V_DC;
			macro_sub_block chroma_V_AC[4];
		};

		uint8_t chroma_V_decoded[4][16];
	};

} macroblock;

typedef struct slice_data {
	unsigned cabac_alignment_one_bit:1;
	uint32_t mb_skip_run;
	unsigned mb_skip_flag:1;
	unsigned mb_field_decoding_flag:1;
	unsigned end_of_slice_flag:1;
	macroblock *macroblocks;
} slice_data;

typedef struct nal_header {
	unsigned ref_idc:2;
	unsigned unit_type:5;
} nal_header;

typedef struct frame_data {
	macroblock *macroblocks;
} frame_data;

typedef struct decoder_context {
	bitstream_reader reader;

	void (*frame_decoded_notify)(struct decoder_context*);
	void *opaque;

	uint8_t *decoded_image;

	decoder_context_sps sps[32];
	decoder_context_pps pps[256];

	decoder_context_sps *active_sps;
	decoder_context_pps *active_pps;

	nal_header   nal;
	slice_header sh;
	slice_data   sd;

	frame_data *frames[1 + 16];

	int get_mb_slice_constraint;

	int NAL_start_delim;
} decoder_context;

void decoder_init(decoder_context *decoder, void *data, uint32_t size);

void decoder_set_notify(decoder_context *decoder,
			void (*frame_decoded_notify)(struct decoder_context*),
			void *opaque);

void decode_current_slice(decoder_context *decoder, unsigned last_mb_id);

size_t decoder_image_frame_size(decoder_context *decoder);

void mb_apply_deblocking(decoder_context *decoder, unsigned mb_id);

#endif // DECODER_H
