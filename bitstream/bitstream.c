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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "bitstream.h"

// #define BITSTREAM_DEBUG

#ifdef BITSTREAM_DEBUG
#define BITSTREAM_DPRINT(f, ...)	printf(f, ## __VA_ARGS__)
#else
#define BITSTREAM_DPRINT(...)	{}
#endif

#define BITSTREAM_ERR(f, ...)						\
{									\
	fprintf(stderr, "%s:%d:\n", __FILE__, __LINE__);		\
	fprintf(stderr, "bitstream_reader: " f, ## __VA_ARGS__);	\
	reader->error = 1;						\
	exit(EXIT_FAILURE);						\
}

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Unsupported host endianness"
#endif

#ifndef clz
#define clz	__builtin_clz
#endif

void bitstream_reader_selftest(void)
{
	uint8_t test_data[] = { 0x0F, 0xFF, 0x03, 0x10, 0x90, 0x7F };
	bitstream_reader reader;
	uint64_t test = htobe64(0xAAAAAAAAAAAAAAAA);
	uint32_t val;
	int i;

	bitstream_init(&reader, test_data, sizeof(test_data));

	val = bitstream_read_ue(&reader);

	printf("codenum = %u\n", val);

	assert(val == 30);

	val = bitstream_read_ue(&reader);

	printf("codenum = %u\n", val);

	assert(val == 0);

	bitstream_read_u(&reader, 6);

	val = bitstream_read_ue(&reader);

	printf("codenum = %u\n", val);

	assert(val == 97);

	reader.data_offset = 3;
	reader.bit_shift = 4;

	val = bitstream_read_ue(&reader);

	printf("codenum = %u\n", val);

	assert(val == 17);

	val = bitstream_read_ue(&reader);

	printf("codenum = %u\n", val);

	assert(val == 30);

	bitstream_init(&reader, &test, sizeof(test));

	assert(bitstream_read_u_no_inc(&reader, 16) == 0xAAAA);

	for (i = 0; i < 64; i++) {
		unsigned cmp = ((be64toh(test) >> (63 - i))) & 1;
		printf("i = %d cmp 0x%X\n", i, cmp);
		assert(bitstream_read_u(&reader, 1) == cmp);
	}

	bitstream_init(&reader, &test, sizeof(test));

	test = htobe64(0x0123456789ABCDEF);

	for (i = 0; i < 12; i++) {
		unsigned cmp = ((be64toh(test) >> (64 - 5 * (i + 1)))) & 31;
		printf("i = %d cmp 0x%X\n", i, cmp);
		assert(bitstream_read_u(&reader, 5) == cmp);
	}

	bitstream_init(&reader, &test, sizeof(test));

	for (i = 0; i < 16; i++) {
		unsigned cmp = ((be64toh(test) >> (64 - 4 * (i + 1)))) & 15;
		printf("i = %d cmp 0x%X\n", i, cmp);
		assert(bitstream_read_u(&reader, 4) == cmp);
	}

	bitstream_init(&reader, &test, sizeof(test));
	reader.bit_shift = 1;

	for (i = 0; i < 15; i++) {
		unsigned cmp = (((be64toh(test) << 1) >> (64 - 4 * (i + 1)))) & 15;
		printf("i = %d cmp 0x%X\n", i, cmp);
		assert(bitstream_read_u(&reader, 4) == cmp);
	}

	printf("%s passed\n", __func__);
}

void bitstream_init(bitstream_reader *reader, void *data, uint32_t size)
{
	reader->data_ptr = data;
	reader->bitstream_end = size;
	reader->data_offset = 0;
	reader->bit_shift = 0;
	reader->rbsp_mode = 0;
	reader->error = 0;
}

static int check_range(bitstream_reader *reader, uint32_t offset)
{
	if (reader->data_offset + offset > reader->bitstream_end) {
		BITSTREAM_ERR("Reached data stream end\n");
	}

	return 0;
}

inline void bitstream_reader_inc_offset(bitstream_reader *reader, uint32_t delta)
{
	reader->data_offset += delta;
}

uint8_t bitstream_read_rbsp_align(bitstream_reader *reader)
{
	if (reader->bit_shift == 0) {
		return 0;
	}

	return bitstream_read_u(reader, reader->bit_shift);
}

uint32_t bitstream_read_next_word(bitstream_reader *reader)
{
	uint32_t offset = reader->data_offset;
	int align = (reader->bit_shift == 0) ? 0 : 1;

	if (check_range(reader, align + 4) != 0) {
		return 0;
	}

	return *((uint32_t *)(reader->data_ptr + offset + align));
}

static uint8_t bitstream_read_u8_no_inc(bitstream_reader *reader);

static uint8_t emulation_escape(bitstream_reader *reader, uint32_t offset,
				uint8_t data, int inc_offset, int *escaped)
{
	uint32_t seq;

	if (data != 0x03 || !reader->rbsp_mode) {
		return data;
	}

	if (offset < 2 || offset == reader->bitstream_end) {
		return data;
	}

	seq = *((uint32_t *)(reader->data_ptr + offset - 2));
	seq = be32toh(seq);

	switch (seq) {
	case 0x00000300:
	case 0x00000301:
	case 0x00000302:
	case 0x00000303:
		BITSTREAM_DPRINT("0x%08X escaped!\n", seq);
		if (inc_offset) {
			reader->data_offset++;
		}
		if (escaped != NULL) {
			*escaped = 1;
		}
		return seq & 0xFF;
	default:
		break;
	}

	return data;
}

static uint8_t bitstream_read_u8_no_inc(bitstream_reader *reader)
{
	uint8_t ret;

	if (reader->error) {
		return 0;
	}

	if (check_range(reader, 1) != 0) {
		return 0;
	}

	ret = *(reader->data_ptr + reader->data_offset);

	return emulation_escape(reader, reader->data_offset, ret, 1, NULL);
}

static uint32_t bitstream_read_bits(bitstream_reader *reader, uint8_t bits_nb,
				    int inc_offset)
{
	const uint8_t bit_shift = reader->bit_shift;
	const uint8_t *data_ptr = reader->data_ptr;
	uint32_t data_offset = reader->data_offset;
	const uint64_t mask = (1ll << bits_nb) - 1;
	uint8_t bytes_to_read = (bits_nb + bit_shift - 1) >> 3;
	const uint8_t rshift = 8 * (bytes_to_read + 1) - (bit_shift + bits_nb);
	int escape_inc_offset = 0;
	uint64_t ret = 0;

	assert(bits_nb != 0);
	assert(bits_nb <= 32);

	if (check_range(reader, bytes_to_read) != 0) {
		return 0;
	}

	BITSTREAM_DPRINT("===\n");
	BITSTREAM_DPRINT("bit_shift %u bits_nb %u bytes_to_read %u\n",
			 bit_shift, bits_nb, bytes_to_read);

	do {
		uint8_t byte = *(data_ptr + data_offset);
		uint8_t lshift = bytes_to_read << 3;
		int escaped = 0;

		byte = emulation_escape(reader, data_offset++, byte,
					!escape_inc_offset || inc_offset,
					&escaped);

		if (escaped && escape_inc_offset && !inc_offset) {
			data_offset++;
		}

		escape_inc_offset = 1;

		ret |= (uint64_t) byte << lshift;

		BITSTREAM_DPRINT("0x%02X lshift %u 0x%lX\n",
				 byte, lshift, ret);
	} while (bytes_to_read--);

	BITSTREAM_DPRINT("ret 0x%lX\n", ret);
	BITSTREAM_DPRINT("rshift %u\n", rshift);
	BITSTREAM_DPRINT("mask 0x%lX\n", mask);

	ret >>= rshift;
	ret &= mask;

	BITSTREAM_DPRINT("ret 0x%lX\n", ret);
	BITSTREAM_DPRINT("===\n\n");

	return ret;
}

static void bitstream_reader_inc_offset_b(bitstream_reader *reader,
					  uint8_t bits_nb)
{
	uint8_t bit_shift = reader->bit_shift;

	reader->data_offset += (bit_shift + bits_nb) >> 3;
	reader->bit_shift = (bit_shift + bits_nb) % 8;
}

uint32_t bitstream_read_u_no_inc(bitstream_reader *reader, uint8_t bits_nb)
{
	uint32_t ret;

	if (reader->bit_shift == 0 && bits_nb == 8) {
		ret = bitstream_read_u8_no_inc(reader);
	} else {
		ret = bitstream_read_bits(reader, bits_nb, 0);
	}

	return ret;
}

uint32_t bitstream_read_u(bitstream_reader *reader, uint8_t bits_nb)
{
	uint32_t ret;

	if (reader->bit_shift == 0 && bits_nb == 8) {
		ret = bitstream_read_u8_no_inc(reader);
		bitstream_reader_inc_offset(reader, 1);
	} else {
		ret = bitstream_read_bits(reader, bits_nb, 1);
		bitstream_reader_inc_offset_b(reader, bits_nb);
	}

	return ret;
}

unsigned bitstream_skip_leading_zeros(bitstream_reader *reader)
{
	const uint8_t bit_shift = reader->bit_shift;
	uint8_t leading_zeros_align = 0;
	uint8_t leading_zeros = 0;

	if (bit_shift != 0 && !reader->error) {
		uint8_t byte = bitstream_read_bits(reader, 8 - bit_shift, 0);

		if (byte != 0) {
			leading_zeros_align = clz(byte) - 24 - bit_shift;
		} else {
			leading_zeros_align = 8 - bit_shift;
		}

		BITSTREAM_DPRINT("byte 0x%X leading_zeros_align %u\n",
				 byte, leading_zeros_align);

		if (byte != 0) {
			reader->bit_shift += leading_zeros_align;

			bitstream_reader_inc_offset_b(reader, 1);

			return leading_zeros_align;
		}

		bitstream_reader_inc_offset_b(reader, leading_zeros_align);
	}

	for (;;) {
		uint8_t byte = bitstream_read_u8_no_inc(reader);

		leading_zeros += byte ? clz(byte) - 24 : 8;

		BITSTREAM_DPRINT("byte 0x%X leading_zeros %u\n",
				 byte, leading_zeros);

		if (byte != 0) {
			reader->bit_shift += leading_zeros % 8;

			bitstream_reader_inc_offset_b(reader, 1);

			leading_zeros += leading_zeros_align;

			BITSTREAM_DPRINT("leading_zeros %u\n", leading_zeros);

			return leading_zeros;
		}

		bitstream_reader_inc_offset(reader, 1);
	}

	return 0;
}

static uint32_t exp_golomb_codenum(uint8_t exp, uint16_t val)
{
	uint32_t ret = (1l << exp) - 1 + val;

	BITSTREAM_DPRINT("exp %u val %u ret %u\n", exp, val, ret);

	return ret;
}

uint32_t bitstream_read_ue(bitstream_reader *reader)
{
	uint8_t leading_zeros;
	uint16_t val = 0;

	leading_zeros = bitstream_skip_leading_zeros(reader);

	if (leading_zeros > 16) {
		BITSTREAM_ERR("Exp-golomb parse error\n");
	}

	if (leading_zeros) {
		val = bitstream_read_u(reader, leading_zeros);
	}

	return exp_golomb_codenum(leading_zeros, val);
}

int32_t bitstream_read_se(bitstream_reader *reader)
{
	uint32_t ue = bitstream_read_ue(reader);
	uint32_t val = (ue > 1) ? (ue >> 1) + (ue & 1) : ue;
	int positive = ue & 1;

	return positive ? val : -val;
}
