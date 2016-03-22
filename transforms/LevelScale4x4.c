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

#define mbIsInterFlag	(decoder->sh.slice_type == P)

static const int v[3][6] = {
	{ 10, 11, 13, 14, 16, 18 },
	{ 16, 18, 20, 23, 25, 29 },
	{ 13, 14, 16, 18, 20, 23 },
};

static int weightScale4x4(const decoder_context *decoder, int i, int j,
			  int iYCbCr)
{
// 	int sId = iYCbCr + (mbIsInterFlag == 1) ? 3 : 0;
//
// 	if (decoder->pps.pic_scaling_matrix_present_flag) {
// 		if (!decoder->pps.UseDefaultScalingMatrix4x4Flag[sId]) {
// 			return decoder->pps.scalingList_4x4[sId][sId];
// 		}
//
// 		return 16;
// 	}
//
// 	if (decoder->sps.seq_scaling_list_present_flag & (1 << sId)) {
// 		if (!decoder->sps.UseDefaultScalingMatrix4x4Flag[sId]) {
// 			return decoder->sps.scalingList_4x4[sId][sId];
// 		}
// 	}

	return 16;
}

static int normAdjust4x4(int m, int i, int j)
{
	i %= 2;
	j %= 2;

	if (i == 0 && j == 0) {
		return v[0][m];
	}

	if (i == 1 && j == 1) {
		return v[1][m];
	}

	return v[2][m];
}

int LevelScale4x4(const decoder_context *decoder, int m, int i, int j, int iYCbCr)
{
	return weightScale4x4(decoder, i, j, iYCbCr) * normAdjust4x4(m, i, j);
}
