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

void inv_transform_4x4(int16_t r[16])
{
	int16_t e[4], f[4][4], g[4], h[4][4];
	int i, j;

	for (i = 0; i < 4; i++) {
		e[0] = r[i * 4] + r[i * 4 + 2];
		e[1] = r[i * 4] - r[i * 4 + 2];
		e[2] = (r[i * 4 + 1] >> 1) - r[i * 4 + 3];
		e[3] = r[i * 4 + 1] + (r[i * 4 + 3] >> 1);

		f[i][0] = e[0] + e[3];
		f[i][1] = e[1] + e[2];
		f[i][2] = e[1] - e[2];
		f[i][3] = e[0] - e[3];
	}

	for (j = 0; j < 4; j++) {
		g[0] = f[0][j] + f[2][j];
		g[1] = f[0][j] - f[2][j];
		g[2] = (f[1][j] >> 1) - f[3][j];
		g[3] = f[1][j] + (f[3][j] >> 1);

		h[0][j] = g[0] + g[3];
		h[1][j] = g[1] + g[2];
		h[2][j] = g[1] - g[2];
		h[3][j] = g[0] - g[3];
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			r[i * 4 + j] = (h[i][j] + (1 << 5)) >> 6;
		}
	}

	TRANSFORM_DPRINT("decoding transformed:\t%d %d %d %d\n",
			 r[0], r[1], r[2], r[3]);
	TRANSFORM_DPRINT("decoding transformed:\t%d %d %d %d\n",
			 r[4], r[5], r[6], r[7]);
	TRANSFORM_DPRINT("decoding transformed:\t%d %d %d %d\n",
			 r[8], r[9], r[10], r[11]);
	TRANSFORM_DPRINT("decoding transformed:\t%d %d %d %d\n",
			 r[12], r[13], r[14], r[15]);
}
