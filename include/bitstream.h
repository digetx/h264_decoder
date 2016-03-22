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

#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>

typedef struct bitstream_reader {
	const uint8_t *data_ptr;
	uint32_t bitstream_end;
	uint32_t data_offset;
	uint8_t bit_shift;
	uint8_t rbsp_mode;
	uint8_t error;
} bitstream_reader;

void bitstream_reader_selftest(void);
void bitstream_init(bitstream_reader *reader, void *data, uint32_t size);
void bitstream_reader_inc_offset(bitstream_reader *reader, uint32_t delta);
uint32_t bitstream_read_u(bitstream_reader *reader, uint8_t bits_nb);
uint32_t bitstream_read_ue(bitstream_reader *reader);
int32_t bitstream_read_se(bitstream_reader *reader);
uint32_t bitstream_read_next_word(bitstream_reader *reader);
uint8_t bitstream_read_rbsp_align(bitstream_reader *reader);
unsigned bitstream_skip_leading_zeros(bitstream_reader *reader);
uint32_t bitstream_read_u_no_inc(bitstream_reader *reader, uint8_t bits_nb);
#define bitstream_read_ae(reader)	0

#endif // BITSTREAM_H
