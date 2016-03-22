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

#include "CAVLC_LUTs.h"
#include "common.h"

static const calvc_level_lut * coeff_token_lut(bitstream_reader *reader,
					       signed coeffNum)
{
	const calvc_level_lut *lvl_lut;
	unsigned lutSize;
	uint16_t data;
	int i;

	switch (coeffNum) {
	case 0 ... 1:
		lvl_lut = coeff_token_nC_0_1;
		lutSize = ARRAY_SIZE(coeff_token_nC_0_1);
		break;
	case 2 ... 3:
		lvl_lut = coeff_token_nC_2_3;
		lutSize = ARRAY_SIZE(coeff_token_nC_2_3);
		break;
	case 4 ... 7:
		lvl_lut = coeff_token_nC_4_7;
		lutSize = ARRAY_SIZE(coeff_token_nC_4_7);
		break;
	case -1:
		lvl_lut = coeff_token_nC_1;
		lutSize = ARRAY_SIZE(coeff_token_nC_1);
		break;
	case -2:
		lvl_lut = coeff_token_nC_2;
		lutSize = ARRAY_SIZE(coeff_token_nC_2);
		break;
	default:
		lvl_lut = coeff_token_nC_8;
		lutSize = ARRAY_SIZE(coeff_token_nC_8);
		break;
	}

	data = bitstream_read_u_no_inc(reader, 16);

// 	SYNTAX_DPRINT("data 0x%02X\n", data);

	for (i = 0; i < lutSize; i++) {
		unsigned lshift = lvl_lut[i].coeff_token_len;
		unsigned rshift = 16 - lvl_lut[i].coeff_token_len;
		uint32_t mask   = (1l << lshift) - 1;
		uint32_t val    = data;

		val >>= rshift;
		val &= mask;

		if (val == lvl_lut[i].coeff_token) {
			bitstream_read_u(reader, lvl_lut[i].coeff_token_len);
			break;
		}
	}

	if (i == lutSize) {
		SYNTAX_ERR("CALVC totalCoeff LUT not matched\n");
	}

	return &lvl_lut[i];
}

static unsigned total_zeros(bitstream_reader *reader, unsigned totalCoeff,
			    unsigned maxNumCoeff)
{
	unsigned tzVlcIndex = totalCoeff;
	unsigned tz_lut_size = 0;
	unsigned zerosLeft = 0;
	const calvc_tz_lut *tz_lut;
	uint16_t data;
	int i;

	switch (maxNumCoeff) {
	case 4:
		switch (tzVlcIndex) {
		case 1:
			tz_lut = calvc_tz_lut_2x2_1;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x2_1);
			break;
		case 2:
			tz_lut = calvc_tz_lut_2x2_2;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x2_2);
			break;
		case 3:
			tz_lut = calvc_tz_lut_2x2_3;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x2_3);
			break;
		}
		break;
	case 8:
		switch (tzVlcIndex) {
		case 1:
			tz_lut = calvc_tz_lut_2x4_1;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x4_1);
			break;
		case 2:
			tz_lut = calvc_tz_lut_2x4_2;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x4_2);
			break;
		case 3:
			tz_lut = calvc_tz_lut_2x4_3;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x4_3);
			break;
		case 4:
			tz_lut = calvc_tz_lut_2x4_4;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x4_4);
			break;
		case 5:
			tz_lut = calvc_tz_lut_2x4_5;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x4_5);
			break;
		case 6:
			tz_lut = calvc_tz_lut_2x4_6;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x4_6);
			break;
		case 7:
			tz_lut = calvc_tz_lut_2x4_7;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_2x4_7);
			break;
		}
		break;
	default:
		switch (tzVlcIndex) {
		case 1:
			tz_lut = calvc_tz_lut_4x4_1;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_1);
			break;
		case 2:
			tz_lut = calvc_tz_lut_4x4_2;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_2);
			break;
		case 3:
			tz_lut = calvc_tz_lut_4x4_3;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_3);
			break;
		case 4:
			tz_lut = calvc_tz_lut_4x4_4;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_4);
			break;
		case 5:
			tz_lut = calvc_tz_lut_4x4_5;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_5);
			break;
		case 6:
			tz_lut = calvc_tz_lut_4x4_6;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_6);
			break;
		case 7:
			tz_lut = calvc_tz_lut_4x4_7;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_7);
			break;
		case 8:
			tz_lut = calvc_tz_lut_4x4_8;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_8);
			break;
		case 9:
			tz_lut = calvc_tz_lut_4x4_9;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_9);
			break;
		case 10:
			tz_lut = calvc_tz_lut_4x4_10;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_10);
			break;
		case 11:
			tz_lut = calvc_tz_lut_4x4_11;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_11);
			break;
		case 12:
			tz_lut = calvc_tz_lut_4x4_12;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_12);
			break;
		case 13:
			tz_lut = calvc_tz_lut_4x4_13;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_13);
			break;
		case 14:
			tz_lut = calvc_tz_lut_4x4_14;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_14);
			break;
		case 15:
			tz_lut = calvc_tz_lut_4x4_15;
			tz_lut_size = ARRAY_SIZE(calvc_tz_lut_4x4_15);
			break;
		}
		break;
	}

	data = bitstream_read_u_no_inc(reader, 16);

	for (i = 0; i < tz_lut_size; i++) {
		unsigned lshift = tz_lut[i].vlc_len;
		unsigned rshift = 16 - tz_lut[i].vlc_len;
		uint32_t mask   = (1l << lshift) - 1;
		uint32_t val    = data;

		val >>= rshift;
		val &= mask;

		if (val == tz_lut[i].vlc) {
			zerosLeft = tz_lut[i].total_zeros;
			bitstream_read_u(reader, tz_lut[i].vlc_len);
			break;
		}
	}

	if (i == tz_lut_size) {
		SYNTAX_ERR("CALVC total_zeros LUT not matched\n");
	}

	return zerosLeft;
}

static unsigned run_before(bitstream_reader *reader, unsigned zerosLeft)
{
	const calvc_run_before_lut *rb_lut;
	unsigned rb_lut_size;
	unsigned runbefore;
	uint16_t data;

	switch (zerosLeft) {
	case 1:
		rb_lut = calvc_rb_lut_1;
		rb_lut_size = ARRAY_SIZE(calvc_rb_lut_1);
		break;
	case 2:
		rb_lut = calvc_rb_lut_2;
		rb_lut_size = ARRAY_SIZE(calvc_rb_lut_2);
		break;
	case 3:
		rb_lut = calvc_rb_lut_3;
		rb_lut_size = ARRAY_SIZE(calvc_rb_lut_3);
		break;
	case 4:
		rb_lut = calvc_rb_lut_4;
		rb_lut_size = ARRAY_SIZE(calvc_rb_lut_4);
		break;
	case 5:
		rb_lut = calvc_rb_lut_5;
		rb_lut_size = ARRAY_SIZE(calvc_rb_lut_5);
		break;
	case 6:
		rb_lut = calvc_rb_lut_6;
		rb_lut_size = ARRAY_SIZE(calvc_rb_lut_6);
		break;
	default:
		rb_lut = calvc_rb_lut_7;
		rb_lut_size = ARRAY_SIZE(calvc_rb_lut_7);
		break;
	}

	data = bitstream_read_u_no_inc(reader, 16);

	for (runbefore = 0; runbefore < rb_lut_size; runbefore++) {
		unsigned lshift = rb_lut[runbefore].vlc_len;
		unsigned rshift = 16 - rb_lut[runbefore].vlc_len;
		uint32_t mask   = (1l << lshift) - 1;
		uint32_t val    = data;

		val >>= rshift;
		val &= mask;

		if (val == rb_lut[runbefore].vlc) {
			bitstream_read_u(reader, rb_lut[runbefore].vlc_len);
			break;
		}
	}

	if (runbefore == rb_lut_size) {
		SYNTAX_ERR("CALVC run_before LUT not matched\n");
	}

	return runbefore;
}

signed residual_block_vlc(bitstream_reader *reader, int startIdx, int endIdx,
			  unsigned maxNumCoeff, signed coeffNum,
			  int16_t coeffLevel[16])
{
	unsigned trailingOnes;
	unsigned totalCoeff;
	unsigned zerosLeft = 0;
	unsigned suffixLength;
	unsigned runVal[16] = { 0 };
	signed levelVal[16] = { 0 };
	const calvc_level_lut *lvl_lut;
	int i;

	assert(endIdx < maxNumCoeff);
	assert(maxNumCoeff <= 16);

	lvl_lut = coeff_token_lut(reader, coeffNum);

	trailingOnes = lvl_lut->trailingOnes;
	totalCoeff   = lvl_lut->totalCoeff;

	assert(totalCoeff <= maxNumCoeff);

	SYNTAX_DPRINT("trailingOnes %u totalCoeff %u\n",
		      trailingOnes, totalCoeff);

	SYNTAX_DPRINT("res coeffNum %d\n", totalCoeff);

	if (totalCoeff == 0) {
		return 0;
	}

	for (i = 0; i < trailingOnes; i++) {
		unsigned neg = bitstream_read_u(reader, 1);

// 		SYNTAX_DPRINT("TrailingOne sign: %s\n", neg ? "-" : "+");

		levelVal[i] = neg ? -1 : 1;

		SYNTAX_DPRINT("levelVal[%d] = %d\n", i, levelVal[i]);
	}

	suffixLength = (totalCoeff > 10) && (trailingOnes < 3);

	SYNTAX_DPRINT("suffixLength %u\n", suffixLength);

	for (; i < totalCoeff; i++) {
		unsigned level_prefix = bitstream_skip_leading_zeros(reader);
		unsigned levelSuffixSize;
		unsigned level_suffix = 0;
		signed levelCode;

		SYNTAX_DPRINT("level_prefix %u\n", level_prefix);

		/* Baseline, Main and Extended profiles.  */
		if (level_prefix > 15) {
			SYNTAX_ERR("malformed level_prefix\n");
		}

		if (level_prefix == 14 && suffixLength == 0) {
			levelSuffixSize = 4;
		} else if (level_prefix >= 15) {
			levelSuffixSize = level_prefix - 3;
		} else {
			levelSuffixSize = suffixLength;
		}

		if (levelSuffixSize) {
			level_suffix = bitstream_read_u(reader, levelSuffixSize);
		}

		SYNTAX_DPRINT("levelSuffixSize %u\n", levelSuffixSize);
		SYNTAX_DPRINT("level_suffix %u\n", level_suffix);

		levelCode = (min(15, level_prefix) << suffixLength) + level_suffix;

		if (level_prefix >= 15 && suffixLength == 0) {
			levelCode += 15;
		}

		if (level_prefix >= 16) {
			levelCode += (1 << (level_prefix - 3)) - 4096;
		}

		if (i == trailingOnes && trailingOnes < 3) {
			levelCode += 2;
		}

		SYNTAX_DPRINT("levelCode %u\n", levelCode);

		if (levelCode % 2 == 0) {
			levelVal[i] = (levelCode + 2) >> 1;
		} else {
			levelVal[i] = (-levelCode - 1) >> 1;
		}

		SYNTAX_DPRINT("levelVal[%d] = %d\n", i, levelVal[i]);

		if (suffixLength == 0) {
			suffixLength = 1;
		}

		if ((abs(levelVal[i]) > (3 << (suffixLength - 1))) &&
			suffixLength < 6)
		{
			suffixLength++;
		}
	}

// 	SYNTAX_DPRINT("suffixLength %u\n", suffixLength);

// 	for (i = 0; i < 16; i++) {
// 		SYNTAX_DPRINT("levelVal[%d] = %d\n", i, levelVal[i]);
// 	}

	if (totalCoeff < endIdx - startIdx + 1) {
		zerosLeft = total_zeros(reader, totalCoeff, maxNumCoeff);
	}

	SYNTAX_DPRINT("zerosLeft %u\n", zerosLeft);

	for (i = 0; i < totalCoeff - 1; i++) {
		if (zerosLeft != 0) {
			runVal[i] = run_before(reader, zerosLeft);
		} else {
			runVal[i] = 0;
		}

		zerosLeft = zerosLeft - runVal[i];
	}

	runVal[i] = zerosLeft;

// 	for (i = 0; i < 16; i++) {
// 		SYNTAX_DPRINT("runVal[%d] = %d\n", i, runVal[i]);
// 	}

	bzero(coeffLevel, sizeof(*coeffLevel) * 16);

	coeffNum = -1;

	for (i = totalCoeff - 1; i >= 0; i--) {
		coeffNum += runVal[i] + 1;
		coeffLevel[startIdx + coeffNum] = levelVal[i];
	}

// 	for (i = 0; i < totalCoeff; i++) {
// 		SYNTAX_DPRINT("coeffLevel[%d] = %d\n", i, coeffLevel[i]);
// 	}

// 	SYNTAX_DPRINT("res coeffNum %d\n", totalCoeff);

	return totalCoeff;
}
