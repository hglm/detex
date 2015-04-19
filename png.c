/*

Copyright (c) 2015 Harm Hanemaaijer <fgenfb@yahoo.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

// This file is not part of the detex library proper, but used by detex-convert.

#include <stdlib.h>
#include <png.h>

#include "detex.h"
#include "detex-png.h"

// Load texture from PNG file (first mip-map only). Returns true if successful.
// The texture is allocated, free with free().
bool detexLoadPNGFile(const char *filename, detexTexture **texture_out) {
	int png_width, png_height;
	png_byte color_type;
	png_byte bit_depth;
	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep *row_pointers;

	png_byte header[8];    // 8 is the maximum size that can be checked

        FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Error - file %s could not be opened for reading.\n", filename);
		return false;
	}
	// Read header.
	size_t r = fread(header, 1, 8, fp);
	if (r != 8) {
		printf("Error reading file %s\n", filename);
		return false;
	}
	if (png_sig_cmp(header, 0, 8)) {
		printf("Error - file %s is not recognized as a PNG file.\n", filename);
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		printf("png_create_read_struct failed\n");
		return false;
	}   

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		printf("png_create_info_struct failed\n");
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("Error during PNG I/O initialization.");
		return false;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	png_width = png_get_image_width(png_ptr, info_ptr);
	png_height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	if (bit_depth != 8 && bit_depth != 16) {
		printf("Error - unexpected bit depth in PNG file\n");
		return false;
	}

	number_of_passes = png_set_interlace_handling(png_ptr);
	if (number_of_passes > 1) {
		printf("Error - interlaced PNG files not supported\n");
		return false;
	}
	png_read_update_info(png_ptr, info_ptr);

        /* Read pixel data. */
	if (setjmp(png_jmpbuf(png_ptr))) {
		printf("Error during png_read_image.\n");
		return false;
        }
	row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * png_height);
	int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	for (int y = 0; y < png_height; y++)
		row_pointers[y] = (png_byte *)malloc(row_bytes);
	png_read_image(png_ptr, row_pointers);
	fclose(fp);

	uint32_t format;
	if (color_type == PNG_COLOR_TYPE_GRAY)
		if (bit_depth == 8)
			format = DETEX_PIXEL_FORMAT_R8;
		else
			format = DETEX_PIXEL_FORMAT_R16;
	else if (color_type == PNG_COLOR_TYPE_RGB)
		if (bit_depth == 8)
			format = DETEX_PIXEL_FORMAT_RGB8;
		else
			format = DETEX_PIXEL_FORMAT_RGB16;
	else if (color_type == PNG_COLOR_TYPE_RGB)
		if (bit_depth == 8)
			format = DETEX_PIXEL_FORMAT_RGBA8;
		else
			format = DETEX_PIXEL_FORMAT_RGBA16;
	else {
		printf("Error - unexpected bit color type in PNG file\n");
		return false;
	}

	detexTexture *texture = (detexTexture *)malloc(sizeof(detexTexture));
	texture->format = format;
	texture->width = png_width;
	texture->height = png_height;
	texture->width_in_blocks = png_width;
	texture->height_in_blocks = png_height;
	texture->data = (uint8_t *)malloc(png_height * row_bytes);
	for (int y = 0; y < png_height; y++)
		memcpy(texture->data + y * row_bytes, row_pointers[y], row_bytes);
	// Clean up.
	for (int y = 0; y < png_height; y++)
		free(row_pointers[y]);
	free(row_pointers);
	*texture_out = texture;
	return true;
}

