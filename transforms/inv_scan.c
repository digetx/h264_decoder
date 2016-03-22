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

void unzigzag_4x4(int16_t coeffLevel[16])
{
	int16_t tmp[12];
	int i;

	tmp[0]  = coeffLevel[5];
	tmp[1]  = coeffLevel[6];
	tmp[2]  = coeffLevel[2];
	tmp[3]  = coeffLevel[4];
	tmp[4]  = coeffLevel[7];
	tmp[5]  = coeffLevel[12];
	tmp[6]  = coeffLevel[3];
	tmp[7]  = coeffLevel[8];
	tmp[8]  = coeffLevel[11];
	tmp[9]  = coeffLevel[13];
	tmp[10] = coeffLevel[9];
	tmp[11] = coeffLevel[10];

	for (i = 0; i < 12; i++) {
		coeffLevel[i + 2] = tmp[i];
	}

	TRANSFORM_DPRINT("unzigzag:\t%d %d %d %d\n",
		coeffLevel[0], coeffLevel[1], coeffLevel[2], coeffLevel[3]);
	TRANSFORM_DPRINT("\t\t%d %d %d %d\n",
		coeffLevel[4], coeffLevel[5], coeffLevel[6], coeffLevel[7]);
	TRANSFORM_DPRINT("\t\t%d %d %d %d\n",
		coeffLevel[8], coeffLevel[9], coeffLevel[10], coeffLevel[11]);
	TRANSFORM_DPRINT("\t\t%d %d %d %d\n",
		coeffLevel[12], coeffLevel[13], coeffLevel[14], coeffLevel[15]);
}

void unzigzag_4x4_15(int16_t coeffLevel[16])
{
	int16_t tmp[12];
	int i;

	tmp[0]  = coeffLevel[4];
	tmp[1]  = coeffLevel[5];
	tmp[2]  = coeffLevel[1];
	tmp[3]  = coeffLevel[3];
	tmp[4]  = coeffLevel[6];
	tmp[5]  = coeffLevel[11];
	tmp[6]  = coeffLevel[2];
	tmp[7]  = coeffLevel[7];
	tmp[8]  = coeffLevel[10];
	tmp[9]  = coeffLevel[12];
	tmp[10] = coeffLevel[8];
	tmp[11] = coeffLevel[9];

	coeffLevel[15] = coeffLevel[14];
	coeffLevel[14] = coeffLevel[13];
	coeffLevel[1]  = coeffLevel[0];

	for (i = 0; i < 12; i++) {
		coeffLevel[i + 2] = tmp[i];
	}

	TRANSFORM_DPRINT("unzigzag:\t%d %d %d %d\n",
		coeffLevel[0], coeffLevel[1], coeffLevel[2], coeffLevel[3]);
	TRANSFORM_DPRINT("\t\t%d %d %d %d\n",
		coeffLevel[4], coeffLevel[5], coeffLevel[6], coeffLevel[7]);
	TRANSFORM_DPRINT("\t\t%d %d %d %d\n",
		coeffLevel[8], coeffLevel[9], coeffLevel[10], coeffLevel[11]);
	TRANSFORM_DPRINT("\t\t%d %d %d %d\n",
		coeffLevel[12], coeffLevel[13], coeffLevel[14], coeffLevel[15]);
}
