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

#ifndef SYNTAX_PARSE_H
#define SYNTAX_PARSE_H

#include <stdint.h>

#include "decoder.h"

#define Intra_4x4	0
#define Intra_8x8	1
#define Intra_16x16	2

#define MB_UNAVAILABLE	-1

void parse_annex_b(decoder_context *decoder);

int parse_mp4(decoder_context *decoder);

int MbPartPredMode(const macroblock *mb, int slice_type);

signed get_mb_id_left(const decoder_context *decoder, unsigned mb_id);

signed get_mb_id_up(const decoder_context *decoder, unsigned mb_id);

signed get_sub_id_4x4_left(const decoder_context *decoder, macroblock **mb,
			   unsigned mb_id, unsigned sub_mb_id);

signed get_sub_id_4x4_up(const decoder_context *decoder, macroblock **mb,
			 unsigned mb_id, unsigned sub_mb_id);

signed get_sub_id_4x4_up_right(const decoder_context *decoder, macroblock **mb,
			       unsigned mb_id, unsigned sub_mb_id);

signed get_sub_id_4x4_left_up(const decoder_context *decoder, macroblock **mb,
			      unsigned mb_id, unsigned sub_mb_id);

signed get_sub_id_8x8_left(const decoder_context *decoder, macroblock **mb,
			   unsigned mb_id, unsigned sub_mb_id);

signed get_sub_id_8x8_up(const decoder_context *decoder, macroblock **mb,
			 unsigned mb_id, unsigned sub_mb_id);

signed get_sub_id_8x8_up_right(const decoder_context *decoder, macroblock **mb,
			       unsigned mb_id, unsigned sub_mb_id);

signed get_sub_id_8x8_left_up(const decoder_context *decoder, macroblock **mb,
			      unsigned mb_id, unsigned sub_mb_id);

#define get_sub_id_2x2_left	get_sub_id_8x8_left
#define get_sub_id_2x2_up	get_sub_id_8x8_up
#define get_sub_id_2x2_up_right	get_sub_id_8x8_up_right
#define get_sub_id_2x2_left_up	get_sub_id_8x8_left_up

const int mb_scan_map(int id);

#endif // SYNTAX_PARSE_H
