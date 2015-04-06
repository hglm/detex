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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "detex.h"
#include "file-info.h"
#include "misc.h"

static const uint8_t ktx_id[12] = {
	0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

// Load texture from KTX file with mip-maps. Returns true if successful.
// nu_mipmaps is a return parameter that returns the number of mipmap levels found.
// textures_out is a return parameter for an array of detexTexture pointers that is allocated,
// free with free(). textures_out[i] are allocated textures corresponding to each level, free
// with free();
bool detexLoadKTXFileWithMipmaps(const char *filename, int max_mipmaps, detexTexture ***textures_out,
int *nu_mipmaps_out) {
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Could not open file %s", filename);
		return false;
	}
	int header[16];
	size_t s = fread(header, 1, 64, f);
	if (s != 64)
		return false;
	if (memcmp(header, ktx_id, 12) != 0) {
		// KTX signature not found.
		detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Couldn't find KTX signature");
		return false;
	}
	int wrong_endian = 0;
	if (header[3] == 0x01020304) {
		// Wrong endian .ktx file.
		wrong_endian = 1;
		for (int i = 3; i < 16; i++) {
			uint8_t *b = (uint8_t *)&header[i];
			uint8_t temp = b[0];
			b[0] = b[3];
			b[3] = temp;
			temp = b[1];
			b[1] = b[2];
			b[2] = temp;
		}
	}
	int glType = header[4];
	int glFormat = header[6];
	int glInternalFormat = header[7];
//	int pixel_depth = header[11];
	const detexTextureFileInfo *info = detexLookupKTXFileInfo(glInternalFormat, glFormat, glType);
	if (info == NULL) {
		detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Unsupported format in .ktx file "
			"(glInternalFormat = 0x%04X)", glInternalFormat);
		return false;
	}
	int bytes_per_block;
	if (detexFormatIsCompressed(info->texture_format))
		bytes_per_block = detexGetCompressedBlockSize(info->texture_format);
	else
		bytes_per_block = detexGetPixelSize(info->texture_format);
	int block_width = info->block_width;
	int block_height = info->block_height;
//	printf("File is %s texture.\n", info->text1);
	int width = header[9];
	int height = header[10];
	int extended_width = ((width + block_width - 1) / block_width) * block_width;
	int extended_height = ((height + block_height - 1) / block_height) * block_height;
	int nu_file_mipmaps = header[14];
//	if (nu_file_mipmaps > 1 && max_mipmaps == 1) {
//		detexSetErrorMessage("Disregarding mipmaps beyond the first level.\n");
//	}
	int nu_mipmaps;
	if (nu_file_mipmaps > max_mipmaps)
		nu_mipmaps = max_mipmaps;
	else
		nu_mipmaps = nu_file_mipmaps;
 	if (header[15] > 0) {
		// Skip metadata.
		uint8_t *metadata = (unsigned char *)malloc(header[15]);
		if (fread(metadata, 1, header[15], f) < header[15]) {
			detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Error reading file %s", filename);
			return false;
		}
		free(metadata);
	}
	detexTexture **textures = (detexTexture **)malloc(sizeof(detexTexture *) * nu_mipmaps);
	for (int i = 0; i < nu_mipmaps; i++) {
		uint32_t image_size_buffer[1];
		size_t r = fread(image_size_buffer, 1, 4, f);
		if (r != 4) {
			for (int j = 0; j < i; j++)
				free(textures[j]);
			free(textures);
			detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Error reading file %s", filename);
			return false;
		}
		if (wrong_endian) {
			uint8_t *image_size_bytep = (uint8_t *)&image_size_buffer[0];
			unsigned char temp = image_size_buffer[0];
			image_size_bytep[0] = image_size_bytep[3];
			image_size_bytep[3] = temp;
			temp = image_size_bytep[1];
			image_size_bytep[1] = image_size_bytep[2];
			image_size_bytep[2] = temp;
		}
		int image_size = image_size_buffer[0];
		int n = (extended_height / block_height) * (extended_width / block_width);
		if (/* (detexPixelSize(info->texture_format) != 3 &&
		detexPixelSize(info->texture_format) != 6 && */
		image_size != n * bytes_per_block) {
			for (int j = 0; j < i; j++)
				free(textures[j]);
			free(textures);
			detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Error loading file %s: "
				"Image size field of mipmap level %d does not match (%d vs %d)",
				filename, i, image_size, n * bytes_per_block);
			return false;
		}
		// Allocate texture.
		textures[i] = (detexTexture *)malloc(sizeof(detexTexture));
		textures[i]->format = info->texture_format;
		textures[i]->data = (uint8_t *)malloc(n * bytes_per_block);
		textures[i]->width = width;
		textures[i]->height = height;
		textures[i]->width_in_blocks = extended_width / block_width;
		textures[i]->height_in_blocks = extended_height / block_height;
		if (fread(textures[i]->data, 1, n * bytes_per_block, f) < n * bytes_per_block) {
			for (int j = 0; j <= i; j++)
				free(textures[j]);
			free(textures);
			detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Error reading file %s", filename);
			return false;
		}
		// Divide by two for the next mipmap level, rounding down.
		width >>= 1;
		height >>= 1;
		extended_width = ((width + block_width - 1) / block_width) * block_width;
		extended_height = ((height + block_height - 1) / block_height) * block_height;
		// Read mipPadding. But not if we have already read everything specified.
		char buffer[4];
		if (i + 1 < nu_mipmaps) {
			int nu_bytes = 3 - ((image_size + 3) % 4);
			if (fread(buffer, 1, nu_bytes, f) != nu_bytes) {
				for (int j = 0; j <= i; j++)
					free(textures[j]);
				free(textures);
				detexSetErrorMessage("detexLoadKTXFileWithMipmaps: Error reading file %s", filename);
				return false;
			}
		}
	}
	fclose(f);
	*nu_mipmaps_out = nu_mipmaps;
	*textures_out = textures;
	return true;
}

// Load texture from KTX file (first mip-map only). Returns true if successful.
// The texture is allocated, free with free().
bool detexLoadKTXFile(const char *filename, detexTexture **texture_out) {
	int nu_mipmaps;
	detexTexture **textures;
	bool r = detexLoadKTXFileWithMipmaps(filename, 1, &textures, &nu_mipmaps);
	if (!r)
		return false;
	*texture_out = textures[0];
	free(textures);
	return true;
}

