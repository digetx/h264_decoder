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

#define NAL_START_CODE	0x000001

static const char * NAL_TYPE(int type)
{
	switch (type) {
	case 0:		return "Undefined";
	case 1:		return "Slice layer without partitioning non IDR";
	case 2:		return "Slice data partition A layer";
	case 3:		return "Slice data partition B layer";
	case 4:		return "Slice data partition C layer";
	case 5:		return "Slice layer without partitioning IDR";
	case 6:		return "Supplemental enhancement information (SEI)";
	case 7:		return "Sequence parameter set (SPS)";
	case 8:		return "Picture parameter set (PPS)";
	case 9:		return "Access Unit Delimiter";
	case 10:	return "End of sequence";
	case 11:	return "End of stream";
	case 12:	return "Filler data";
	case 13:	return "Sequence parameter set extension";
	case 14:	return "Prefix NAL unit";
	case 15:	return "Subset sequence parameter set";
	case 16:	return "Depth parameter set";
	case 17 ... 18:	return "Reserved";
	case 19:	return "Coded slice of an auxiliary coded picture " \
				"without partitioning";
	case 20:	return "Coded slice extension";
	case 21:	return "Coded slice extension for a depth view " \
				"component or a 3D-AVC texture view component";
	case 22 ... 23:	return "Reserved";
	case 24 ... 31:	return "Undefined";
	default:	return "Bad value";
	}
}

int is_NAL_start_code(bitstream_reader *reader)
{
	uint32_t data = bitstream_read_next_word(reader);

	return (be32toh(data) >> 8 == NAL_START_CODE);
}

int seek_to_NAL_start(bitstream_reader *reader)
{
	int NAL_found = 0;

	SYNTAX_IPRINT("Searching for the NAL ...\n");

	reader->bit_shift = 0;

	do {
		NAL_found = is_NAL_start_code(reader);

		if (reader->error) {
			return 0;
		}

		bitstream_reader_inc_offset(reader, NAL_found ? 3 : 1);

		if (NAL_found) {
			SYNTAX_IPRINT("found NAL_start_code at offset 0x%X\n",
				      reader->data_offset);
		}
	} while (!NAL_found);

	return NAL_found;
}

void parse_NAL(decoder_context *decoder)
{
	bitstream_reader *reader = &decoder->reader;
	unsigned forbidden_zero_bit;

	reader->rbsp_mode = 1;

	forbidden_zero_bit     = bitstream_read_u(reader, 1);
	decoder->nal.ref_idc   = bitstream_read_u(reader, 2);
	decoder->nal.unit_type = bitstream_read_u(reader, 5);

	SYNTAX_IPRINT("forbidden_zero_bit = %u\n", forbidden_zero_bit);
	SYNTAX_IPRINT("ref idc = %u\n", decoder->nal.ref_idc);
	SYNTAX_IPRINT("type %u = \"%s\"\n", decoder->nal.unit_type,
		      NAL_TYPE(decoder->nal.unit_type));

	if (forbidden_zero_bit != 0) {
		SYNTAX_ERR("NAL is malformed\n");
	}

	switch (decoder->nal.unit_type) {
	case 5:
		parse_slice_header(decoder);
		parse_slice_data(decoder);
		break;
	case 7:
		parse_SPS(decoder);
		break;
	case 8:
		parse_PPS(decoder);
		break;
	default:
		break;
	}

	reader->rbsp_mode = 0;
}
