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

void macroblock_prediction(const decoder_context *decoder, macroblock *mb,
			   unsigned mb_id)
{
	bitstream_reader *reader = (void *) &decoder->reader;
	uint32_t intra_chroma_pred_mode;

	switch ( MbPartPredMode(mb, I) ) {
	case Intra_4x4:
		macroblock_prediction_mode_intra_4x4(decoder, mb, mb_id);
		break;
	case Intra_8x8:
		SYNTAX_ERR("Intra_8x8 unimplemented\n");
		break;
	case Intra_16x16:
		break;
	default:
		SYNTAX_ERR("Shouldn't be here\n");
	}

	intra_chroma_pred_mode = bitstream_read_ue(reader);

	if (intra_chroma_pred_mode > 3) {
		SYNTAX_ERR("intra_chroma_pred_mode malformed\n");
	}

	mb->intra_chroma_pred_mode = intra_chroma_pred_mode;

// 	SYNTAX_IPRINT("intra_chroma_pred_mode = %u\n", intra_chroma_pred_mode);
}
