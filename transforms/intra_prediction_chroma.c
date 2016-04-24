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
	Intra_Chroma_DC,
	Intra_Chroma_Horizontal,
	Intra_Chroma_Vertical,
	Intra_Chroma_Plane,
};

static const int v_sub_id(int sub_mb_id)
{
	switch (sub_mb_id) {
	case 0:
	case 2:
		return 0;
	case 1:
	case 3:
		return 1;
	}

	return -1;
}

static const int h_sub_id(int sub_mb_id)
{
	switch (sub_mb_id) {
	case 0:
	case 1:
		return 0;
	case 2:
	case 3:
		return 2;
	}

	return -1;
}

void mb_apply_chroma_prediction(const decoder_context *decoder, int plane_id,
				unsigned mb_id, int16_t residual[16][16],
				unsigned pred_mode)
{
	macroblock *mb = &decoder->frames[0]->macroblocks[mb_id];
	macroblock *src_mb_left, *src_mb_up;
	int bit_depth = decoder->active_sps->bit_depth_chroma_minus8 + 8;
	uint8_t *chroma_decoded;
	int sub_mb, sub_mb_id_left, sub_mb_id_up;
	int H, V, a, b, c, M;
	uint8_t tmp[8];
	int pred;
	int i, j;

	TRANSFORM_DPRINT("intra_chroma_pred_mode %d\n", pred_mode);

	switch (pred_mode) {
	case Intra_Chroma_DC:
		for (sub_mb = 0; sub_mb < 4; sub_mb++) {
			src_mb_up = src_mb_left = NULL;
			sub_mb_id_left = MB_UNAVAILABLE;
			sub_mb_id_up = MB_UNAVAILABLE;
			pred = 0;

			switch (sub_mb) {
			case 0:
			case 3:
				sub_mb_id_left = get_sub_id_2x2_left(decoder, &src_mb_left, mb_id, h_sub_id(sub_mb));
				sub_mb_id_up = get_sub_id_2x2_up(decoder, &src_mb_up, mb_id, v_sub_id(sub_mb));
				break;
			case 1:
				sub_mb_id_up = get_sub_id_2x2_up(decoder, &src_mb_up, mb_id, v_sub_id(sub_mb));

				if (sub_mb_id_up == MB_UNAVAILABLE) {
					sub_mb_id_left = get_sub_id_2x2_left(decoder, &src_mb_left, mb_id, h_sub_id(sub_mb));
				}
				break;
			case 2:
				sub_mb_id_left = get_sub_id_2x2_left(decoder, &src_mb_left, mb_id, h_sub_id(sub_mb));

				if (sub_mb_id_left == MB_UNAVAILABLE) {
					sub_mb_id_up = get_sub_id_2x2_up(decoder, &src_mb_up, mb_id, v_sub_id(sub_mb));
				}
				break;
			}

			if (sub_mb_id_left != MB_UNAVAILABLE) {
				SETUP_PLANE_PTR(chroma_decoded, plane_id,src_mb_left, sub_mb_id_left);

				pred += chroma_decoded[3];
				pred += chroma_decoded[7];
				pred += chroma_decoded[11];
				pred += chroma_decoded[15];
			}

			if (sub_mb_id_up != MB_UNAVAILABLE) {
				SETUP_PLANE_PTR(chroma_decoded, plane_id,src_mb_up, sub_mb_id_up);

				pred += chroma_decoded[12];
				pred += chroma_decoded[13];
				pred += chroma_decoded[14];
				pred += chroma_decoded[15];
			}

			if (sub_mb_id_left != MB_UNAVAILABLE && sub_mb_id_up != MB_UNAVAILABLE) {
				pred = (pred + 4) >> 3;
			} else if (sub_mb_id_left != MB_UNAVAILABLE || sub_mb_id_up != MB_UNAVAILABLE) {
				pred = (pred + 2) >> 2;
			} else {
				pred = 1 << (bit_depth - 1);
			}

			SETUP_PLANE_PTR(chroma_decoded, plane_id,mb, sub_mb);

			for (i = 0; i < 16; i++) {
				chroma_decoded[i] = Clip1(pred + residual[sub_mb][i]);
			}
		}
		break;
	case Intra_Chroma_Horizontal:
		for (sub_mb = 0; sub_mb < 4; sub_mb++) {
			src_mb_left = NULL;
			sub_mb_id_left = get_sub_id_2x2_left(decoder, &src_mb_left, mb_id, h_sub_id(sub_mb));

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			SETUP_PLANE_PTR(chroma_decoded, plane_id,src_mb_left, sub_mb_id_left);

			tmp[0] = chroma_decoded[3];
			tmp[1] = chroma_decoded[7];
			tmp[2] = chroma_decoded[11];
			tmp[3] = chroma_decoded[15];

			SETUP_PLANE_PTR(chroma_decoded, plane_id,mb, sub_mb);

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					chroma_decoded[i * 4 + j] = Clip1(tmp[i] + residual[sub_mb][i * 4 + j]);
				}
			}
		}
		break;
	case Intra_Chroma_Vertical:
		for (sub_mb = 0; sub_mb < 4; sub_mb++) {
			src_mb_up = NULL;
			sub_mb_id_up = get_sub_id_2x2_up(decoder, &src_mb_up, mb_id, v_sub_id(sub_mb));

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			SETUP_PLANE_PTR(chroma_decoded, plane_id,src_mb_up, sub_mb_id_up);

			tmp[0] = chroma_decoded[12];
			tmp[1] = chroma_decoded[13];
			tmp[2] = chroma_decoded[14];
			tmp[3] = chroma_decoded[15];

			SETUP_PLANE_PTR(chroma_decoded, plane_id,mb, sub_mb);

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 4; j++) {
					chroma_decoded[i * 4 + j] = Clip1(tmp[j] + residual[sub_mb][i * 4 + j]);
				}
			}
		}
		break;
	case Intra_Chroma_Plane:
		src_mb_up = NULL;
		sub_mb_id_up = get_sub_id_2x2_left_up(decoder, &src_mb_up, mb_id, 0);

		assert(sub_mb_id_up != MB_UNAVAILABLE);

		SETUP_PLANE_PTR(chroma_decoded, plane_id,src_mb_up, sub_mb_id_up);

		M = chroma_decoded[15];

		for (i = 0, j = 0; i < 2; i++) {
			src_mb_up = NULL;
			sub_mb_id_up = get_sub_id_2x2_up(decoder, &src_mb_up, mb_id, i);

			assert(sub_mb_id_up != MB_UNAVAILABLE);

			SETUP_PLANE_PTR(chroma_decoded, plane_id,src_mb_up, sub_mb_id_up);

			tmp[j++] = chroma_decoded[12];
			tmp[j++] = chroma_decoded[13];
			tmp[j++] = chroma_decoded[14];
			tmp[j++] = chroma_decoded[15];
		}

		H = 4 * (tmp[7] - M);

		for (i = 0; i < 3; i++) {
			H += (i + 1) * (tmp[4 + i] - tmp[2 - i]);
		}

		a = tmp[7];

		for (i = 0, j = 0; i < 2; i++) {
			src_mb_left = NULL;
			sub_mb_id_left = get_sub_id_2x2_left(decoder, &src_mb_left, mb_id, i ? 2 : 0);

			assert(sub_mb_id_left != MB_UNAVAILABLE);

			SETUP_PLANE_PTR(chroma_decoded, plane_id,src_mb_left, sub_mb_id_left);

			tmp[j++] = chroma_decoded[3];
			tmp[j++] = chroma_decoded[7];
			tmp[j++] = chroma_decoded[11];
			tmp[j++] = chroma_decoded[15];
		}

		V = 4 * (tmp[7] - M);

		for (i = 0; i < 3; i++) {
			V += (i + 1) * (tmp[4 + i] - tmp[2 - i]);
		}

		a = 16 * (a + tmp[7]);

		b = (34 * H + 32) >> 6;

		c = (34 * V + 32) >> 6;

		TRANSFORM_DPRINT("H %d V %d a %d b %d c %d\n",
					H, V, a, b, c);

		for (j = 0; j < 4; j++) {
			int mb_offt_x, mb_offt_y;

			sub_mb = j;

			mb_offt_x = (j % 2) * 4;
			mb_offt_y = (j / 2) * 4;

			SETUP_PLANE_PTR(chroma_decoded, plane_id,mb, sub_mb);

			for (i = 0; i < 16; i++) {
				int x = mb_offt_x + i % 4;
				int y = mb_offt_y + i / 4;

				pred = Clip1((a + b * (x - 3) + c * (y - 3) + 16) >> 5);

				chroma_decoded[i] = Clip1(residual[sub_mb][i] + pred);
			}
		}
		break;
	default:
		assert(0);
		break;
	}
}
