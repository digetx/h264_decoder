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

#include "syntax_parse.h"

#include "common.h"

enum {
	Intra_4x4_Vertical,
	Intra_4x4_Horizontal,
	Intra_4x4_DC,
	Intra_4x4_Diagonal_Down_Left,
	Intra_4x4_Diagonal_Down_Right,
	Intra_4x4_Vertical_Right,
	Intra_4x4_Horizontal_Down,
	Intra_4x4_Vertical_Left,
	Intra_4x4_Horizontal_Up,
};

void mb_apply_luma_prediction_4x4(const decoder_context *decoder,
				  unsigned mb_id, int16_t residual[16][16])
{
	unsigned mb_id_in_slice = mb_id - decoder->sh.first_mb_in_slice;
	macroblock *mb = &decoder->sd.macroblocks[mb_id_in_slice];
	macroblock *src_mb_left, *src_mb_up;
	int bit_depth = decoder->active_sps->bit_depth_luma_minus8 + 8;
	int sub_mb, sub_mb_id_left, sub_mb_id_up;
	unsigned luma_pred_mode[16];
	uint8_t tmp[9];
	int pred;
	int i, j;

	for (sub_mb = 0; sub_mb < 16; sub_mb++) {
		luma_pred_mode[sub_mb] = mb->luma_pred_mode[sub_mb];
	}

	for (sub_mb = 0; sub_mb < 16; sub_mb++) {
		src_mb_left = mb;
		src_mb_up = mb;

		TRANSFORM_DPRINT("luma_pred_mode[%d] %d\n",
				 sub_mb, luma_pred_mode[sub_mb]);

		switch (luma_pred_mode[sub_mb]) {
		case Intra_4x4_Vertical:
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[0] = src_mb_up->luma_decoded[sub_mb_id_up][12];
			tmp[1] = src_mb_up->luma_decoded[sub_mb_id_up][13];
			tmp[2] = src_mb_up->luma_decoded[sub_mb_id_up][14];
			tmp[3] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			for (i = 0; i < 4; i++) {
				mb->luma_decoded[sub_mb][i * 4 + 0] = Clip1(residual[sub_mb][i * 4 + 0] + tmp[0]);
				mb->luma_decoded[sub_mb][i * 4 + 1] = Clip1(residual[sub_mb][i * 4 + 1] + tmp[1]);
				mb->luma_decoded[sub_mb][i * 4 + 2] = Clip1(residual[sub_mb][i * 4 + 2] + tmp[2]);
				mb->luma_decoded[sub_mb][i * 4 + 3] = Clip1(residual[sub_mb][i * 4 + 3] + tmp[3]);
			}
			break;
		case Intra_4x4_Horizontal:
			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, sub_mb);

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			tmp[0] = src_mb_left->luma_decoded[sub_mb_id_left][3];
			tmp[1] = src_mb_left->luma_decoded[sub_mb_id_left][7];
			tmp[2] = src_mb_left->luma_decoded[sub_mb_id_left][11];
			tmp[3] = src_mb_left->luma_decoded[sub_mb_id_left][15];

			for (i = 0; i < 4; i++) {
				mb->luma_decoded[sub_mb][i * 4 + 0] = Clip1(residual[sub_mb][i * 4 + 0] + tmp[i]);
				mb->luma_decoded[sub_mb][i * 4 + 1] = Clip1(residual[sub_mb][i * 4 + 1] + tmp[i]);
				mb->luma_decoded[sub_mb][i * 4 + 2] = Clip1(residual[sub_mb][i * 4 + 2] + tmp[i]);
				mb->luma_decoded[sub_mb][i * 4 + 3] = Clip1(residual[sub_mb][i * 4 + 3] + tmp[i]);
			}
			break;
		case Intra_4x4_DC:
			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, sub_mb);
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb);
			pred = 0;

			if (sub_mb_id_left != MB_UNAVAILABLE) {
				pred += src_mb_left->luma_decoded[sub_mb_id_left][3];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][7];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][11];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][15];
			}

			if (sub_mb_id_up != MB_UNAVAILABLE) {
				pred += src_mb_up->luma_decoded[sub_mb_id_up][12];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][13];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][14];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][15];
			}

			if (sub_mb_id_left != MB_UNAVAILABLE && sub_mb_id_up != MB_UNAVAILABLE) {
				pred = (pred + 4) >> 3;
			} else if (sub_mb_id_left != MB_UNAVAILABLE || sub_mb_id_up != MB_UNAVAILABLE) {
				pred = (pred + 2) >> 2;
			} else {
				pred = 1 << (bit_depth - 1);
			}

			for (i = 0; i < 16; i++) {
				mb->luma_decoded[sub_mb][i] = Clip1(pred + residual[sub_mb][i]);
			}
			break;
		case Intra_4x4_Diagonal_Down_Left:
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[0] = src_mb_up->luma_decoded[sub_mb_id_up][12];
			tmp[1] = src_mb_up->luma_decoded[sub_mb_id_up][13];
			tmp[2] = src_mb_up->luma_decoded[sub_mb_id_up][14];
			tmp[3] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			src_mb_up = mb;
			sub_mb_id_up = get_sub_id_4x4_up_right(decoder, &src_mb_up, mb_id, sub_mb);

			if (sub_mb_id_up != MB_UNAVAILABLE) {
				tmp[4] = src_mb_up->luma_decoded[sub_mb_id_up][12];
				tmp[5] = src_mb_up->luma_decoded[sub_mb_id_up][13];
				tmp[6] = src_mb_up->luma_decoded[sub_mb_id_up][14];
				tmp[7] = src_mb_up->luma_decoded[sub_mb_id_up][15];
			} else {
				tmp[4] = tmp[5] = tmp[6] = tmp[7] = tmp[3];
			}

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (i == 3 && j == 3) {
						pred = tmp[6] + 3 * tmp[7];
					} else {
						pred = tmp[i + j] + 2 * tmp[i + j + 1] + tmp[i + j + 2];
					}

					pred = (pred + 2) >> 2;

					mb->luma_decoded[sub_mb][i * 4 + j] = Clip1(pred + residual[sub_mb][i * 4 + j]);
				}
			}
			break;
		case Intra_4x4_Diagonal_Down_Right:
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[0] = src_mb_up->luma_decoded[sub_mb_id_up][12];
			tmp[1] = src_mb_up->luma_decoded[sub_mb_id_up][13];
			tmp[2] = src_mb_up->luma_decoded[sub_mb_id_up][14];
			tmp[3] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, sub_mb);

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			tmp[4] = src_mb_left->luma_decoded[sub_mb_id_left][3];
			tmp[5] = src_mb_left->luma_decoded[sub_mb_id_left][7];
			tmp[6] = src_mb_left->luma_decoded[sub_mb_id_left][11];
			tmp[7] = src_mb_left->luma_decoded[sub_mb_id_left][15];

			src_mb_up = mb;
			sub_mb_id_up = get_sub_id_4x4_left_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[8] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (j > i) {
						if (j - i - 2 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[j - i - 2];
						}
						pred += 2 * tmp[j - i - 1] + tmp[j - i];
					} else if (j < i) {
						if (i - j - 2 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[4 + i - j - 2 ];
						}
						pred += 2 * tmp[4 + i - j - 1] + tmp[4 + i - j];
					} else {
						pred = tmp[0] + 2 * tmp[8] + tmp[4];
					}

					pred = (pred + 2) >> 2;

					mb->luma_decoded[sub_mb][i * 4 + j] = Clip1(pred + residual[sub_mb][i * 4 + j]);
				}
			}
			break;
		case Intra_4x4_Vertical_Right:
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[0] = src_mb_up->luma_decoded[sub_mb_id_up][12];
			tmp[1] = src_mb_up->luma_decoded[sub_mb_id_up][13];
			tmp[2] = src_mb_up->luma_decoded[sub_mb_id_up][14];
			tmp[3] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, sub_mb);

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			tmp[4] = src_mb_left->luma_decoded[sub_mb_id_left][3];
			tmp[5] = src_mb_left->luma_decoded[sub_mb_id_left][7];
			tmp[6] = src_mb_left->luma_decoded[sub_mb_id_left][11];
			tmp[7] = src_mb_left->luma_decoded[sub_mb_id_left][15];

			src_mb_up = mb;
			sub_mb_id_up = get_sub_id_4x4_left_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[8] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					int zVR = 2 * j - i;

					switch (zVR) {
					case 0:
					case 2:
					case 4:
					case 6:
						if (j - (i >> 1) - 1 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[j - (i >> 1) - 1];
						}
						pred = (pred + tmp[j - (i >> 1)] + 1) >> 1;
						break;
					case 1:
					case 3:
					case 5:
						if (j - (i >> 1) - 2 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[j - (i >> 1) - 2];
						}
						pred = (pred + 2 * tmp[j - (i >> 1) - 1] + tmp[j - (i >> 1)] + 2) >> 2;
						break;
					case -1:
						pred = (tmp[4] + 2 * tmp[8] + tmp[0] + 2) >> 2;
						break;
					default:
						if (i - 3 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[4 + i - 3];
						}
						pred = (tmp[4 + i - 1] + 2 * tmp[4 + i - 2] + pred + 2) >> 2;
						break;
					}

					mb->luma_decoded[sub_mb][i * 4 + j] = Clip1(pred + residual[sub_mb][i * 4 + j]);
				}
			}
			break;
		case Intra_4x4_Horizontal_Down:
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[0] = src_mb_up->luma_decoded[sub_mb_id_up][12];
			tmp[1] = src_mb_up->luma_decoded[sub_mb_id_up][13];
			tmp[2] = src_mb_up->luma_decoded[sub_mb_id_up][14];
			tmp[3] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, sub_mb);

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			tmp[4] = src_mb_left->luma_decoded[sub_mb_id_left][3];
			tmp[5] = src_mb_left->luma_decoded[sub_mb_id_left][7];
			tmp[6] = src_mb_left->luma_decoded[sub_mb_id_left][11];
			tmp[7] = src_mb_left->luma_decoded[sub_mb_id_left][15];

			src_mb_up = mb;
			sub_mb_id_up = get_sub_id_4x4_left_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[8] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					int zHD = 2 * i - j;

					switch (zHD) {
					case 0:
					case 2:
					case 4:
					case 6:
						if (i - (j >> 1) - 1 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[4 + i - (j >> 1) - 1];
						}
						pred = (pred + tmp[4 + i - (j >> 1)] + 1) >> 1;
						break;
					case 1:
					case 3:
					case 5:
						if (i - (j >> 1) - 2 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[4 + i - (j >> 1) - 2];
						}
						pred = (pred + 2 * tmp[4 + i - (j >> 1) - 1] + tmp[4 + i - (j >> 1)] + 2) >> 2;
						break;
					case -1:
						pred = (tmp[0] + 2 * tmp[8] + tmp[4] + 2) >> 2;
						break;
					default:
						if (j - 3 == -1) {
							pred = tmp[8];
						} else {
							pred = tmp[j - 3];
						}
						pred = (tmp[j - 1] + 2 * tmp[j - 2] + pred + 2) >> 2;
						break;
					}

					mb->luma_decoded[sub_mb][i * 4 + j] = Clip1(pred + residual[sub_mb][i * 4 + j]);
				}
			}
			break;
		case Intra_4x4_Vertical_Left:
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[0] = src_mb_up->luma_decoded[sub_mb_id_up][12];
			tmp[1] = src_mb_up->luma_decoded[sub_mb_id_up][13];
			tmp[2] = src_mb_up->luma_decoded[sub_mb_id_up][14];
			tmp[3] = src_mb_up->luma_decoded[sub_mb_id_up][15];

			src_mb_up = mb;
			sub_mb_id_up = get_sub_id_4x4_up_right(decoder, &src_mb_up, mb_id, sub_mb);

			if (sub_mb_id_up != MB_UNAVAILABLE) {
				tmp[4] = src_mb_up->luma_decoded[sub_mb_id_up][12];
				tmp[5] = src_mb_up->luma_decoded[sub_mb_id_up][13];
				tmp[6] = src_mb_up->luma_decoded[sub_mb_id_up][14];
				tmp[7] = src_mb_up->luma_decoded[sub_mb_id_up][15];
			} else {
				tmp[4] = tmp[5] = tmp[6] = tmp[7] = tmp[3];
			}

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					if (i == 0 || i == 2) {
						pred = (tmp[j + (i >> 1)] + tmp[j + (i >> 1) + 1] + 1) >> 1;
					} else {
						pred = (tmp[j + (i >> 1)] + 2 * tmp[j + (i >> 1) + 1] + tmp[j + (i >> 1) + 2] + 2 ) >> 2;
					}

					mb->luma_decoded[sub_mb][i * 4 + j] = Clip1(pred + residual[sub_mb][i * 4 + j]);
				}
			}
			break;
		case Intra_4x4_Horizontal_Up:
			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, sub_mb);

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			tmp[0] = src_mb_left->luma_decoded[sub_mb_id_left][3];
			tmp[1] = src_mb_left->luma_decoded[sub_mb_id_left][7];
			tmp[2] = src_mb_left->luma_decoded[sub_mb_id_left][11];
			tmp[3] = src_mb_left->luma_decoded[sub_mb_id_left][15];

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					int zHU = 2 * i + j;

					switch (zHU) {
					case 0:
					case 2:
					case 4:
						pred = (tmp[i + (j >> 1)] + tmp[i + (j >> 1) + 1] + 1) >> 1;
						break;
					case 1:
					case 3:
						pred = (tmp[i + (j >> 1)] + 2 * tmp[i + (j >> 1) + 1] + tmp[i + (j >> 1) + 2] + 2) >> 2;
						break;
					case 5:
						pred = (tmp[2] + 3 * tmp[3] + 2) >> 2;
						break;
					default:
						pred = tmp[3];
						break;
					}

					mb->luma_decoded[sub_mb][i * 4 + j] = Clip1(pred + residual[sub_mb][i * 4 + j]);
				}
			}
			break;
		default:
			assert(0);
			break;
		}
	}
}
