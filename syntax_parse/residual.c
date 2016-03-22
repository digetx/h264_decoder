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

static signed get_total_coeff_luma_AC_left(const decoder_context *decoder,
					   macroblock *mb, unsigned mb_id,
					   signed sub_mb_id)
{
	macroblock *src_mb = mb;

	sub_mb_id = get_sub_id_4x4_left(decoder, &src_mb, mb_id, sub_mb_id);

	if (sub_mb_id == MB_UNAVAILABLE) {
		return MB_UNAVAILABLE;
	}

	return src_mb->luma_AC[sub_mb_id].totalcoeff;
}

static signed get_total_coeff_luma_AC_up(const decoder_context *decoder,
					 macroblock *mb, unsigned mb_id,
					 unsigned sub_mb_id)
{
	macroblock *src_mb = mb;

	sub_mb_id = get_sub_id_4x4_up(decoder, &src_mb, mb_id, sub_mb_id);

	if (sub_mb_id == MB_UNAVAILABLE) {
		return MB_UNAVAILABLE;
	}

	return src_mb->luma_AC[sub_mb_id].totalcoeff;
}

static signed get_total_coeff_chroma_AC_left(const decoder_context *decoder,
					     macroblock *mb, unsigned mb_id,
					     signed sub_mb_id, unsigned plane_id)
{
	macroblock *src_mb = mb;

	sub_mb_id = get_sub_id_8x8_left(decoder, &src_mb, mb_id, sub_mb_id);

	if (sub_mb_id == MB_UNAVAILABLE) {
		return MB_UNAVAILABLE;
	}

	return src_mb->chroma_AC[plane_id][sub_mb_id].totalcoeff;
}

static signed get_total_coeff_chroma_AC_up(const decoder_context *decoder,
					   macroblock *mb, unsigned mb_id,
					   signed sub_mb_id, unsigned plane_id)
{
	macroblock *src_mb = mb;

	sub_mb_id = get_sub_id_8x8_up(decoder, &src_mb, mb_id, sub_mb_id);

	if (sub_mb_id == MB_UNAVAILABLE) {
		return MB_UNAVAILABLE;
	}

	return src_mb->chroma_AC[plane_id][sub_mb_id].totalcoeff;
}

static unsigned residual_block(const decoder_context *decoder, macroblock *mb,
			       int startIdx, int endIdx, unsigned maxNumCoeff,
			       signed nC, int16_t res_array[16])
{
	bitstream_reader *reader = (void *) &decoder->reader;
	unsigned totalcoeff;

	if (!CABAC_MODE) {
		totalcoeff = residual_block_vlc(reader, startIdx, endIdx,
						maxNumCoeff, nC, res_array);
	} else {
		SYNTAX_ERR("Shouldn't be here");
	}

	return totalcoeff;
}

static unsigned coeffNum(bitstream_reader *reader, signed nCA, signed nCB)
{
	unsigned nC;

	if (nCA != MB_UNAVAILABLE && nCB != MB_UNAVAILABLE) {
		nC = (nCA + nCB + 1) >> 1;
	} else if (nCA != MB_UNAVAILABLE) {
		nC = nCA;
	} else if (nCB != MB_UNAVAILABLE) {
		nC = nCB;
	} else {
		nC = 0;
	}

// 	SYNTAX_DPRINT("coeffNum = %u nCA %d nCB %d\n", nC, nCA, nCB);

	return nC;
}

static void residual_luma(const decoder_context *decoder, macroblock *mb,
			  unsigned mb_id, int startIdx, int endIdx,
			  unsigned CBPLuma)
{
	bitstream_reader *reader = (void *) &decoder->reader;
	unsigned maxNumCoeff = 16;
	signed nCA, nCB, nC;
	int i4x4, i8x8;

	if (startIdx == 0 && MbPartPredMode(mb, I) == Intra_16x16) {
		nCA = get_total_coeff_luma_AC_left(decoder, mb, mb_id, 0);
		nCB = get_total_coeff_luma_AC_up(decoder, mb, mb_id, 0);
		nC = coeffNum(reader, nCA, nCB);

// 				SYNTAX_DPRINT("luma_DC ");
		mb->luma_DC.totalcoeff = residual_block(decoder, mb, 0, 15, 16,
							nC, mb->luma_DC.coeffs);
	} else {
		mb->luma_DC.totalcoeff = 0;
	}

	if (MbPartPredMode(mb, I) == Intra_16x16) {
		startIdx = max(0, startIdx - 1);
		endIdx = endIdx - 1;
		maxNumCoeff = 15;
	}

	for (i8x8 = 0; i8x8 < 4; i8x8++) {
		for (i4x4 = 0; i4x4 < 4; i4x4++) {
			unsigned id = i8x8 * 4 + i4x4;
			int16_t *luma_AC = mb->luma_AC[id].coeffs;

			if (CBPLuma & (1 << i8x8)) {
				nCA = get_total_coeff_luma_AC_left(
							decoder, mb, mb_id, id);
				nCB = get_total_coeff_luma_AC_up(
							decoder, mb, mb_id, id);
				nC = coeffNum(reader, nCA, nCB);

// 				SYNTAX_DPRINT("luma_AC ");
				mb->luma_AC[id].totalcoeff =
					residual_block(decoder, mb, startIdx,
						       endIdx, maxNumCoeff, nC,
						       luma_AC);
			} else {
				mb->luma_AC[id].totalcoeff = 0;;
			}
		}
	}
}

void residual(const decoder_context *decoder, macroblock *mb, unsigned mb_id,
	      int startIdx, int endIdx, unsigned CBPLuma, unsigned CBPChroma)
{
	bitstream_reader *reader = (void *) &decoder->reader;
	unsigned ChromaArrayType = ChromaArrayType();
	signed nCA, nCB, nC;
	int iCbCr, i4x4;

	residual_luma(decoder, mb, mb_id, startIdx, endIdx, CBPLuma);

	if (ChromaArrayType == 1 || ChromaArrayType == 2) {
		for (iCbCr = 0; iCbCr < 2; iCbCr++) {
			int16_t *chroma_DC = mb->chroma_DC[iCbCr].coeffs;

			if (!(CBPChroma & 3) || startIdx != 0) {
				mb->chroma_DC[iCbCr].totalcoeff = 0;
				continue;
			}

// 			SYNTAX_DPRINT("chroma_DC ");
// 			SYNTAX_DPRINT("coeffNum = %u nCA %d nCB %d\n", 0, 0, -1);
			/* chroma DC residual present */
			mb->chroma_DC[iCbCr].totalcoeff =
				residual_block(decoder, mb, 0, 3, 4,
					(ChromaArrayType == 1) ? -1 : -2,
					chroma_DC);
		}

		startIdx = max(0, startIdx - 1);
		endIdx = endIdx - 1;

		for (iCbCr = 0; iCbCr < 2; iCbCr++) {
			for (i4x4 = 0; i4x4 < 4; i4x4++) {
				int16_t *chroma_AC = mb->chroma_AC[iCbCr][i4x4].coeffs;

				if (!(CBPChroma & 2)) {
					mb->chroma_AC[iCbCr][i4x4].totalcoeff = 0;
					continue;
				}

				nCA = get_total_coeff_chroma_AC_left(
					decoder, mb, mb_id, i4x4, iCbCr);
				nCB = get_total_coeff_chroma_AC_up(
					decoder, mb, mb_id, i4x4, iCbCr);
				nC = coeffNum(reader, nCA, nCB);

// 				SYNTAX_DPRINT("chroma_AC ");
				mb->chroma_AC[iCbCr][i4x4].totalcoeff =
					residual_block(decoder, mb, startIdx,
						endIdx, 15, nC, chroma_AC);
			}
		}
	} else if (ChromaArrayType == 3) {
		SYNTAX_ERR("Shouldn't be here");
	} else {
		for (iCbCr = 0; iCbCr < 2; iCbCr++) {
			mb->chroma_DC[iCbCr].totalcoeff = 0;

			for (i4x4 = 0; i4x4 < 4; i4x4++) {
				mb->chroma_AC[iCbCr][i4x4].totalcoeff = 0;
			}
		}
	}
}
