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

// Load texture from DDS file with mip-maps. Returns true if successful.
// nu_levels is a return parameter that returns the number of mipmap levels found.
// textures_out is a return parameter for an array of detexTexture pointers that is allocated,
// free with free(). textures_out[i] are allocated textures corresponding to each level, free
// with free();
bool detexLoadDDSFileWithMipmaps(const char *filename, int max_mipmaps, detexTexture ***textures_out,
int *nu_levels_out) {
	FILE *f = fopen(filename, "rb");
	if (f == NULL) {
		detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Could not open file %s", filename);
		return false;
	}

	char id[4];
	size_t s = fread(id, 1, 4, f);
	if (s != 4) {
		detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Error reading file %s", filename);
		return false;
	}
	if (id[0] != 'D' || id[1] != 'D' || id[2] != 'S' || id[3] != ' ') {
		detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Couldn't find DDS signature");
		return false;
	}
	uint8_t header[124];
	s = fread(header, 1, 124, f);
	if (s != 124) {
		detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Error reading file %s", filename);
		return false;
	}
	uint8_t *headerp = &header[0];
	int width = *(uint32_t *)(headerp + 12);
	int height = *(uint32_t *)(headerp + 8);
//	int pitch = *(uint32_t *)(headerp + 16);
	int pixel_format_flags = *(uint32_t *)(headerp + 76);
	int block_width = 4;
	int block_height = 4;
	int bitcount = *(uint32_t *)(headerp + 84);
	uint32_t red_mask = *(uint32_t *)(headerp + 88);
	uint32_t green_mask = *(uint32_t *)(headerp + 92);
	uint32_t blue_mask = *(uint32_t *)(headerp + 96);
	uint32_t alpha_mask = *(uint32_t *)(headerp + 100);
	char four_cc[5];
	strncpy(four_cc, (char *)&header[80], 4);
	four_cc[4] = '\0';
	uint32_t dx10_format = 0;
	if (strncmp(four_cc, "DX10", 4) == 0) {
		uint32_t dx10_header[5];
		s = fread(dx10_header, 1, 20, f);
		if (s != 20) {
			detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Error reading file %s", filename);
			return false;
		}
		dx10_format = dx10_header[0];
		uint32_t resource_dimension = dx10_header[1];
		if (resource_dimension != 3) {
			detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Only 2D textures supported for .dds files");
			return false;
		}
	}
	const detexTextureFileInfo *info = detexLookupDDSFileInfo(four_cc, dx10_format, pixel_format_flags, bitcount,
		red_mask, green_mask, blue_mask, alpha_mask);
	if (info == NULL) {
		detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Unsupported format in .dds file (fourCC = %s, "
			"DX10 format = %d).", four_cc, dx10_format);
		return false;
	}
	// Maybe implement option to treat BC1 as BC1A?
	int bytes_per_block;
	if (detexFormatIsCompressed(info->texture_format))
		bytes_per_block = detexGetCompressedBlockSize(info->texture_format);
	else
		bytes_per_block = detexGetPixelSize(info->texture_format);
	block_width = info->block_width;
	block_height = info->block_height;
	int extended_width = ((width + block_width - 1) / block_width) * block_width;
	int extended_height = ((height + block_height - 1) / block_height) * block_height;
	uint32_t flags = *(uint32_t *)(headerp + 4);
	int nu_file_mipmaps = 1;
	if (flags & 0x20000) {
		nu_file_mipmaps = *(uint32_t *)(headerp + 24);
//		if (nu_file_mipmaps > 1 && max_mipmaps == 1) {
//			detexSetErrorMessage("Disregarding mipmaps beyond the first level.\n");
//		}
	}
	int nu_mipmaps;
	if (nu_file_mipmaps > max_mipmaps)
		nu_mipmaps = max_mipmaps;
	else
		nu_mipmaps = nu_file_mipmaps;
	detexTexture **textures = (detexTexture **)malloc(sizeof(detexTexture *) * nu_mipmaps);
	for (int i = 0; i < nu_mipmaps; i++) {
		int n = (extended_height / block_width) * (extended_width / block_height);
		// Allocate texture.
		textures[i] = (detexTexture *)malloc(sizeof(detexTexture));
		textures[i]->format = info->texture_format;
		textures[i]->data = (uint8_t *)malloc(n * bytes_per_block);
		textures[i]->width = width;
		textures[i]->height = height;
		textures[i]->width_in_blocks = extended_width / block_width;
		textures[i]->height_in_blocks = extended_height / block_height;
		size_t r = fread(textures[i]->data, 1, n * bytes_per_block, f);
		if (r < n * bytes_per_block) {
			detexSetErrorMessage("detexLoadDDSFileWithMipmaps: Error reading file %s", filename);
			return false;
		}
		// Divide by two for the next mipmap level, rounding down.
		width >>= 1;
		height >>= 1;
		extended_width = ((width + block_width - 1) / block_width) * block_width;
		extended_height = ((height + block_height - 1) / block_height) * block_height;
	}
	fclose(f);
	*nu_levels_out = nu_mipmaps;
	*textures_out = textures;
	return true;
}


// Load texture from DDS file (first mip-map only). Returns true if successful.
// The texture is allocated, free with free().
bool detexLoadDDSFile(const char *filename, detexTexture **texture_out) {
	int nu_mipmaps;
	detexTexture **textures;
	bool r = detexLoadDDSFileWithMipmaps(filename, 1, &textures, &nu_mipmaps);
	if (!r)
		return false;
	*texture_out = textures[0];
	free(textures);
	return true;
}

