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

#define YUV400	0
#define YUV420	1
#define YUV422	2
#define YUV444	3

static const uint8_t coded_block_pattern_intra_1_2[] = {
	47, 31, 15, 0, 23, 27, 29, 30, 7, 11, 13, 14, 39, 43, 45, 46, 16, 3, 5,
	10, 12, 19, 21, 26, 28, 35, 37, 42, 44, 1, 2, 4, 8, 17, 18, 20, 24, 6,
	9, 22, 25, 32, 33, 34, 36, 40, 38, 41,
};

static const uint8_t coded_block_pattern_intra_0_3[] = {
	15, 0, 7, 11, 13, 14, 3, 5, 10, 12, 1, 2, 4, 8, 6, 9,
};

static const char * MB_TYPE_I(const macroblock *mb)
{
	switch (mb->mb_type) {
	case 0:		return mb->transform_size_8x8_flag ? "I_8x8" : "I_4x4";
	case 1 ... 24:	return "I_16x16";
// 	case 1:		return "I_16x16_0_0_0";
// 	case 2:		return "I_16x16_1_0_0";
// 	case 3:		return "I_16x16_2_0_0";
// 	case 4:		return "I_16x16_3_0_0";
// 	case 5:		return "I_16x16_0_1_0";
// 	case 6:		return "I_16x16_1_1_0";
// 	case 7:		return "I_16x16_2_1_0";
// 	case 8:		return "I_16x16_3_1_0";
// 	case 9:		return "I_16x16_0_2_0";
// 	case 10:	return "I_16x16_1_2_0";
// 	case 11:	return "I_16x16_2_2_0";
// 	case 12:	return "I_16x16_3_2_0";
// 	case 13:	return "I_16x16_0_0_1";
// 	case 14:	return "I_16x16_1_0_1";
// 	case 15:	return "I_16x16_2_0_1";
// 	case 16:	return "I_16x16_3_0_1";
// 	case 17:	return "I_16x16_0_1_1";
// 	case 18:	return "I_16x16_1_1_1";
// 	case 19:	return "I_16x16_2_1_1";
// 	case 20:	return "I_16x16_3_1_1";
// 	case 21:	return "I_16x16_0_2_1";
// 	case 22:	return "I_16x16_1_2_1";
// 	case 23:	return "I_16x16_2_2_1";
// 	case 24:	return "I_16x16_3_2_1";
	case 25:	return "I_PCM";
	default:
		SYNTAX_ERR("Bad MB type %d\n", mb->mb_type);
		break;
	}

	return "";
}

static unsigned NumMbPart(const macroblock *mb)
{
	if (mb->mb_type == 0) {
		return mb->transform_size_8x8_flag ? 4 : 16;
	}

	return 1;
}

static int MbPartPredMode_I(const macroblock *mb)
{
	switch (mb->mb_type) {
	case 0:
		return mb->transform_size_8x8_flag ? Intra_8x8 : Intra_4x4;
	case 1 ... 24:
		return Intra_16x16;
	default:
		break;
	}

	return -1;
}

int MbPartPredMode(const macroblock *mb, int slice_type)
{
	switch (slice_type) {
	case I:
		return MbPartPredMode_I(mb);
	default:
		break;
	}

	return -1;
}

void macroblock_layer(const decoder_context *decoder, unsigned mb_id)
{
	bitstream_reader *reader = (void *) &decoder->reader;
	unsigned mb_id_in_slice = mb_id - decoder->sh.first_mb_in_slice;
	macroblock *mb = &decoder->sd.macroblocks[mb_id_in_slice];
	unsigned ChromaArrayType = ChromaArrayType();
	unsigned CodedBlockPatternLuma = 0;
	unsigned CodedBlockPatternChroma = 0;
	unsigned noSubMbPartSizeLessThan8x8Flag = 1;
	unsigned MbWidthC, MbHeightC;
	int32_t mb_qp_delta;
	int i;

	mb->transform_size_8x8_flag = 0;

	if (CABAC_MODE) {
		mb->mb_type = bitstream_read_ae(reader);
	} else {
		mb->mb_type = bitstream_read_ue(reader);
	}

	switch (decoder->sh.slice_type) {
	case I:
		SYNTAX_IPRINT("MacroBlock type %d = %s\n",
			      mb->mb_type, MB_TYPE_I(mb));
		break;
	default:
		SYNTAX_ERR("MacroBlock slice_type unimplemented\n");
		break;
	}

	switch (decoder->sps.chroma_format_idc) {
	case YUV400:
		MbWidthC  = 4;
		MbHeightC = 4;
		break;
	case YUV420:
		MbWidthC  = 8;
		MbHeightC = 8;
		break;
	case YUV422:
		MbWidthC  = 8;
		MbHeightC = 16;
		break;
	case YUV444:
		MbWidthC  = 16;
		MbHeightC = 16;
		break;
	default:
		SYNTAX_ERR("Bad color format\n");
		break;
	}

	if (mb->mb_type == I_PCM) {
		uint8_t align_bits = bitstream_read_rbsp_align(reader);
// 		uint8_t luma_depth = decoder->sps.bit_depth_luma_minus8 + 8;
// 		uint8_t chroma_depth = decoder->sps.bit_depth_chroma_minus8 + 8;

		SYNTAX_ERR("I_PCM unimplemented\n");

		if (align_bits != 0) {
			SYNTAX_ERR("I_PCM align failure\n");
		}

		for (i = 0; i < 16 * 16; i++) {
// 			mb->luma_decoded[i] =
// 					bitstream_read_u(reader, luma_depth);
		}

		for (i = 0; i < MbWidthC * MbHeightC; i++) {
// 			mb->chromaU_decoded[i] =
// 					bitstream_read_u(reader, chroma_depth);
		}

		for (i = 0; i < MbWidthC * MbHeightC; i++) {
// 			mb->chromaV_decoded[i] =
// 					bitstream_read_u(reader, chroma_depth);
		}

		return;
	}

	if (mb->mb_type != I_NxN &&
		MbPartPredMode(mb, decoder->sh.slice_type) != Intra_16x16 &&
			NumMbPart(mb) == 4)
	{
		SYNTAX_ERR("MbPartPredMode unimplemented\n");
	} else {
		if (decoder->pps.transform_8x8_mode_flag &&
			mb->mb_type == I_NxN)
		{
			mb->transform_size_8x8_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("transform_size_8x8_flag %u\n",
				      mb->transform_size_8x8_flag);
		}

		macroblock_prediction(decoder, mb, mb_id);
	}

	if (MbPartPredMode(mb, decoder->sh.slice_type) != Intra_16x16) {
		unsigned codeNum = bitstream_read_ue(reader);
		unsigned coded_block_pattern = 0;

		if (ChromaArrayType == 1 || ChromaArrayType == 2) {
			if (codeNum > 47) {
				SYNTAX_ERR("Malformed codeNum %d\n", codeNum);
			}

			coded_block_pattern = coded_block_pattern_intra_1_2[codeNum];
		}

		if (ChromaArrayType == 0 || ChromaArrayType == 3) {
			if (codeNum > 15) {
				SYNTAX_ERR("Malformed codeNum %d\n", codeNum);
			}

			coded_block_pattern = coded_block_pattern_intra_0_3[codeNum];
		}

		CodedBlockPatternLuma   = coded_block_pattern % 16;
		CodedBlockPatternChroma = coded_block_pattern / 16;

// 		SYNTAX_IPRINT("coded_block_pattern = %u\n", coded_block_pattern);
// 		SYNTAX_IPRINT("CodedBlockPatternLuma = %u\n", CodedBlockPatternLuma);
// 		SYNTAX_IPRINT("CodedBlockPatternChroma = %u\n", CodedBlockPatternChroma);

		if (CodedBlockPatternLuma > 0 &&
			decoder->pps.transform_8x8_mode_flag && mb->mb_type != I_NxN &&
			noSubMbPartSizeLessThan8x8Flag &&
			(/*mb->mb_type != B_Direct_16x16 ||*/ decoder->sps.direct_8x8_inference_flag))
		{
			mb->transform_size_8x8_flag = bitstream_read_u(reader, 1);

			SYNTAX_IPRINT("transform_size_8x8_flag %u\n",
				      mb->transform_size_8x8_flag);
		}
	} else {
		switch (mb->mb_type) {
		case 1 ... 4:
			break;
		case 5 ... 8:
			CodedBlockPatternChroma = 1;
			break;
		case 9 ... 12:
			CodedBlockPatternChroma = 2;
			break;
		case 13 ... 16:
			CodedBlockPatternLuma = 15;
			break;
		case 17 ... 20:
			CodedBlockPatternChroma = 1;
			CodedBlockPatternLuma = 15;
			break;
		case 21 ... 24:
			CodedBlockPatternChroma = 2;
			CodedBlockPatternLuma = 15;
			break;
		}
	}

	if (CodedBlockPatternLuma == 0 && CodedBlockPatternChroma == 0 &&
		MbPartPredMode(mb, decoder->sh.slice_type) != Intra_16x16)
	{
		SYNTAX_IPRINT("MacroBlock is empty\n");

		mb->mb_qp_delta = 0;
		mb->luma_DC.totalcoeff = 0;

		for (i = 0; i < 16; i++) {
			mb->luma_AC[i].totalcoeff = 0;

			if (i < 2) {
				mb->chroma_DC[i].totalcoeff = 0;
			}

			if (i < 4) {
				mb->chroma_AC[0][i].totalcoeff = 0;
				mb->chroma_AC[1][i].totalcoeff = 0;
			}
		}
		return;
	}

	mb_qp_delta = bitstream_read_se(reader);

	if (mb_qp_delta > 26 || mb_qp_delta < -26) {
		SYNTAX_ERR("Malformed mb_qp_delta %d\n", mb_qp_delta);
	}

	mb->mb_qp_delta = mb_qp_delta;

// 	SYNTAX_IPRINT("mb_qp_delta = %d\n", mb->mb_qp_delta);

	residual(decoder, mb, mb_id, 0, 15, CodedBlockPatternLuma,
		 CodedBlockPatternChroma);
}
