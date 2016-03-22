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

static signed get_luma_pred_mode_left(const decoder_context *decoder,
				      macroblock *mb, unsigned mb_id,
				      signed sub_mb_id)
{
	unsigned cur_pred_mode = MbPartPredMode(mb, I);
	unsigned left_pred_mode;
	macroblock *src_mb = mb;

	switch (cur_pred_mode) {
	case Intra_4x4:
		sub_mb_id = get_sub_id_4x4_left(decoder, &src_mb,
						mb_id, sub_mb_id);
		break;
	case Intra_8x8:
		sub_mb_id = get_sub_id_8x8_left(decoder, &src_mb,
						mb_id, sub_mb_id >> 2);
		break;
	case Intra_16x16:
		return 2;
	}

	if (sub_mb_id == MB_UNAVAILABLE) {
		return MB_UNAVAILABLE;
	}

	left_pred_mode = MbPartPredMode(src_mb, I);

	if (cur_pred_mode != left_pred_mode) {
		switch (left_pred_mode) {
		case Intra_4x4:
			sub_mb_id = 4 * sub_mb_id + 1;
			break;
		case Intra_8x8:
			sub_mb_id = sub_mb_id >> 2;
			break;
		case Intra_16x16:
			return 2;
		}
	}

	return src_mb->luma_pred_mode[sub_mb_id];
}

static signed get_luma_pred_mode_up(const decoder_context *decoder,
				    macroblock *mb,unsigned mb_id,
				    signed sub_mb_id)
{
	unsigned cur_pred_mode = MbPartPredMode(mb, I);
	unsigned up_pred_mode;
	macroblock *src_mb = mb;

	switch ( MbPartPredMode(src_mb, I) ) {
	case Intra_4x4:
		sub_mb_id = get_sub_id_4x4_up(decoder, &src_mb,
					      mb_id, sub_mb_id);
		break;
	case Intra_8x8:
		sub_mb_id = get_sub_id_8x8_up(decoder, &src_mb,
					      mb_id, sub_mb_id >> 2);
		break;
	case Intra_16x16:
		return 2;
	}

	if (sub_mb_id == MB_UNAVAILABLE) {
		return MB_UNAVAILABLE;
	}

	up_pred_mode = MbPartPredMode(src_mb, I);

	if (cur_pred_mode != up_pred_mode) {
		switch (up_pred_mode) {
		case Intra_4x4:
			sub_mb_id = 4 * sub_mb_id + 2;
			break;
		case Intra_8x8:
			sub_mb_id = sub_mb_id >> 2;
			break;
		case Intra_16x16:
			return 2;
		}
	}

	return src_mb->luma_pred_mode[sub_mb_id];
}

void macroblock_prediction_mode_intra_4x4(const decoder_context *decoder,
					  macroblock *mb, unsigned mb_id)
{
	bitstream_reader *reader = (void *) &decoder->reader;
	int i;

	for (i = 0; i < 16; i++) {
		unsigned prev_pred_mode_flag = bitstream_read_u(reader, 1);
		unsigned rem_pred_mode = 0;
		unsigned PredMode = 2;
		signed pred_left = get_luma_pred_mode_left(decoder, mb, mb_id, i);
		signed pred_up;

		SYNTAX_DPRINT("prev_pred_mode_flag[%d] %u\n", i, prev_pred_mode_flag);

		if (!prev_pred_mode_flag) {
			rem_pred_mode = bitstream_read_u(reader, 3);

			SYNTAX_DPRINT("rem_pred_mode[%d] %u\n", i, rem_pred_mode);
		}

		if (pred_left != MB_UNAVAILABLE) {
			pred_up = get_luma_pred_mode_up(decoder, mb, mb_id, i);
		}

		if (pred_left != MB_UNAVAILABLE && pred_up != MB_UNAVAILABLE) {
			PredMode = min(pred_left, pred_up);
		}

		if (prev_pred_mode_flag) {
			mb->luma_pred_mode[i] = PredMode;
		} else {
			if (rem_pred_mode >= PredMode) {
				rem_pred_mode++;
			}

			mb->luma_pred_mode[i] = rem_pred_mode;
		}

		SYNTAX_DPRINT("luma_pred_mode[%d] %u\n",
			      i, mb->luma_pred_mode[i]);
	}
}
