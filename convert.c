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

static void ConvertPixel32SwapRAndB(uint8_t * DETEX_RESTRICT source_pixel_buffer, int nu_pixels,
uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		/* Swap R and B. */
		uint32_t pixel = *source_pixel32_buffer;
		pixel = detexPack32RGBA8(
			detexPixel32GetB8(pixel),
			detexPixel32GetG8(pixel),
			detexPixel32GetR8(pixel),
			detexPixel32GetA8(pixel)
			);
		*target_pixel32_buffer = pixel;
		source_pixel32_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel64SwapRAndB(uint8_t * DETEX_RESTRICT source_pixel_buffer, int nu_pixels,
uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint64_t *source_pixel64_buffer = (uint64_t *)source_pixel_buffer;
	uint64_t *target_pixel64_buffer = (uint64_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		/* Swap R and B (16-bit). */
		uint64_t pixel = *source_pixel64_buffer;
		pixel = detexPack64RGBA16(
			detexPixel64GetB16(pixel),
			detexPixel64GetG16(pixel),
			detexPixel64GetR16(pixel),
			detexPixel64GetA16(pixel)
			);
		*target_pixel64_buffer = pixel;
		source_pixel64_buffer++;
		target_pixel64_buffer++;
	}
}

static void ConvertPixel32RG16ToRGBX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		uint32_t red = detexPixel32GetR16(pixel);
		uint32_t green = detexPixel32GetG16(pixel);
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(red >> 8,
			green >> 8, 0);
		source_pixel32_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel32RG16ToBGRX8(uint8_t * DETEX_RESTRICT source_pixel_buffer, int nu_pixels,
uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		uint32_t red = detexPixel32GetR16(pixel);
		uint32_t green = detexPixel32GetG16(pixel);
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(0, green >> 8,
			red >> 8);
		source_pixel32_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel32SignedRG16ToRGBX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		uint32_t red = detexPixel32GetSignedR16(pixel) + 0x8000;
		uint32_t green = detexPixel32GetSignedG16(pixel) + 0x8000;
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(red >> 8,
			green >> 8, 0);
		source_pixel32_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel32SignedRG16ToBGRX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		uint32_t red = detexPixel32GetSignedR16(pixel) + 0x8000;
		uint32_t green = detexPixel32GetSignedG16(pixel) + 0x8000;
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(0,
			green >> 8, red >> 8);
		source_pixel32_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel16R16ToRGBX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t red = *source_pixel16_buffer;
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(red >> 8,
			0, 0);
		source_pixel16_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel16R16ToBGRX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t red = *source_pixel16_buffer;
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(0, 0,
			red >> 8);
		source_pixel16_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel16SignedR16ToRGBX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t red = *(int16_t *)source_pixel16_buffer + 0x8000;
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(red >> 8,
			0, 0);
		source_pixel16_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel16SignedR16ToBGRX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t red = *(int16_t *)source_pixel16_buffer + 0x8000;
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(0, 0,
			red >> 8);
		source_pixel16_buffer++;
		target_pixel32_buffer++;
	}
}

/* Convert pixels between different formats. Return true if succesful. */

bool detexConvertPixels(uint8_t * DETEX_RESTRICT source_pixel_buffer, uint32_t nu_pixels,
uint32_t source_pixel_format, uint8_t * DETEX_RESTRICT target_pixel_buffer,
uint32_t target_pixel_format) {
	if (source_pixel_format == target_pixel_format) {
		memcpy(target_pixel_buffer, source_pixel_buffer,
			detexGetBlockSize(target_pixel_format));
		return true;
	}
	int source_pixel_size = detexGetPixelSize(source_pixel_format);
	int target_pixel_size = detexGetPixelSize(target_pixel_format);
	int source_nu_components = detexGetNumberOfComponents(
		source_pixel_format);
	int target_nu_components = detexGetNumberOfComponents(
		target_pixel_format);
	if (source_nu_components != target_nu_components) {
		if (source_pixel_format == DETEX_PIXEL_FORMAT_RG16) {
			if (target_pixel_format == DETEX_PIXEL_FORMAT_RGBX8) {
				ConvertPixel32RG16ToRGBX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
			else if (target_pixel_format == DETEX_PIXEL_FORMAT_BGRX8) {
				ConvertPixel32RG16ToBGRX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
		}
		else if (source_pixel_format == DETEX_PIXEL_FORMAT_SIGNED_RG16) {
			if (target_pixel_format == DETEX_PIXEL_FORMAT_RGBX8) {
				ConvertPixel32SignedRG16ToRGBX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
			else if (target_pixel_format == DETEX_PIXEL_FORMAT_BGRX8) {
				ConvertPixel32SignedRG16ToBGRX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
		}
		else if (source_pixel_format == DETEX_PIXEL_FORMAT_R16) {
			if (target_pixel_format == DETEX_PIXEL_FORMAT_RGBX8) {
				ConvertPixel16R16ToRGBX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
			else if (target_pixel_format == DETEX_PIXEL_FORMAT_BGRX8) {
				ConvertPixel16R16ToBGRX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
		}
		else if (source_pixel_format == DETEX_PIXEL_FORMAT_SIGNED_R16) {
			if (target_pixel_format == DETEX_PIXEL_FORMAT_RGBX8) {
				ConvertPixel16SignedR16ToRGBX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
			else if (target_pixel_format == DETEX_PIXEL_FORMAT_BGRX8) {
				ConvertPixel16SignedR16ToBGRX8(source_pixel_buffer,
					nu_pixels, target_pixel_buffer);
				return true;
			}
		}
		return false;
	}
	if (source_pixel_size != target_pixel_size)
		return false;
	// Pixel size and number of components of source and target are equal.
	if (source_pixel_size == 4) {
		if (source_nu_components < 3)
			return false;
		/* Conversion between RGBA8, BGRA8, RGBX8 and BGRX8. */
		/* Only need action when RGB order differs. */
		if ((source_pixel_format &
		DETEX_PIXEL_FORMAT_BGR_COMPONENT_ORDER_BIT) !=
		(target_pixel_format &
		DETEX_PIXEL_FORMAT_BGR_COMPONENT_ORDER_BIT))
			ConvertPixel32SwapRAndB(source_pixel_buffer, nu_pixels,
				target_pixel_buffer);
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
		DETEX_PIXEL_FORMAT_BGR_COMPONENT_ORDER_BIT))
			ConvertPixel64SwapRAndB(source_pixel_buffer, nu_pixels,
				target_pixel_buffer);
		return true;
	}
	else
		return false;
}

