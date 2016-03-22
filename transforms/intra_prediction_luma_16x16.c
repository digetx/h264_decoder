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
	Intra_16x16_Vertical,
	Intra_16x16_Horizontal,
	Intra_16x16_DC,
	Intra_16x16_Plane,
};

static const int v_sub_id(int sub_mb_id)
{
	switch (sub_mb_id) {
	case 0:
	case 2:
	case 8:
	case 10:
		return 0;
	case 1:
	case 3:
	case 9:
	case 11:
		return 1;
	case 4:
	case 6:
	case 12:
	case 14:
		return 4;
	case 5:
	case 7:
	case 13:
	case 15:
		return 5;
	}

	return -1;
}

static const int h_sub_id(int sub_mb_id)
{
	switch (sub_mb_id) {
	case 0:
	case 1:
	case 4:
	case 5:
		return 0;
	case 2:
	case 3:
	case 6:
	case 7:
		return 2;
	case 8:
	case 9:
	case 12:
	case 13:
		return 8;
	case 10:
	case 11:
	case 14:
	case 15:
		return 10;
	}

	return -1;
}

void mb_apply_luma_prediction_16x16(const decoder_context *decoder,
				    unsigned mb_id, int Intra16x16PredMode,
				    int16_t residual[16][16])
{
	unsigned mb_id_in_slice = mb_id - decoder->sh.first_mb_in_slice;
	macroblock *mb = &decoder->sd.macroblocks[mb_id_in_slice];
	macroblock *src_mb_left, *src_mb_up;
	int bit_depth = decoder->sps.bit_depth_luma_minus8 + 8;
	int sub_mb, sub_mb_id_left, sub_mb_id_up;
	int left_avail, up_avail;
	int H, V, a, b, c, M;
	uint8_t tmp[16];
	int pred;
	int i, j;

	TRANSFORM_DPRINT("Intra16x16PredMode %d\n", Intra16x16PredMode);

	switch (Intra16x16PredMode) {
	case Intra_16x16_Vertical:
		for (sub_mb = 0; sub_mb < 16; sub_mb++) {
			src_mb_up = NULL;
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, v_sub_id(sub_mb));

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
		}
		break;
	case Intra_16x16_Horizontal:
		for (sub_mb = 0; sub_mb < 16; sub_mb++) {
			src_mb_left = NULL;
			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, h_sub_id(sub_mb));

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
		}
		break;
	case Intra_16x16_DC:
		left_avail = get_mb_id_left(decoder, mb_id);
		up_avail = get_mb_id_up(decoder, mb_id);
		pred = 0;

		if (left_avail != MB_UNAVAILABLE && up_avail!= MB_UNAVAILABLE) {
			for (i = 0; i < 4; i++) {
				src_mb_up = NULL;
				sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, v_sub_id( mb_scan_map(i) ));

				assert(sub_mb_id_up != MB_UNAVAILABLE);

				pred += src_mb_up->luma_decoded[sub_mb_id_up][12];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][13];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][14];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][15];
			}

			for (i = 0; i < 4; i++) {
				src_mb_left = NULL;
				sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, h_sub_id( mb_scan_map(i * 4) ));

				assert(sub_mb_id_left != MB_UNAVAILABLE);

				pred += src_mb_left->luma_decoded[sub_mb_id_left][3];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][7];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][11];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][15];
			}

			pred = (pred + 16) >> 5;
		} else if (left_avail != MB_UNAVAILABLE) {
			for (i = 0; i < 4; i++) {
				src_mb_left = NULL;
				sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, h_sub_id( mb_scan_map(i * 4) ));

				assert(sub_mb_id_left != MB_UNAVAILABLE);

				pred += src_mb_left->luma_decoded[sub_mb_id_left][3];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][7];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][11];
				pred += src_mb_left->luma_decoded[sub_mb_id_left][15];
			}

			pred = (pred + 8) >> 4;
		} else if (up_avail != MB_UNAVAILABLE) {
			for (i = 0; i < 4; i++) {
				src_mb_up = NULL;
				sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, v_sub_id( mb_scan_map(i) ));

				assert(sub_mb_id_up != MB_UNAVAILABLE);

				pred += src_mb_up->luma_decoded[sub_mb_id_up][12];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][13];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][14];
				pred += src_mb_up->luma_decoded[sub_mb_id_up][15];
			}

			pred = (pred + 8) >> 4;
		} else {
			pred = 1 << (bit_depth - 1);
		}

		for (sub_mb = 0; sub_mb < 16; sub_mb++) {
			for (i = 0; i < 16; i++) {
				mb->luma_decoded[sub_mb][i] = Clip1(residual[sub_mb][i] + pred);
			}
		}
		break;
	case Intra_16x16_Plane:
		src_mb_up = NULL;
		sub_mb_id_up = get_sub_id_4x4_left_up(decoder, &src_mb_up, mb_id, 0);

		assert(sub_mb_id_up != MB_UNAVAILABLE);

		M = src_mb_up->luma_decoded[sub_mb_id_up][15];

		for (i = 0, j = 0; i < 4; i++) {
			src_mb_up = NULL;
			sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, v_sub_id( mb_scan_map(i) ));

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			tmp[j++] = src_mb_up->luma_decoded[sub_mb_id_up][12];
			tmp[j++] = src_mb_up->luma_decoded[sub_mb_id_up][13];
			tmp[j++] = src_mb_up->luma_decoded[sub_mb_id_up][14];
			tmp[j++] = src_mb_up->luma_decoded[sub_mb_id_up][15];
		}

		H = 8 * (tmp[15] - M);

		for (i = 0; i < 7; i++) {
			H += (i + 1) * (tmp[8 + i] - tmp[6 - i]);
		}

		a = tmp[15];

		for (i = 0, j = 0; i < 4; i++) {
			src_mb_left = NULL;
			sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left, mb_id, h_sub_id( mb_scan_map(i * 4) ));

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			tmp[j++] = src_mb_left->luma_decoded[sub_mb_id_left][3];
			tmp[j++] = src_mb_left->luma_decoded[sub_mb_id_left][7];
			tmp[j++] = src_mb_left->luma_decoded[sub_mb_id_left][11];
			tmp[j++] = src_mb_left->luma_decoded[sub_mb_id_left][15];
		}

		V = 8 * (tmp[15] - M);

		for (i = 0; i < 7; i++) {
			V += (i + 1) * (tmp[8 + i] - tmp[6 - i]);
		}

		a = 16 * (a + tmp[15]);

		b = (5 * H + 32) >> 6;

		c = (5 * V + 32) >> 6;

		TRANSFORM_DPRINT("H %d V %d a %d b %d c %d\n", H, V, a, b, c);

		for (j = 0; j < 16; j++) {
			int mb_offt_x, mb_offt_y;

			sub_mb = mb_scan_map(j);

			mb_offt_x = (j % 4) * 4;
			mb_offt_y = (j / 4) * 4;

			for (i = 0; i < 16; i++) {
				int x = mb_offt_x + i % 4;
				int y = mb_offt_y + i / 4;

				pred = Clip1((a + b * (x - 7) + c * (y - 7) + 16) >> 5);

				mb->luma_decoded[sub_mb][i] = Clip1(residual[sub_mb][i] + pred);
			}
		}
		break;
	default:
		assert(0);
		break;
	}
}
