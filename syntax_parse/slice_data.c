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

#define MbaffFrameFlag	\
	(decoder->active_sps->mb_adaptive_frame_field_flag && !decoder->sh.field_pic_flag)

static int is_end_of_NAL(bitstream_reader *reader, int NAL_start_delim)
{
	int no_more_data = (reader->data_offset >= reader->bitstream_end);

	if (!NAL_start_delim) {
		return no_more_data;
	}

	return no_more_data || is_NAL_start_code(reader);
}

int more_rbsp_data(decoder_context *decoder)
{
	bitstream_reader *reader = &decoder->reader;
	uint32_t data_offset = reader->data_offset;
	uint8_t bit_shift = reader->bit_shift;

	/* Check stop bit.  */
	if (bitstream_read_u(reader, 1) == 0) {
		goto more_data;
	}

	reader->rbsp_mode = 0;

	if (bitstream_read_rbsp_align(reader) != 0) {
		goto more_data;
	}

	while (!is_end_of_NAL(reader, decoder->NAL_start_delim)) {
		if (bitstream_read_u(reader, 8) != 0) {
			goto more_data;
		}
	}

	return 0;

more_data:
	reader->rbsp_mode = 1;
	reader->data_offset = data_offset;
	reader->bit_shift = bit_shift;

	return 1;
}

static unsigned NextMbAddress(decoder_context *decoder, unsigned n)
{
	unsigned i = n + 1;

	if (decoder->active_pps->num_slice_groups_minus1 != 0) {
		SYNTAX_ERR("Slice grouping unimplemented\n");
	}

	return i;
}

void rescale_levels(decoder_context *decoder, unsigned mb_id);

void parse_slice_data(decoder_context *decoder)
{
	bitstream_reader *reader = &decoder->reader;
	decoder_context_pps *pps = decoder->active_pps;
	unsigned CurrMbAddr = decoder->sh.first_mb_in_slice * (1 + MbaffFrameFlag);
	unsigned mb_alloc_nb = 1;
	unsigned moreDataFlag = 1;
	unsigned prevMbSkipped = 0;
	unsigned pic_width_in_mbs;
	unsigned pic_height_in_mbs;
	int i;

	decoder_reset_SD(decoder);

	if (CABAC_MODE) {
		SYNTAX_ERR("CABAC unimplemented\n");
	}

	decoder->get_mb_slice_constraint = 1;

	pic_width_in_mbs = decoder->active_sps->pic_width_in_mbs_minus1 + 1;
	pic_height_in_mbs = decoder->active_sps->pic_height_in_map_units_minus1 + 1;

	decoder->frames[0]->macroblocks = realloc(decoder->frames[0]->macroblocks,
		sizeof(macroblock) * pic_width_in_mbs * pic_height_in_mbs);
	assert(decoder->frames[0]->macroblocks != NULL);

	decoder->sd.macroblocks = &decoder->frames[0]->macroblocks[CurrMbAddr];

	do {
		SYNTAX_IPRINT("---- parsing MacroBlock id = %u ----\n", CurrMbAddr);

		switch (decoder->sh.slice_type) {
		default:
			if (!CABAC_MODE) {
				decoder->sd.mb_skip_run = bitstream_read_ue(reader);

				SYNTAX_IPRINT("mb_skip_run = %u\n",
					      decoder->sd.mb_skip_run);

				prevMbSkipped = !!decoder->sd.mb_skip_run;

				for (i = 0; i < decoder->sd.mb_skip_run; i++) {
					CurrMbAddr = NextMbAddress(decoder, CurrMbAddr);
				}

				if (decoder->sd.mb_skip_run) {
					moreDataFlag = more_rbsp_data(decoder);
				}
			} else {
				decoder->sd.mb_skip_run = bitstream_read_ae(reader);
				moreDataFlag = !decoder->sd.mb_skip_run;
			}
		case I:
		case SI:
			if (moreDataFlag) {
				if (MbaffFrameFlag && (CurrMbAddr % 2 == 0 ||
					(CurrMbAddr % 2 == 1 && prevMbSkipped)))
				{
					if (!CABAC_MODE) {
						decoder->sd.mb_field_decoding_flag =
								bitstream_read_u(reader, 1);
					} else {
						decoder->sd.mb_field_decoding_flag =
								bitstream_read_ae(reader);
					}

					SYNTAX_IPRINT("mb_field_decoding_flag = %u\n",
						      decoder->sd.mb_field_decoding_flag);
				}

				macroblock_layer(decoder, CurrMbAddr);
			}
			break;
		}

		if (!CABAC_MODE) {
			moreDataFlag = more_rbsp_data(decoder);
		} else {
// 			if( slice_type != I && slice_type != SI )
// 			prevMbSkipped = mb_skip_flag
// 			if( MbaffFrameFlag && CurrMbAddr % 2 = = 0 )
// 			moreDataFlag = 1
// 			else {
// 				end_of_slice_flag
// 				moreDataFlag = !end_of_slice_flag
// 			}
		}

		CurrMbAddr = NextMbAddress(decoder, CurrMbAddr);
	} while (moreDataFlag);

	SYNTAX_DPRINT("No moreDataFlag %d 0x%X\n",
		      moreDataFlag, reader->data_offset);

	decode_current_slice(decoder, CurrMbAddr);
}
