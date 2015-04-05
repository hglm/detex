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

#include <string.h>
#include "detex.h"

typedef bool (*detexDecompressBlockFuncType)(const uint8_t *bitstring,
	uint32_t mode_mask, uint32_t flags, uint8_t *pixel_buffer);

static detexDecompressBlockFuncType decompress_function[] = {
	NULL,
	detexDecompressBlockBC1,
	detexDecompressBlockBC1A,
	detexDecompressBlockBC2,
	detexDecompressBlockBC3,
	detexDecompressBlockRGTC1,
	detexDecompressBlockSIGNED_RGTC1,
	detexDecompressBlockRGTC2,
	detexDecompressBlockSIGNED_RGTC2,
	detexDecompressBlockBPTC_FLOAT,
	detexDecompressBlockBPTC_SIGNED_FLOAT,
	detexDecompressBlockBPTC,
	detexDecompressBlockETC1,
	detexDecompressBlockETC2,
	detexDecompressBlockETC2_PUNCHTHROUGH,
	detexDecompressBlockETC2_EAC,
	detexDecompressBlockEAC_R11,
	detexDecompressBlockEAC_SIGNED_R11,
	detexDecompressBlockEAC_RG11,
	detexDecompressBlockEAC_SIGNED_RG11,
};

/*
 * General block decompression function. Block is decompressed using the given
 * compressed format, and stored in the given pixel format. Returns true if
 * succesful.
 */
bool detexDecompressBlock(const uint8_t * DETEX_RESTRICT bitstring, uint32_t texture_format,
uint32_t mode_mask, uint32_t flags, uint8_t * DETEX_RESTRICT pixel_buffer,
uint32_t pixel_format) {
	uint8_t block_buffer[DETEX_MAX_BLOCK_SIZE];
	uint32_t compressed_format = detexGetCompressedFormat(texture_format);
	bool r = decompress_function[compressed_format](bitstring, mode_mask, flags,
            block_buffer);
	if (!r)
		return false;
	/* Convert into desired pixel format. */
	return detexConvertPixels(block_buffer, 16,
		detexGetPixelFormat(texture_format), pixel_buffer, pixel_format); 
}

/*
 * Decode texture function (tiled). Decode an entire compressed texture into an
 * array of image buffer tiles (corresponding to compressed blocks), converting
 * into the given pixel format.
 */
bool detexDecompressTextureTiled(const uint8_t * DETEX_RESTRICT bitstring,
uint32_t texture_format, uint32_t width_in_blocks, uint32_t height_in_blocks,
uint8_t * DETEX_RESTRICT pixel_buffer, uint32_t pixel_format) {
	bool result = true;
	for (int y = 0; y < height_in_blocks; y++)
		for (int x = 0; x < width_in_blocks; x++) {
			bool r = detexDecompressBlock(bitstring, texture_format,
				DETEX_MODE_MASK_ALL, 0, pixel_buffer, pixel_format);
			uint32_t block_size = detexGetPixelSize(pixel_format) * 16;
			if (!r) {
				result = false;
				memset(pixel_buffer, 0, block_size);
			}
			bitstring += detexGetCompressedBlockSize(texture_format);
			pixel_buffer += block_size;
		}
	return result;
}

/*
 * Decode texture function (linear). Decode an entire texture into a single
 * image buffer, with pixels stored row-by-row, converting into the given pixel
 * format.
 */
bool detexDecompressTextureLinear(const uint8_t * DETEX_RESTRICT bitstring,
uint32_t texture_format, uint32_t width_in_blocks, uint32_t height_in_blocks,
uint8_t * DETEX_RESTRICT pixel_buffer, uint32_t pixel_format) {
	uint8_t block_buffer[DETEX_MAX_BLOCK_SIZE];
	int pixel_size = detexGetPixelSize(pixel_format);
	bool result = true;
	for (int y = 0; y < height_in_blocks; y++)
		for (int x = 0; x < width_in_blocks; x++) {
			bool r = detexDecompressBlock(bitstring, texture_format,
				DETEX_MODE_MASK_ALL, 0, block_buffer, pixel_format);
			uint32_t block_size = detexGetPixelSize(pixel_format) * 16;
			if (!r) {
				result = false;
				memset(block_buffer, 0, block_size);
			}
			uint8_t *pixelp = pixel_buffer +
				y * (4 * width_in_blocks * 4 *
					pixel_size)
				+ x * 4 * pixel_size;
			for (int row = 0; row < 4; row++)
				memcpy(pixelp + row * 4 * width_in_blocks *
					pixel_size,
					block_buffer + row * 4 * pixel_size,
					4 * pixel_size);
			bitstring += detexGetCompressedBlockSize(texture_format);
		}
	return result;
}

