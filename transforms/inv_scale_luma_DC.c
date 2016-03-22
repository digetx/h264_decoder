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

void inv_scale_luma_DC(const decoder_context *decoder, const int16_t c[16],
		       int16_t dcY[16], int qP)
{
	int scale;
	int i;

	scale = LevelScale4x4(decoder, qP % 6, 0, 0, Y_DC);

	for (i = 0; i < 16; i++) {
		int16_t f = c[i];

		if (qP >= 36) {
			dcY[i] = (f * scale) << (qP / 6 - 6);
		} else {
			dcY[i] = ((f * scale) + (1 << (5 - qP / 6))) >> (6 - qP / 6);
		}
	}

	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 dcY[0], dcY[1], dcY[2], dcY[3]);
	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 dcY[4], dcY[5], dcY[6], dcY[7]);
	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 dcY[8], dcY[9], dcY[10], dcY[11]);
	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 dcY[12], dcY[13], dcY[14], dcY[15]);
}
