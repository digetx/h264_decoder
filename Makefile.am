AUTOMAKE_OPTIONS = subdir-objects
AM_CFLAGS = -I$(top_srcdir)/include

noinst_PROGRAMS = h264_decode

h264_decode_SOURCES =					\
	syntax_parse/ANNEX_B.c				\
	syntax_parse/NAL.c				\
	syntax_parse/SPS.c				\
	syntax_parse/PPS.c				\
	syntax_parse/slice_data.c			\
	syntax_parse/slice_header.c			\
	syntax_parse/macroblock.c			\
	syntax_parse/macroblock_layer.c			\
	syntax_parse/macroblock_prediction.c		\
	syntax_parse/macroblock_prediction_intra_4x4.c	\
	syntax_parse/residual.c				\
	syntax_parse/CAVLC.c				\
	syntax_parse/MP4.c				\
	syntax_parse/VUI.c				\
	bitstream/bitstream.c				\
	transforms/inv_scan.c				\
	transforms/inv_scale_4x4.c			\
	transforms/inv_scale_chroma_DC.c		\
	transforms/inv_scale_luma_DC.c			\
	transforms/inv_transform_4x4.c			\
	transforms/inv_transform_chroma_DC.c		\
	transforms/inv_transform_luma_DC.c		\
	transforms/LevelScale4x4.c			\
	transforms/matrix_mul.c				\
	transforms/intra_prediction_luma_4x4.c		\
	transforms/intra_prediction_luma_16x16.c	\
	transforms/intra_prediction_chroma.c		\
	decoder.c					\
	main.c
