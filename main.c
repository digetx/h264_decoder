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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "decoder.h"
#include "syntax_parse.h"

static void save_decoded_frame(decoder_context *decoder)
{
	size_t img_sz = decoder_image_frame_size(decoder);
	FILE *fp = decoder->opaque;

	fwrite(decoder->decoded_image, 1, img_sz, fp);
	assert(ferror(fp) == 0);
}

int main(int argc, char **argv)
{
	struct stat sb;
	decoder_context decoder;
	const char *in_file_path = NULL;
	const char *out_file_path = NULL;
	FILE *fp_out;
	void *data_ptr;
	int fd;
	int c;

	while ((c = getopt(argc, argv, "i:o:")) != -1) {
		switch (c) {
		case 'i':
			in_file_path = optarg;
			break;
		case 'o':
			out_file_path = optarg;
			break;
		default:
			break;
		}
	}

	if (in_file_path == NULL || out_file_path == NULL) {
		fprintf(stderr, "-i h264 input file path\n");
		fprintf(stderr, "-o decoded i420 frames output file path\n");
		exit(EXIT_FAILURE);
	}

	fd = open(in_file_path, O_RDONLY);

	assert(fd != -1);
	assert(fstat(fd, &sb) != -1);

	fp_out = fopen(out_file_path, "w+");

	assert(fp_out != NULL);

	data_ptr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	assert(data_ptr != MAP_FAILED);

	assert(madvise(data_ptr, sb.st_size, MADV_SEQUENTIAL) == 0);

	decoder_init(&decoder, data_ptr, sb.st_size);

	if (out_file_path) {
		decoder_set_notify(&decoder, save_decoded_frame, fp_out);
	}

	parse_annex_b(&decoder);

	return 0;
}
