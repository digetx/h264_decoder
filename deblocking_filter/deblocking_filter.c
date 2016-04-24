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
#include <stdlib.h>

#include "decoder.h"
#include "syntax_parse.h"
#include "syntax_parse/common.h"
#include "transforms/common.h"

static const int table_alpha[52] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 5, 6, 7, 8, 9,
	10, 12, 13, 15, 17, 20, 22, 25, 28, 32, 36, 40, 45, 50, 56, 63, 71, 80,
	90, 101, 113, 127, 144, 162, 182, 203, 226, 255, 255,
};

static const int table_beta[52] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 3, 3, 3, 3, 4,
	4, 4, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
	15, 15, 16, 16, 17, 17, 18, 18,
};

static const int table_tC0[3][52] = {
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4,
		4, 5, 6, 6, 7, 8, 9, 10, 11, 13,
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5,
		6, 7, 8, 8, 10, 11, 12, 13, 15, 17,
	},
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 6, 6, 7, 8,
		9, 10, 11, 13, 14, 16, 18, 20, 23, 25,
	},
};

static int is_intra_mb(macroblock *mb)
{
	return mb->slice_type == I;
}

static void apply_filter_bs4(uint8_t *p[4], uint8_t *q[4], unsigned alpha,
			     unsigned beta, int chromaStyleFilteringFlag)
{
	int32_t p0 = *p[0], p1 = *p[1], p2 = *p[2], p3 = *p[3];
	int32_t q0 = *q[0], q1 = *q[1], q2 = *q[2], q3 = *q[3];

	if (!chromaStyleFilteringFlag &&
		abs(p2 - p0) < beta && abs(p0 - q0) < ((alpha >> 2) + 2))
	{
		*p[0] = (p2 + 2 * p1 + 2 * p0 + 2 * q0 + q1 + 4) >> 3;
		*p[1] = (p2 + p1 + p0 + q0 + 2) >> 2;
		*p[2] = (2 * p3 + 3 * p2 + p1 + p0 + q0 + 4) >> 3;
	} else {
		*p[0] = (2 * p1 + p0 + q1 + 2) >> 2;
	}

	if (!chromaStyleFilteringFlag &&
		abs(q2 - q0) < beta && abs(p0 - q0) < ((alpha >> 2) + 2))
	{
		*q[0] = (p1 + 2 * p0 + 2 * q0 + 2 * q1 + q2 + 4) >> 3;
		*q[1] = (p0 + q0 + q1 + q2 + 2) >> 2;
		*q[2] = (2 * q3 + 3 * q2 + q1 + q0 + p0 + 4) >> 3;
	} else {
		*q[0] = (2 * q1 + q0 + p1 + 2) >> 2;
	}
}

static void apply_filter_bs1_3(uint8_t *p[4], uint8_t *q[4], unsigned beta,
			       int bit_depth, int chromaStyleFilteringFlag,
			       int tC0)
{
	int32_t p0 = *p[0], p1 = *p[1], p2 = *p[2];
	int32_t q0 = *q[0], q1 = *q[1], q2 = *q[2];
	unsigned ap = abs(p2 - p0);
	unsigned aq = abs(q2 - q0);
	int32_t delta;
	int tC;

	if (chromaStyleFilteringFlag) {
		tC = tC0 + 1;
	} else {
		tC = tC0 + ((ap < beta) ? 1 : 0) + ((aq < beta) ? 1 : 0);
	}

	delta = Clip3(-tC, tC, ((((q0 - p0) << 2) + (p1 - q1) + 4) >> 3));
	*p[0] = Clip1(p0 + delta);
	*q[0] = Clip1(q0 - delta);

	if (!chromaStyleFilteringFlag && (ap < beta)) {
		*p[1] = p1 + Clip3(-tC0, tC0,
				   (p2 + ((p0 + q0 + 1) >> 1) - (p1 << 1)) >> 1);
	}

	if (!chromaStyleFilteringFlag && (aq < beta)) {
		*q[1] = q1 + Clip3(-tC0, tC0,
				   (q2 + ((p0 + q0 + 1) >> 1) - (q1 << 1)) >> 1);
	}
}

static void apply_filter(uint8_t *p[4], uint8_t *q[4], unsigned alpha,
			 unsigned beta, unsigned boundary_strength,
			 int bit_depth, int chromaStyleFilteringFlag, int tC0)
{
	if (boundary_strength == 4) {
		apply_filter_bs4(p, q, alpha, beta, chromaStyleFilteringFlag);
	} else {
		apply_filter_bs1_3(p, q, beta, bit_depth,
				   chromaStyleFilteringFlag, tC0);
	}
}

static int filterSamplesFlag(unsigned alpha, unsigned beta, int p0, int p1,
			     int q0, int q1)
{
	return (abs(p0 - q0) < alpha && abs(p1 - p0) < beta && abs(q1 - q0) < beta);
}

static void get_alpha_beta_tC0(decoder_context *decoder, int QP, int QP2,
			       int bs, int *alpha, int *beta, int *tC0)
{
	int qPav = (QP + QP2 + 1) >> 1;
	int bit_depth_minus8;
	int indexA, indexB;

	indexA = Clip3(0, 51, qPav + (decoder->sh.slice_alpha_c0_offset_div2 << 1));
	indexB = Clip3(0, 51, qPav + (decoder->sh.slice_beta_offset_div2 << 1));

	bit_depth_minus8 = decoder->active_sps->bit_depth_luma_minus8;

	*alpha = table_alpha[indexA] * (1 << bit_depth_minus8);
	*beta = table_beta[indexB] * (1 << bit_depth_minus8);

	if (bs < 4) {
		*tC0 = table_tC0[bs - 1][indexA] * (1 << bit_depth_minus8);
	}
}

static void vertcal_filter(decoder_context *decoder,
			   unsigned mb_id, unsigned sub_mb_id)
{
	macroblock *mb = &decoder->frames[0]->macroblocks[mb_id];
	const decoder_context_sps *sps = decoder->active_sps;
	macroblock *src_mb_left = mb;
	unsigned sub_mb_id_left;
	unsigned boundary_strength;
	unsigned alpha, beta;
	uint8_t *chroma_decoded;
	uint8_t *p[4], *q[4];
	int chromaStyleFilteringFlag;
	int bit_depth_minus8;
	int plane_id;
	int tC0;
	int i;

	sub_mb_id_left = get_sub_id_4x4_left(decoder, &src_mb_left,
					     mb_id, sub_mb_id);

	if (sub_mb_id_left == MB_UNAVAILABLE) {
		return;
	}

	if (is_intra_mb(mb) || (mb != src_mb_left && is_intra_mb(src_mb_left))) {
		if (mb != src_mb_left) {
			boundary_strength = 4;
		} else {
			boundary_strength = 3;
		}
	} else {
		DECODE_ERR("Shouldn't be here");
	}

	get_alpha_beta_tC0(decoder, mb->QP_Y, src_mb_left->QP_Y,
			   boundary_strength, &alpha, &beta, &tC0);

	bit_depth_minus8 = decoder->active_sps->bit_depth_luma_minus8;

	for (i = 0; i < 4; i++) {
		p[0] = &src_mb_left->luma_decoded[sub_mb_id_left][3 + i * 4];
		p[1] = &src_mb_left->luma_decoded[sub_mb_id_left][2 + i * 4];
		p[2] = &src_mb_left->luma_decoded[sub_mb_id_left][1 + i * 4];
		p[3] = &src_mb_left->luma_decoded[sub_mb_id_left][0 + i * 4];

		q[0] = &mb->luma_decoded[sub_mb_id][0 + i * 4];
		q[1] = &mb->luma_decoded[sub_mb_id][1 + i * 4];
		q[2] = &mb->luma_decoded[sub_mb_id][2 + i * 4];
		q[3] = &mb->luma_decoded[sub_mb_id][3 + i * 4];

		if (filterSamplesFlag(alpha, beta, *p[0], *p[1], *q[0], *q[1])) {
			apply_filter(p, q, alpha, beta, boundary_strength,
				     bit_depth_minus8 + 8, 0,
				     tC0 * (1 << bit_depth_minus8));
		}
	}

	if (sub_mb_id % 4 != 0 || ChromaArrayType() == 0) {
		return;
	}

	sub_mb_id >>= 2;
	sub_mb_id_left = get_sub_id_2x2_left(decoder, &src_mb_left,
					     mb_id, sub_mb_id);
	assert(sub_mb_id_left != MB_UNAVAILABLE);

	bit_depth_minus8 = decoder->active_sps->bit_depth_chroma_minus8;
	chromaStyleFilteringFlag = (ChromaArrayType() != 3);

	for (plane_id = 0; plane_id < 2; plane_id++) {
		if (plane_id == 0) {
			get_alpha_beta_tC0(decoder, mb->QPcb, src_mb_left->QPcb,
					boundary_strength, &alpha, &beta, &tC0);
		} else {
			get_alpha_beta_tC0(decoder, mb->QPcr, src_mb_left->QPcr,
					boundary_strength, &alpha, &beta, &tC0);
		}

		for (i = 0; i < 4; i++) {
			SETUP_PLANE_PTR(chroma_decoded, plane_id, src_mb_left,
					sub_mb_id_left);

			p[0] = &chroma_decoded[3 + i * 4];
			p[1] = &chroma_decoded[2 + i * 4];
			p[2] = &chroma_decoded[1 + i * 4];
			p[3] = &chroma_decoded[0 + i * 4];

			SETUP_PLANE_PTR(chroma_decoded, plane_id, mb, sub_mb_id);

			q[0] = &chroma_decoded[0 + i * 4];
			q[1] = &chroma_decoded[1 + i * 4];
			q[2] = &chroma_decoded[2 + i * 4];
			q[3] = &chroma_decoded[3 + i * 4];

			if (filterSamplesFlag(alpha, beta,
						*p[0], *p[1], *q[0], *q[1]))
			{
				apply_filter(p, q, alpha, beta, boundary_strength,
					     bit_depth_minus8 + 8,
					     chromaStyleFilteringFlag,
					     tC0 * (1 << bit_depth_minus8));
			}
		}
	}
}

static void horizontal_filter(decoder_context *decoder,
			      unsigned mb_id, unsigned sub_mb_id)
{
	macroblock *mb = &decoder->frames[0]->macroblocks[mb_id];
	const decoder_context_sps *sps = decoder->active_sps;
	macroblock *src_mb_up = mb;
	unsigned sub_mb_id_up;
	unsigned boundary_strength;
	unsigned alpha, beta;
	uint8_t *chroma_decoded;
	uint8_t *p[4], *q[4];
	int chromaStyleFilteringFlag;
	int bit_depth_minus8;
	int plane_id;
	int tC0;
	int i;

	sub_mb_id_up = get_sub_id_4x4_up(decoder, &src_mb_up, mb_id, sub_mb_id);

	if (sub_mb_id_up == MB_UNAVAILABLE) {
		return;
	}

	if (is_intra_mb(mb) || (mb != src_mb_up && is_intra_mb(src_mb_up))) {
		if (mb != src_mb_up) {
			boundary_strength = 4;
		} else {
			boundary_strength = 3;
		}
	} else {
		DECODE_ERR("Shouldn't be here");
	}

	get_alpha_beta_tC0(decoder, mb->QP_Y, src_mb_up->QP_Y,
			   boundary_strength, &alpha, &beta, &tC0);

	bit_depth_minus8 = decoder->active_sps->bit_depth_luma_minus8;

	for (i = 0; i < 4; i++) {
		p[0] = &src_mb_up->luma_decoded[sub_mb_id_up][12 + i];
		p[1] = &src_mb_up->luma_decoded[sub_mb_id_up][ 8 + i];
		p[2] = &src_mb_up->luma_decoded[sub_mb_id_up][ 4 + i];
		p[3] = &src_mb_up->luma_decoded[sub_mb_id_up][ 0 + i];

		q[0] = &mb->luma_decoded[sub_mb_id][ 0 + i];
		q[1] = &mb->luma_decoded[sub_mb_id][ 4 + i];
		q[2] = &mb->luma_decoded[sub_mb_id][ 8 + i];
		q[3] = &mb->luma_decoded[sub_mb_id][12 + i];

		if (filterSamplesFlag(alpha, beta, *p[0], *p[1], *q[0], *q[1])) {
			apply_filter(p, q, alpha, beta, boundary_strength,
				     bit_depth_minus8 + 8, 0,
				     tC0 * (1 << bit_depth_minus8));
		}
	}

	if (sub_mb_id % 4 != 0 || ChromaArrayType() == 0) {
		return;
	}

	sub_mb_id >>= 2;
	sub_mb_id_up = get_sub_id_2x2_up(decoder, &src_mb_up, mb_id, sub_mb_id);
	assert(sub_mb_id_up != MB_UNAVAILABLE);

	bit_depth_minus8 = decoder->active_sps->bit_depth_chroma_minus8;
	chromaStyleFilteringFlag = (ChromaArrayType() != 3);

	for (plane_id = 0; plane_id < 2; plane_id++) {
		if (plane_id == 0) {
			get_alpha_beta_tC0(decoder, mb->QPcb, src_mb_up->QPcb,
					boundary_strength, &alpha, &beta, &tC0);
		} else {
			get_alpha_beta_tC0(decoder, mb->QPcr, src_mb_up->QPcr,
					boundary_strength, &alpha, &beta, &tC0);
		}

		for (i = 0; i < 4; i++) {
			SETUP_PLANE_PTR(chroma_decoded, plane_id, src_mb_up,
					sub_mb_id_up);

			p[0] = &chroma_decoded[12 + i];
			p[1] = &chroma_decoded[ 8 + i];
			p[2] = &chroma_decoded[ 4 + i];
			p[3] = &chroma_decoded[ 0 + i];

			SETUP_PLANE_PTR(chroma_decoded, plane_id, mb, sub_mb_id);

			q[0] = &chroma_decoded[ 0 + i];
			q[1] = &chroma_decoded[ 4 + i];
			q[2] = &chroma_decoded[ 8 + i];
			q[3] = &chroma_decoded[12 + i];

			if (filterSamplesFlag(alpha, beta,
						*p[0], *p[1], *q[0], *q[1]))
			{
				apply_filter(p, q, alpha, beta, boundary_strength,
					     bit_depth_minus8 + 8,
					     chromaStyleFilteringFlag,
					     tC0 * (1 << bit_depth_minus8));
			}
		}
	}
}

void mb_apply_deblocking(decoder_context *decoder, unsigned mb_id)
{
	unsigned sub_mb_id;

	switch (decoder->sh.disable_deblocking_filter_idc) {
	case 0:
		decoder->get_mb_slice_constraint = 0;
		break;
	case 1:
		return;
	case 2:
		decoder->get_mb_slice_constraint = 1;
		break;
	}

	for (sub_mb_id = 0; sub_mb_id < 16; sub_mb_id++) {
		vertcal_filter(decoder, mb_id, sub_mb_id);
	}

	for (sub_mb_id = 0; sub_mb_id < 16; sub_mb_id++) {
		horizontal_filter(decoder, mb_id, sub_mb_id);
	}

	decoder->get_mb_slice_constraint = 1;
}
