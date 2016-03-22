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

static const int16_t tfmat[4] = {
	1,  1,
	1, -1,
};

void inv_transform_chroma_DC(const int16_t c[4], int16_t result[4])
{
	int16_t tmp[4];

	matrix_mul(tfmat, c, tmp, 2);
	matrix_mul(tmp, tfmat, result, 2);

	TRANSFORM_DPRINT("transformed:\t%d %d %d %d\n",
			 result[0], result[1], result[2], result[3]);
}
