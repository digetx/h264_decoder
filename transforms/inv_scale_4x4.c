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

void inv_scale_4x4(const decoder_context *decoder, const int16_t c[16],
		   int16_t d[16], int qP)
{
	int i, scale;

	TRANSFORM_DPRINT("in scaled:\t%d %d %d %d\n", c[0], c[1], c[2], c[3]);
	TRANSFORM_DPRINT("in scaled:\t%d %d %d %d\n", c[4], c[5], c[6], c[7]);
	TRANSFORM_DPRINT("in scaled:\t%d %d %d %d\n", c[8], c[9], c[10], c[11]);
	TRANSFORM_DPRINT("in scaled:\t%d %d %d %d\n", c[12], c[13], c[14], c[15]);

	for (i = 0; i < 16; i++) {
		scale = LevelScale4x4(decoder, qP % 6, i / 4, i % 4, Y_AC);

		if (qP >= 24) {
			d[i] = (c[i] * scale) << (qP / 6 - 4);
		} else {
			d[i] = (c[i] * scale + (1 << (3 - qP / 6))) >> (4 - qP / 6);
		}
	}

	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 d[0], d[1], d[2], d[3]);
	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 d[4], d[5], d[6], d[7]);
	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 d[8], d[9], d[10], d[11]);
	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 d[12], d[13], d[14], d[15]);
}
