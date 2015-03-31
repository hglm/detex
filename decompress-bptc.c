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

#include "detex.h"

/* Decompress a 128-bit 4x4 pixel texture block compressed using the BPTC */
/* (BC7) format. */
bool detexDecompressBlockBPTC(const uint8_t *bitstring, uint32_t mode_mask,
uint32_t flags, uint8_t *pixel_buffer) {
	return false;
}

/* Decompress a 128-bit 4x4 pixel texture block compressed using the */
/* BPTC_FLOAT (BC6H) format. The output format is */
/* DETEX_PIXEL_FORMAT_FLOAT_RGBX16. */
bool detexDecompressBlockBPTC_FLOAT(const uint8_t *bitstring, uint32_t mode_mask,
uint32_t flags, uint8_t *pixel_buffer) {
	return false;
}

/* Decompress a 128-bit 4x4 pixel texture block compressed using the */
/* BPTC_FLOAT (BC6H_FLOAT) format. The output format is */
/* DETEX_PIXEL_FORMAT_SIGNED_FLOAT_RGBX16. */
bool detexDecompressBlockBPTC_SIGNED_FLOAT(const uint8_t *bitstring,
uint32_t mode_mask, uint32_t flags, uint8_t *pixel_buffer) {
	return false;
}


