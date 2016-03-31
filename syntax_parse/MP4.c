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

#include "syntax_parse.h"

#include "common.h"

#define FOURCC(a, b, c, d)	(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define U32C(v)			\
	((v) >> 24) & 0xFF,	\
	((v) >> 16) & 0xFF,	\
	((v) >> 8) & 0xFF,	\
	(v) & 0xFF		\

static void read_atom_header(bitstream_reader *reader,
			     int64_t *size, uint32_t *type)
{
	*size = bitstream_read_u(reader, 32);
	*type = bitstream_read_u(reader, 32);

	SYNTAX_IPRINT("ATOM: \"%c%c%c%c\" size: 0x%X\n",
		      U32C(*type), (uint32_t) *size);

	*size -= 8;
	assert(*size >= 0);
}

static int is_MP4(bitstream_reader *reader)
{
	uint32_t size, ftyp, brand, version;

	size = bitstream_read_u(reader, 32);
	ftyp = bitstream_read_u(reader, 32);

	if ((size & 3) || (size < 8) || ftyp != FOURCC('f', 't', 'y', 'p')) {
		SYNTAX_WARN("MP4 header is malformed\n");
		return 0;
	}

	brand = bitstream_read_u(reader, 32);
	version = bitstream_read_u(reader, 32);

	SYNTAX_IPRINT("Possible MP4 header identified\n");
	SYNTAX_IPRINT("Brand: %c%c%c%c\n", U32C(brand));
	SYNTAX_IPRINT("Version: 0x%08X\n", version);

	size = (size >> 2) - 2;

	if (size != 0) {
		SYNTAX_IPRINT("Compatible with: ");
	}

	while (size--) {
		brand = bitstream_read_u(reader, 32);
		SYNTAX_IPRINT("%c%c%c%c%s", U32C(brand), size ? ", " : "\n");
	}

	return 1;
}

int parse_mp4(decoder_context *decoder)
{
	bitstream_reader *reader = &decoder->reader;
	uint32_t type;
	int64_t size;

	if (!is_MP4(reader)) {
		reader->bit_shift = reader->data_offset = 0;
		return 0;
	}

	while (!reader->error) {
		read_atom_header(reader, &size, &type);

		switch (type) {
		case FOURCC('m', 'd', 'a', 't'):
		{
			uint32_t orig_end = reader->bitstream_end;

			do {
				uint32_t NAL_size = bitstream_read_u(reader, 32);
				uint32_t prev_offt = reader->data_offset;

				reader->bitstream_end = prev_offt + NAL_size;

				SYNTAX_IPRINT("+++++++++++++++\n");
				SYNTAX_IPRINT("NAL size 0x%X\n", NAL_size);

				parse_NAL(decoder);

				SYNTAX_IPRINT("---------------\n\n");

				reader->data_offset = prev_offt + NAL_size;
				reader->bitstream_end += 4;
				reader->bit_shift = 0;

				size -= NAL_size + 4;
			} while (size > 0);

			reader->bitstream_end = orig_end;
			break;
		}
		default:
			break;
		}

		assert(size >= 0);

		if (size != 0) {
			bitstream_reader_inc_offset(reader, size);
		}
	}

	return 1;
}
