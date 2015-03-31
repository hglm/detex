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

/* Convert pixels between different formats in place. Return true if succesful. */

bool detexConvertPixels(uint8_t *pixel_buffer, uint32_t nu_pixels,
uint32_t source_pixel_format, uint32_t target_pixel_format) {
	if (source_pixel_format == target_pixel_format)
		return true;
	int source_pixel_size = detexGetPixelSize(source_pixel_format);
	int target_pixel_size = detexGetPixelSize(target_pixel_format);
	if (source_pixel_size != target_pixel_size)
		return false;
	int source_nu_components = detexGetNumberOfComponents(
		source_pixel_format);
	int target_nu_components = detexGetNumberOfComponents(
		target_pixel_format);
	if (source_nu_components != target_nu_components)
		return false;
	if (source_pixel_size == 4) {
		if (source_nu_components < 3)
			return false;
		/* Conversion between RGBA8, BGRA8, RGBX8 and BGRX8. */
		/* Only need action when RGB order differs. */
		if ((source_pixel_format &
		DETEX_PIXEL_FORMAT_BGR_COMPONENT_ORDER_BIT) !=
		(target_pixel_format &
		DETEX_PIXEL_FORMAT_BGR_COMPONENT_ORDER_BIT)) {
			for (int i = 0; i < nu_pixels; i++) {
				/* Swap R and B. */
				uint8_t c0 = *pixel_buffer;
				uint8_t c2 = *(pixel_buffer + 2);
				*pixel_buffer = c2;
				*(pixel_buffer + 2) = c0;
				pixel_buffer += 4;
			}
		}
		return true;
	}
	else if (source_pixel_size == 8) {
		if ((source_pixel_format &
		DETEX_PIXEL_FORMAT_FLOAT_BIT) !=
		(target_pixel_format &
		DETEX_PIXEL_FORMAT_FLOAT_BIT))
			return false;
		if ((source_pixel_format &
		DETEX_PIXEL_FORMAT_SIGNED_BIT) !=
		(target_pixel_format &
		DETEX_PIXEL_FORMAT_SIGNED_BIT))
			return false;
		/* Only need action when RGB order differs. */
		if ((source_pixel_format &
		DETEX_PIXEL_FORMAT_BGR_COMPONENT_ORDER_BIT) !=
		(target_pixel_format &
		DETEX_PIXEL_FORMAT_BGR_COMPONENT_ORDER_BIT)) {
			for (int i = 0; i < nu_pixels; i++) {
				/* Swap R and B. */
				uint16_t c0 = *(uint16_t *)pixel_buffer;
				uint8_t c2 = *(uint16_t *)(pixel_buffer + 4);
				*(uint16_t *)pixel_buffer = c2;
				*(uint16_t *)(pixel_buffer + 4) = c0;
				pixel_buffer += 8;
			}
		}
		return true;
	}
	else
		return false;
}

