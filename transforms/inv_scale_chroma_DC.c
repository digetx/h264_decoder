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

int qPc(int qPi)
{
	switch (qPi) {
	case 30: return 29;
	case 31: return 30;
	case 32: return 31;
	case 33: return 32;
	case 34: return 32;
	case 35: return 33;
	case 36: return 34;
	case 37: return 34;
	case 38: return 35;
	case 39: return 35;
	case 40: return 36;
	case 41: return 36;
	case 42: return 37;
	case 43: return 37;
	case 44: return 37;
	case 45: return 38;
	case 46: return 38;
	case 47: return 38;
	case 48: return 39;
	case 49: return 39;
	case 50: return 39;
	case 51: return 39;
	default: return qPi;
	}
}

void inv_scale_chroma_DC(const decoder_context *decoder, const int16_t c[16],
			 int16_t dcC[4], int QPc)
{
	int scale;
	int i;

	scale = LevelScale4x4(decoder, QPc % 6, 0, 0, C_DC);

	for (i = 0; i < 4; i++) {
		dcC[i] = ((c[i] * scale) << (QPc / 6)) >> 5;
	}

	TRANSFORM_DPRINT("decoding scaled:\t%d %d %d %d\n",
			 dcC[0], dcC[1], dcC[2], dcC[3]);
}
