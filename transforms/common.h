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

#ifndef TRANSFORMS_COMMON_H
#define TRANSFORMS_COMMON_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "decoder.h"
#include "syntax_parse.h"

#define TRANSFORM_DPRINT(f, ...)	printf(f, ## __VA_ARGS__)

enum { Y_DC, Y_AC, C_DC, C_AC};

void matrix_mul(const int16_t *a, const int16_t *b, int16_t *result, int sz);

void inv_transform_chroma_DC(const int16_t c[4], int16_t result[4]);

int QPy(const decoder_context *decoder, unsigned mb_id);

int LevelScale4x4(const decoder_context *decoder, int m, int i, int j, int iYCbCr);

void inv_transform_luma_DC(const int16_t c[16], int16_t result[16]);

void inv_transform_4x4(int16_t r[16]);

void inv_scale_luma_DC(const decoder_context *decoder, const int16_t c[16],
		       int16_t dcY[16], int qP);

void inv_scale_chroma_DC(const decoder_context *decoder, const int16_t c[16],
			 int16_t dcC[4], int QPc);

void inv_scale_4x4(const decoder_context *decoder, const int16_t c[16],
		   int16_t d[16], int qP);

int qPc(int qPi);

void unzigzag_4x4(int16_t coeffLevel[16]);

void unzigzag_4x4_15(int16_t coeffLevel[16]);

void mb_apply_chroma_prediction(const decoder_context *decoder, int plane_id,
				unsigned mb_id, int16_t residual[16][16]);

void mb_apply_luma_prediction_16x16(const decoder_context *decoder,
				    unsigned mb_id, int Intra16x16PredMode,
				    int16_t residual[16][16]);

void mb_apply_luma_prediction_4x4(const decoder_context *decoder,
				  unsigned mb_id, int16_t residual[16][16]);

#define Clip1(z)	Clip3(0, (1 << bit_depth) - 1, (z))
#define Clip3(x, y, z)	(((z) < (x)) ? (x) : (((z) > (y)) ? (y) : (z)))

#define SETUP_PLANE_PTR(chroma_decoded, plane_id, mb, id)	\
	if (plane_id == 0) {					\
		chroma_decoded = mb->chroma_U_decoded[id];	\
	} else {						\
		chroma_decoded = mb->chroma_V_decoded[id];	\
	}

#endif // TRANSFORMS_COMMON_H
