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

const int mb_scan_map(int id)
{
	switch (id) {
	case 0:  return 0;
	case 1:  return 1;
	case 2:  return 4;
	case 3:  return 5;
	case 4:  return 2;
	case 5:  return 3;
	case 6:  return 6;
	case 7:  return 7;
	case 8:  return 8;
	case 9:  return 9;
	case 10: return 12;
	case 11: return 13;
	case 12: return 10;
	case 13: return 11;
	case 14: return 14;
	case 15: return 15;
	}

	return -1;
}

signed get_mb_id_left(const decoder_context *decoder, unsigned mb_id)
{
	const decoder_context_sps *sps = decoder->active_sps;
	unsigned first_mb_in_slice = decoder->sh.first_mb_in_slice;
	unsigned width = sps->pic_width_in_mbs_minus1 + 1;
	signed left_id = mb_id - 1;

	if (mb_id % width == 0) {
		return MB_UNAVAILABLE;
	}

	if (left_id < first_mb_in_slice) {
		return MB_UNAVAILABLE;
	}

	return left_id - first_mb_in_slice;
}

signed get_mb_id_up(const decoder_context *decoder, unsigned mb_id)
{
	const decoder_context_sps *sps = decoder->active_sps;
	unsigned first_mb_in_slice = decoder->sh.first_mb_in_slice;
	unsigned width = sps->pic_width_in_mbs_minus1 + 1;
	signed up_id = mb_id - width;

	if (up_id < 0) {
		return MB_UNAVAILABLE;
	}

	if (up_id < first_mb_in_slice) {
		return MB_UNAVAILABLE;
	}

	return up_id - first_mb_in_slice;
}

static signed get_mb_id_up_right(const decoder_context *decoder, unsigned mb_id)
{
	const decoder_context_sps *sps = decoder->active_sps;
	unsigned first_mb_in_slice = decoder->sh.first_mb_in_slice;
	unsigned width = sps->pic_width_in_mbs_minus1 + 1;
	signed up_right_id = mb_id - width + 1;

	if (up_right_id % width == 0) {
		return MB_UNAVAILABLE;
	}

	if (up_right_id < 0) {
		return MB_UNAVAILABLE;
	}

	if (up_right_id < first_mb_in_slice) {
		return MB_UNAVAILABLE;
	}

	return up_right_id - first_mb_in_slice;
}

static signed get_mb_id_left_up(const decoder_context *decoder, unsigned mb_id)
{
	const decoder_context_sps *sps = decoder->active_sps;
	unsigned first_mb_in_slice = decoder->sh.first_mb_in_slice;
	unsigned width = sps->pic_width_in_mbs_minus1 + 1;
	signed left_upt_id = mb_id - width - 1;

	if (left_upt_id < 0) {
		return MB_UNAVAILABLE;
	}

	if (left_upt_id < first_mb_in_slice) {
		return MB_UNAVAILABLE;
	}

	return left_upt_id - first_mb_in_slice;
}

static unsigned get_sub_mb_id_left_4x4(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 5;
	case 1: return 0;
	case 2: return 7;
	case 3: return 2;
	case 4: return 1;
	case 5: return 4;
	case 6: return 3;
	case 7: return 6;
	case 8: return 13;
	case 9: return 8;
	case 10: return 15;
	case 11: return 10;
	case 12: return 9;
	case 13: return 12;
	case 14: return 11;
	case 15: return 14;
	default: break;
	}

	assert(0);

	return ~0;
}

static unsigned get_sub_mb_id_left_8x8(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 1;
	case 1: return 0;
	case 2: return 3;
	case 3: return 2;
	default: break;
	}

	assert(0);

	return ~0;
}

static unsigned get_sub_mb_id_up_8x8(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 2;
	case 1: return 3;
	case 2: return 0;
	case 3: return 1;
	default: break;
	}

	assert(0);

	return ~0;
}

static unsigned get_sub_mb_id_up_right_8x8(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 2;
	case 1: return 3;
	case 2: return 1;
	case 3: return 0;
	default: break;
	}

	assert(0);

	return ~0;
}

static unsigned get_sub_mb_id_left_up_8x8(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 3;
	case 1: return 2;
	case 2: return 1;
	case 3: return 0;
	default: break;
	}

	assert(0);

	return ~0;
}

static unsigned get_sub_mb_id_up_4x4(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 10;
	case 1: return 11;
	case 2: return 0;
	case 3: return 1;
	case 4: return 14;
	case 5: return 15;
	case 6: return 4;
	case 7: return 5;
	case 8: return 2;
	case 9: return 3;
	case 10: return 8;
	case 11: return 9;
	case 12: return 6;
	case 13: return 7;
	case 14: return 12;
	case 15: return 13;
	default: break;
	}

	assert(0);

	return ~0;
}

static unsigned get_sub_mb_id_up_right_4x4(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 11;
	case 1: return 14;
	case 2: return 1;
	case 3: return 4;
	case 4: return 15;
	case 5: return 10;
	case 6: return 5;
	case 7: return 0;
	case 8: return 3;
	case 9: return 6;
	case 10: return 9;
	case 11: return 12;
	case 12: return 7;
	case 13: return 2;
	case 14: return 13;
	case 15: return 8;
	default: break;
	}

	assert(0);

	return ~0;
}

static unsigned get_sub_mb_id_left_up_4x4(unsigned sub_mb_id)
{
	switch (sub_mb_id) {
	case 0: return 15;
	case 1: return 10;
	case 2: return 5;
	case 3: return 0;
	case 4: return 11;
	case 5: return 14;
	case 6: return 1;
	case 7: return 4;
	case 8: return 7;
	case 9: return 2;
	case 10: return 13;
	case 11: return 8;
	case 12: return 3;
	case 13: return 6;
	case 14: return 9;
	case 15: return 12;
	default: break;
	}

	assert(0);

	return ~0;
}

signed get_sub_id_4x4_left(const decoder_context *decoder, macroblock **mb,
			   unsigned mb_id, unsigned sub_mb_id)
{
	signed left_mb_id;

	switch (sub_mb_id) {
	case 0:
	case 2:
	case 8:
	case 10:
		left_mb_id = get_mb_id_left(decoder, mb_id);

		if (left_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_mb_id];
	default:
		sub_mb_id = get_sub_mb_id_left_4x4(sub_mb_id);
		break;
	}

	return sub_mb_id;
}

signed get_sub_id_4x4_up(const decoder_context *decoder, macroblock **mb,
			 unsigned mb_id, unsigned sub_mb_id)
{
	signed up_mb_id;

	switch (sub_mb_id) {
	case 0:
	case 1:
	case 4:
	case 5:
		up_mb_id = get_mb_id_up(decoder, mb_id);

		if (up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[up_mb_id];
	default:
		sub_mb_id = get_sub_mb_id_up_4x4(sub_mb_id);
		break;
	}

	return sub_mb_id;
}

signed get_sub_id_4x4_up_right(const decoder_context *decoder, macroblock **mb,
			       unsigned mb_id, unsigned sub_mb_id)
{
	signed sub_mb_id_up_right = get_sub_mb_id_up_right_4x4(sub_mb_id);
	signed up_right_mb_id;

	switch (sub_mb_id) {
	case 0:
	case 1:
	case 4:
		up_right_mb_id = get_mb_id_up(decoder, mb_id);

		if (up_right_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[up_right_mb_id];

		return sub_mb_id_up_right;
	case 5:
		up_right_mb_id = get_mb_id_up_right(decoder, mb_id);

		if (up_right_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[up_right_mb_id];

		return sub_mb_id_up_right;
	case 7:
	case 13:
	case 15:
		return MB_UNAVAILABLE;
	default:
		break;
	}

	if (sub_mb_id_up_right > sub_mb_id) {
		return MB_UNAVAILABLE;
	}

	return sub_mb_id_up_right;
}

signed get_sub_id_4x4_left_up(const decoder_context *decoder, macroblock **mb,
			      unsigned mb_id, unsigned sub_mb_id)
{
	signed left_up_mb_id;

	switch (sub_mb_id) {
	case 0:
		left_up_mb_id = get_mb_id_left_up(decoder, mb_id);

		if (left_up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_up_mb_id];
		break;
	case 1:
	case 4:
	case 5:
		left_up_mb_id = get_mb_id_up(decoder, mb_id);

		if (left_up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_up_mb_id];
		break;
	case 2:
	case 8:
	case 10:
		left_up_mb_id = get_mb_id_left(decoder, mb_id);

		if (left_up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_up_mb_id];
		break;
	default:
		break;
	}

	return get_sub_mb_id_left_up_4x4(sub_mb_id);
}

signed get_sub_id_8x8_left(const decoder_context *decoder, macroblock **mb,
			   unsigned mb_id, unsigned sub_mb_id)
{
	signed left_mb_id;

	switch (sub_mb_id) {
	case 0:
	case 2:
		left_mb_id = get_mb_id_left(decoder, mb_id);

		if (left_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_mb_id];
	default:
		sub_mb_id = get_sub_mb_id_left_8x8(sub_mb_id);
		break;
	}

	return sub_mb_id;
}

signed get_sub_id_8x8_up(const decoder_context *decoder, macroblock **mb,
			 unsigned mb_id, unsigned sub_mb_id)
{
	signed up_mb_id;

	switch (sub_mb_id) {
	case 0:
	case 1:
		up_mb_id = get_mb_id_up(decoder, mb_id);

		if (up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[up_mb_id];
	default:
		sub_mb_id = get_sub_mb_id_up_8x8(sub_mb_id);
		break;
	}

	return sub_mb_id;
}

signed get_sub_id_8x8_up_right(const decoder_context *decoder, macroblock **mb,
			       unsigned mb_id, unsigned sub_mb_id)
{
	signed sub_mb_id_up_right = get_sub_mb_id_up_right_8x8(sub_mb_id);
	signed up_right_mb_id;

	switch (sub_mb_id) {
	case 0:
		up_right_mb_id = get_mb_id_up(decoder, mb_id);

		if (up_right_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[up_right_mb_id];

		return sub_mb_id_up_right;
	case 1:
		up_right_mb_id = get_mb_id_up_right(decoder, mb_id);

		if (up_right_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[up_right_mb_id];

		return sub_mb_id_up_right;
	case 3:
		return MB_UNAVAILABLE;
	default:
		break;
	}

	if (sub_mb_id_up_right > sub_mb_id) {
		return MB_UNAVAILABLE;
	}

	return sub_mb_id_up_right;
}

signed get_sub_id_8x8_left_up(const decoder_context *decoder, macroblock **mb,
			      unsigned mb_id, unsigned sub_mb_id)
{
	signed left_up_mb_id;

	switch (sub_mb_id) {
	case 0:
		left_up_mb_id = get_mb_id_left_up(decoder, mb_id);

		if (left_up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_up_mb_id];
		break;
	case 1:
		left_up_mb_id = get_mb_id_up(decoder, mb_id);

		if (left_up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_up_mb_id];
		break;
	case 2:
		left_up_mb_id = get_mb_id_left(decoder, mb_id);

		if (left_up_mb_id == MB_UNAVAILABLE) {
			return MB_UNAVAILABLE;
		}

		*mb = &decoder->sd.macroblocks[left_up_mb_id];
		break;
	default:
		break;
	}

	return get_sub_mb_id_left_up_8x8(sub_mb_id);
}
