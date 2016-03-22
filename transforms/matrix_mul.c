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

void matrix_mul(const int16_t *mat1, const int16_t *mat2, int16_t *result, int sz)
{
	int i, row, column;

	for (row = 0; row < sz; row++) {
		for (i = 0; i < sz; i++) {
			int16_t res = 0;

			for (column = 0; column < sz; column++) {
				res += mat1[row * sz + column] * mat2[column * sz + i];
			}

			result[row * sz + i] = res;
		}
	}
}
