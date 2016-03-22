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

#ifndef CAVLC_LUTS_H
#define CAVLC_LUTS_H

typedef struct calvc_level_lut {
	unsigned trailingOnes:2;
	unsigned totalCoeff:5;
	unsigned coeff_token_len:5;
	unsigned coeff_token:6;
} calvc_level_lut;

typedef struct calvc_tz_lut {
	unsigned total_zeros:4;
	unsigned vlc_len:4;
	unsigned vlc:3;
} calvc_tz_lut;

typedef struct calvc_run_before_lut {
	unsigned vlc_len:4;
	unsigned vlc:3;
} calvc_run_before_lut;

static const calvc_level_lut coeff_token_nC_0_1[] = {
	{ 0,	0,	1,	0x1 },
	{ 0,	1,	6,	0x5 },
	{ 1,	1,	2,	0x1 },
	{ 0,	2,	8,	0x7 },
	{ 1,	2,	6,	0x4 },
	{ 2,	2,	3,	0x1 },
	{ 0,	3,	9,	0x7 },
	{ 1,	3,	8,	0x6 },
	{ 2,	3,	7,	0x5 },
	{ 3,	3,	5,	0x3 },
	{ 0,	4,	10,	0x7 },
	{ 1,	4,	9,	0x6 },
	{ 2,	4,	8,	0x5 },
	{ 3,	4,	6,	0x3 },
	{ 0,	5,	11,	0x7 },
	{ 1,	5,	10,	0x6 },
	{ 2,	5,	9,	0x5 },
	{ 3,	5,	7,	0x4 },
	{ 0,	6,	13,	0xF },
	{ 1,	6,	11,	0x6 },
	{ 2,	6,	10,	0x5 },
	{ 3,	6,	8,	0x4 },
	{ 0,	7,	13,	0xB },
	{ 1,	7,	13,	0xE },
	{ 2,	7,	11,	0x5 },
	{ 3,	7,	9,	0x4 },
	{ 0,	8,	13,	0x8 },
	{ 1,	8,	13,	0xA },
	{ 2,	8,	13,	0xD },
	{ 3,	8,	10,	0x4 },
	{ 0,	9,	14,	0xF },
	{ 1,	9,	14,	0xE },
	{ 2,	9,	13,	0x9 },
	{ 3,	9,	11,	0x4 },
	{ 0,	10,	14,	0xB },
	{ 1,	10,	14,	0xA },
	{ 2,	10,	14,	0xD },
	{ 3,	10,	13,	0xC },
	{ 0,	11,	15,	0xF },
	{ 1,	11,	15,	0xE },
	{ 2,	11,	14,	0x9 },
	{ 3,	11,	14,	0xC },
	{ 0,	12,	15,	0xB },
	{ 1,	12,	15,	0xA },
	{ 2,	12,	15,	0xD },
	{ 3,	12,	14,	0x8 },
	{ 0,	13,	16,	0xF },
	{ 1,	13,	15,	0x1 },
	{ 2,	13,	15,	0x9 },
	{ 3,	13,	15,	0xC },
	{ 0,	14,	16,	0xB },
	{ 1,	14,	16,	0xE },
	{ 2,	14,	16,	0xD },
	{ 3,	14,	15,	0x8 },
	{ 0,	15,	16,	0x7 },
	{ 1,	15,	16,	0xA },
	{ 2,	15,	16,	0x9 },
	{ 3,	15,	16,	0xC },
	{ 0,	16,	16,	0x4 },
	{ 1,	16,	16,	0x6 },
	{ 2,	16,	16,	0x5 },
	{ 3,	16,	16,	0x8 },
};

static const calvc_level_lut coeff_token_nC_2_3[] = {
	{ 0,	0,	2,	0x3 },
	{ 0,	1,	6,	0xB },
	{ 1,	1,	2,	0x2 },
	{ 0,	2,	6,	0x7 },
	{ 1,	2,	5,	0x7 },
	{ 2,	2,	3,	0x3 },
	{ 0,	3,	7,	0x7 },
	{ 1,	3,	6,	0xA },
	{ 2,	3,	6,	0x9 },
	{ 3,	3,	4,	0x5 },
	{ 0,	4,	8,	0x7 },
	{ 1,	4,	6,	0x6 },
	{ 2,	4,	6,	0x5 },
	{ 3,	4,	4,	0x4 },
	{ 0,	5,	8,	0x4 },
	{ 1,	5,	7,	0x6 },
	{ 2,	5,	7,	0x5 },
	{ 3,	5,	5,	0x6 },
	{ 0,	6,	9,	0x7 },
	{ 1,	6,	8,	0x6 },
	{ 2,	6,	8,	0x5 },
	{ 3,	6,	6,	0x8 },
	{ 0,	7,	11,	0xF },
	{ 1,	7,	9,	0x6 },
	{ 2,	7,	9,	0x5 },
	{ 3,	7,	6,	0x4 },
	{ 0,	8,	11,	0xB },
	{ 1,	8,	11,	0xE },
	{ 2,	8,	11,	0xD },
	{ 3,	8,	7,	0x4 },
	{ 0,	9,	12,	0xF },
	{ 1,	9,	11,	0xA },
	{ 2,	9,	11,	0x9 },
	{ 3,	9,	9,	0x4 },
	{ 0,	10,	12,	0xB },
	{ 1,	10,	12,	0xE },
	{ 2,	10,	12,	0xD },
	{ 3,	10,	11,	0xC },
	{ 0,	11,	12,	0x8 },
	{ 1,	11,	12,	0xA },
	{ 2,	11,	12,	0x9 },
	{ 3,	11,	11,	0x8 },
	{ 0,	12,	13,	0xF },
	{ 1,	12,	13,	0xE },
	{ 2,	12,	13,	0xD },
	{ 3,	12,	12,	0xC },
	{ 0,	13,	13,	0xB },
	{ 1,	13,	13,	0xA },
	{ 2,	13,	13,	0x9 },
	{ 3,	13,	13,	0xC },
	{ 0,	14,	13,	0x7 },
	{ 1,	14,	14,	0xB },
	{ 2,	14,	13,	0x6 },
	{ 3,	14,	13,	0x8 },
	{ 0,	15,	14,	0x9 },
	{ 1,	15,	14,	0x8 },
	{ 2,	15,	14,	0xA },
	{ 3,	15,	13,	0x1 },
	{ 0,	16,	14,	0x7 },
	{ 1,	16,	14,	0x6 },
	{ 2,	16,	14,	0x5 },
	{ 3,	16,	14,	0x4 },
};

static const calvc_level_lut coeff_token_nC_4_7[] = {
	{ 0,	0,	4,	0xF },
	{ 0,	1,	6,	0xF },
	{ 1,	1,	4,	0xE },
	{ 0,	2,	6,	0xB },
	{ 1,	2,	5,	0xF },
	{ 2,	2,	4,	0xD },
	{ 0,	3,	6,	0x8 },
	{ 1,	3,	5,	0xC },
	{ 2,	3,	5,	0xE },
	{ 3,	3,	4,	0xC },
	{ 0,	4,	7,	0xF },
	{ 1,	4,	5,	0xA },
	{ 2,	4,	5,	0xB },
	{ 3,	4,	4,	0xB },
	{ 0,	5,	7,	0xB },
	{ 1,	5,	5,	0x8 },
	{ 2,	5,	5,	0x9 },
	{ 3,	5,	4,	0xA },
	{ 0,	6,	7,	0x9 },
	{ 1,	6,	6,	0xE },
	{ 2,	6,	6,	0xD },
	{ 3,	6,	4,	0x9 },
	{ 0,	7,	7,	0x8 },
	{ 1,	7,	6,	0xA },
	{ 2,	7,	6,	0x9 },
	{ 3,	7,	4,	0x8 },
	{ 0,	8,	8,	0xF },
	{ 1,	8,	7,	0xE },
	{ 2,	8,	7,	0xD },
	{ 3,	8,	5,	0xD },
	{ 0,	9,	8,	0xB },
	{ 1,	9,	8,	0xE },
	{ 2,	9,	7,	0xA },
	{ 3,	9,	6,	0xC },
	{ 0,	10,	9,	0xF },
	{ 1,	10,	8,	0xA },
	{ 2,	10,	8,	0xD },
	{ 3,	10,	7,	0xC },
	{ 0,	11,	9,	0xB },
	{ 1,	11,	9,	0xE },
	{ 2,	11,	8,	0x9 },
	{ 3,	11,	8,	0xC },
	{ 0,	12,	9,	0x8 },
	{ 1,	12,	9,	0xA },
	{ 2,	12,	9,	0xD },
	{ 3,	12,	8,	0x8 },
	{ 0,	13,	10,	0xD },
	{ 1,	13,	9,	0x7 },
	{ 2,	13,	9,	0x9 },
	{ 3,	13,	9,	0xC },
	{ 0,	14,	10,	0x9 },
	{ 1,	14,	10,	0xC },
	{ 2,	14,	10,	0xB },
	{ 3,	14,	10,	0xA },
	{ 0,	15,	10,	0x5 },
	{ 1,	15,	10,	0x8 },
	{ 2,	15,	10,	0x7 },
	{ 3,	15,	10,	0x6 },
	{ 0,	16,	10,	0x1 },
	{ 1,	16,	10,	0x4 },
	{ 2,	16,	10,	0x3 },
	{ 3,	16,	10,	0x2 },
};

static const calvc_level_lut coeff_token_nC_8[] = {
	{ 0,	0,	6,	0x3 },
	{ 0,	1,	6,	0x0 },
	{ 1,	1,	6,	0x1 },
	{ 0,	2,	6,	0x4 },
	{ 1,	2,	6,	0x5 },
	{ 2,	2,	6,	0x6 },
	{ 0,	3,	6,	0x8 },
	{ 1,	3,	6,	0x9 },
	{ 2,	3,	6,	0xA },
	{ 3,	3,	6,	0xB },
	{ 0,	4,	6,	0xC },
	{ 1,	4,	6,	0xD },
	{ 2,	4,	6,	0xE },
	{ 3,	4,	6,	0xF },
	{ 0,	5,	6,	0x10 },
	{ 1,	5,	6,	0x11 },
	{ 2,	5,	6,	0x12 },
	{ 3,	5,	6,	0x13 },
	{ 0,	6,	6,	0x14 },
	{ 1,	6,	6,	0x15 },
	{ 2,	6,	6,	0x16 },
	{ 3,	6,	6,	0x17 },
	{ 0,	7,	6,	0x18 },
	{ 1,	7,	6,	0x19 },
	{ 2,	7,	6,	0x1A },
	{ 3,	7,	6,	0x1B },
	{ 0,	8,	6,	0x1C },
	{ 1,	8,	6,	0x1D },
	{ 2,	8,	6,	0x1E },
	{ 3,	8,	6,	0x1F },
	{ 0,	9,	6,	0x20 },
	{ 1,	9,	6,	0x21 },
	{ 2,	9,	6,	0x22 },
	{ 3,	9,	6,	0x23 },
	{ 0,	10,	6,	0x24 },
	{ 1,	10,	6,	0x25 },
	{ 2,	10,	6,	0x26 },
	{ 3,	10,	6,	0x27 },
	{ 0,	11,	6,	0x28 },
	{ 1,	11,	6,	0x29 },
	{ 2,	11,	6,	0x2A },
	{ 3,	11,	6,	0x2B },
	{ 0,	12,	6,	0x2C },
	{ 1,	12,	6,	0x2D },
	{ 2,	12,	6,	0x2E },
	{ 3,	12,	6,	0x2F },
	{ 0,	13,	6,	0x30 },
	{ 1,	13,	6,	0x31 },
	{ 2,	13,	6,	0x32 },
	{ 3,	13,	6,	0x33 },
	{ 0,	14,	6,	0x34 },
	{ 1,	14,	6,	0x35 },
	{ 2,	14,	6,	0x36 },
	{ 3,	14,	6,	0x37 },
	{ 0,	15,	6,	0x38 },
	{ 1,	15,	6,	0x39 },
	{ 2,	15,	6,	0x3A },
	{ 3,	15,	6,	0x3B },
	{ 0,	16,	6,	0x3C },
	{ 1,	16,	6,	0x3D },
	{ 2,	16,	6,	0x3E },
	{ 3,	16,	6,	0x3F },
};

static const calvc_level_lut coeff_token_nC_1[] = {
	{ 0,	0,	2,	0x1 },
	{ 0,	1,	6,	0x7 },
	{ 1,	1,	1,	0x1 },
	{ 0,	2,	6,	0x4 },
	{ 1,	2,	6,	0x6 },
	{ 2,	2,	3,	0x1 },
	{ 0,	3,	6,	0x3 },
	{ 1,	3,	7,	0x3 },
	{ 2,	3,	7,	0x2 },
	{ 3,	3,	6,	0x5 },
	{ 0,	4,	6,	0x2 },
	{ 1,	4,	8,	0x3 },
	{ 2,	4,	8,	0x2 },
	{ 3,	4,	7,	0x0 },
};

static const calvc_level_lut coeff_token_nC_2[] = {
	{ 0,	0,	1,	0x1 },
	{ 0,	1,	7,	0xF },
	{ 1,	1,	2,	0x1 },
	{ 0,	2,	7,	0xE },
	{ 1,	2,	7,	0xD },
	{ 2,	2,	3,	0x1 },
	{ 0,	3,	9,	0x7 },
	{ 1,	3,	7,	0xC },
	{ 2,	3,	7,	0xB },
	{ 3,	3,	5,	0x1 },
	{ 0,	4,	9,	0x6 },
	{ 1,	4,	9,	0x5 },
	{ 2,	4,	7,	0xA },
	{ 3,	4,	6,	0x1 },
	{ 0,	5,	10,	0x7 },
	{ 1,	5,	10,	0x6 },
	{ 2,	5,	9,	0x4 },
	{ 3,	5,	7,	0x9 },
	{ 0,	6,	11,	0x7 },
	{ 1,	6,	11,	0x6 },
	{ 2,	6,	10,	0x5 },
	{ 3,	6,	7,	0x8 },
	{ 0,	7,	12,	0x7 },
	{ 1,	7,	12,	0x6 },
	{ 2,	7,	11,	0x5 },
	{ 3,	7,	10,	0x4 },
	{ 0,	8,	13,	0x7 },
	{ 1,	8,	12,	0x5 },
	{ 2,	8,	12,	0x4 },
	{ 3,	8,	11,	0x4 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_1[] = {
	{ 0,	1,	0x1 },
	{ 1,	3,	0x3 },
	{ 2,	3,	0x2 },
	{ 3,	4,	0x3 },
	{ 4,	4,	0x2 },
	{ 5,	5,	0x3 },
	{ 6,	5,	0x2 },
	{ 7,	6,	0x3 },
	{ 8,	6,	0x2 },
	{ 9,	7,	0x3 },
	{ 10,	7,	0x2 },
	{ 11,	8,	0x3 },
	{ 12,	8,	0x2 },
	{ 13,	9,	0x3 },
	{ 14,	9,	0x2 },
	{ 15,	9,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_2[] = {
	{ 0,	3,	0x7 },
	{ 1,	3,	0x6 },
	{ 2,	3,	0x5 },
	{ 3,	3,	0x4 },
	{ 4,	3,	0x3 },
	{ 5,	4,	0x5 },
	{ 6,	4,	0x4 },
	{ 7,	4,	0x3 },
	{ 8,	4,	0x2 },
	{ 9,	5,	0x3 },
	{ 10,	5,	0x2 },
	{ 11,	6,	0x3 },
	{ 12,	6,	0x2 },
	{ 13,	6,	0x1 },
	{ 14,	6,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_3[] = {
	{ 0,	4,	0x5 },
	{ 1,	3,	0x7 },
	{ 2,	3,	0x6 },
	{ 3,	3,	0x5 },
	{ 4,	4,	0x4 },
	{ 5,	4,	0x3 },
	{ 6,	3,	0x4 },
	{ 7,	3,	0x3 },
	{ 8,	4,	0x2 },
	{ 9,	5,	0x3 },
	{ 10,	5,	0x2 },
	{ 11,	6,	0x1 },
	{ 12,	5,	0x1 },
	{ 13,	6,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_4[] = {
	{ 0, 	5,	0x3 },
	{ 1,	3,	0x7 },
	{ 2,	4,	0x5 },
	{ 3,	4,	0x4 },
	{ 4,	3,	0x6 },
	{ 5,	3,	0x5 },
	{ 6,	3,	0x4 },
	{ 7,	4,	0x3 },
	{ 8,	3,	0x3 },
	{ 9,	4,	0x2 },
	{ 10,	5,	0x2 },
	{ 11,	5,	0x1 },
	{ 12,	5,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_5[] = {
	{ 0,	4,	0x5 },
	{ 1,	4,	0x4 },
	{ 2,	4,	0x3 },
	{ 3,	3,	0x7 },
	{ 4,	3,	0x6 },
	{ 5,	3,	0x5 },
	{ 6,	3,	0x4 },
	{ 7,	3,	0x3 },
	{ 8,	4,	0x2 },
	{ 9,	5,	0x1 },
	{ 10,	4,	0x1 },
	{ 11,	5,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_6[] = {
	{ 0,	6,	0x1 },
	{ 1,	5,	0x1 },
	{ 2,	3,	0x7 },
	{ 3,	3,	0x6 },
	{ 4,	3,	0x5 },
	{ 5,	3,	0x4 },
	{ 6,	3,	0x3 },
	{ 7,	3,	0x2 },
	{ 8,	4,	0x1 },
	{ 9,	3,	0x1 },
	{ 10,	6,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_7[] = {
	{ 0,	6,	0x1 },
	{ 1,	5,	0x1 },
	{ 2,	3,	0x5 },
	{ 3,	3,	0x4 },
	{ 4,	3,	0x3 },
	{ 5,	2,	0x3 },
	{ 6,	3,	0x2 },
	{ 7,	4,	0x1 },
	{ 8,	3,	0x1 },
	{ 9,	6,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_8[] = {
	{ 0,	6,	0x1 },
	{ 1,	4,	0x1 },
	{ 2,	5,	0x1 },
	{ 3,	3,	0x3 },
	{ 4,	2,	0x3 },
	{ 5,	2,	0x2 },
	{ 6,	3,	0x2 },
	{ 7,	3,	0x1 },
	{ 8,	6,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_9[] = {
	{ 0,	6,	0x1 },
	{ 1,	6,	0x0 },
	{ 2,	4,	0x1 },
	{ 3,	2,	0x3 },
	{ 4,	2,	0x2 },
	{ 5,	3,	0x1 },
	{ 6,	2,	0x1 },
	{ 7,	5,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_10[] = {
	{ 0,	5,	0x1 },
	{ 1,	5,	0x0 },
	{ 2,	3,	0x1 },
	{ 3, 	2,	0x3 },
	{ 4,	2,	0x2 },
	{ 5,	2,	0x1 },
	{ 6,	4,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_11[] = {
	{ 0,	4,	0x0 },
	{ 1,	4,	0x1 },
	{ 2,	3,	0x1 },
	{ 3,	3,	0x2 },
	{ 4,	1,	0x1 },
	{ 5,	3,	0x3 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_12[] = {
	{ 0,	4,	0x0 },
	{ 1,	4,	0x1 },
	{ 2,	2,	0x1 },
	{ 3,	1,	0x1 },
	{ 4,	3,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_13[] = {
	{ 0,	3,	0x0 },
	{ 1,	3,	0x1 },
	{ 2,	1,	0x1 },
	{ 3,	2,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_14[] = {
	{ 0,	2,	0x0 },
	{ 1,	2,	0x1 },
	{ 2,	1,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_4x4_15[] = {
	{ 0,	1,	0x0 },
	{ 1,	1,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_2x2_1[] = {
	{ 0,	1,	0x1 },
	{ 1,	2,	0x1 },
	{ 2,	3,	0x1 },
	{ 3,	3,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_2x2_2[] = {
	{ 0,	1,	0x1 },
	{ 1,	2,	0x1 },
	{ 2,	2,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_2x2_3[] = {
	{ 0,	1,	0x1 },
	{ 1,	1,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_2x4_1[] = {
	{ 0,	1,	0x1 },
	{ 1,	3,	0x2 },
	{ 2,	3,	0x3 },
	{ 3,	4,	0x2 },
	{ 4,	4,	0x3 },
	{ 5,	4,	0x1 },
	{ 6,	5,	0x1 },
	{ 7,	5,	0x0 },
};

static const calvc_tz_lut calvc_tz_lut_2x4_2[] = {
	{ 0,	3,	0x0 },
	{ 1,	2,	0x1 },
	{ 2,	3,	0x1 },
	{ 3,	3,	0x4 },
	{ 4,	3,	0x5 },
	{ 5,	3,	0x6 },
	{ 6,	3,	0x7 },
};

static const calvc_tz_lut calvc_tz_lut_2x4_3[] = {
	{ 0,	3,	0x0 },
	{ 1,	3,	0x1 },
	{ 2,	2,	0x1 },
	{ 3,	2,	0x2 },
	{ 4,	3,	0x6 },
	{ 5,	3,	0x7 },
};

static const calvc_tz_lut calvc_tz_lut_2x4_4[] = {
	{ 0,	3,	0x6 },
	{ 1,	2,	0x0 },
	{ 2,	2,	0x1 },
	{ 3,	2,	0x2 },
	{ 4,	3,	0x7 },
};

static const calvc_tz_lut calvc_tz_lut_2x4_5[] = {
	{ 0,	2,	0x0 },
	{ 1,	2,	0x1 },
	{ 2,	2,	0x2 },
	{ 3,	2,	0x3 },
};

static const calvc_tz_lut calvc_tz_lut_2x4_6[] = {
	{ 0,	2,	0x0 },
	{ 1,	2,	0x1 },
	{ 2,	1,	0x1 },
};

static const calvc_tz_lut calvc_tz_lut_2x4_7[] = {
	{ 0,	1,	0x0 },
	{ 1,	1,	0x1 },
};

static const calvc_run_before_lut calvc_rb_lut_1[] = {
	{ 1,	0x1 },
	{ 1,	0x0 },
};

static const calvc_run_before_lut calvc_rb_lut_2[] = {
	{ 1,	0x1 },
	{ 2,	0x1 },
	{ 2,	0x0 },
};

static const calvc_run_before_lut calvc_rb_lut_3[] = {
	{ 2,	0x3 },
	{ 2,	0x2 },
	{ 2,	0x1 },
	{ 2,	0x0 },
};

static const calvc_run_before_lut calvc_rb_lut_4[] = {
	{ 2,	0x3 },
	{ 2,	0x2 },
	{ 2,	0x1 },
	{ 3,	0x1 },
	{ 3,	0x0 },
};

static const calvc_run_before_lut calvc_rb_lut_5[] = {
	{ 2,	0x3 },
	{ 2,	0x2 },
	{ 3,	0x3 },
	{ 3,	0x2 },
	{ 3,	0x1 },
	{ 3,	0x0 },
};

static const calvc_run_before_lut calvc_rb_lut_6[] = {
	{ 2,	0x3 },
	{ 3,	0x0 },
	{ 3,	0x1 },
	{ 3,	0x3 },
	{ 3,	0x2 },
	{ 3,	0x5 },
	{ 3,	0x4 },
};

static const calvc_run_before_lut calvc_rb_lut_7[] = {
	{ 3,	0x7 },
	{ 3,	0x6 },
	{ 3,	0x5 },
	{ 3,	0x4 },
	{ 3,	0x3 },
	{ 3,	0x2 },
	{ 3,	0x1 },
	{ 4,	0x1 },
	{ 5,	0x1 },
	{ 6,	0x1 },
	{ 7,	0x1 },
	{ 8,	0x1 },
	{ 9,	0x1 },
	{ 10,	0x1 },
	{ 11,	0x1 },
};

#endif // CAVLC_LUTS_H
