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

// Conversion functions. For conversions where the pixel size is unchanged,
// the conversion is performed in-place and target_pixel_buffer will be NULL.

static void ConvertNoop(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
}

// In-place BGR <-> RGB conversions.

static void ConvertPixel32RGBA8ToBGRA8(uint8_t * DETEX_RESTRICT source_pixel_buffer, int nu_pixels,
uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		/* Swap R and B. */
		uint32_t pixel = *source_pixel32_buffer;
		pixel = detexPack32RGBA8(
			detexPixel32GetB8(pixel),
			detexPixel32GetG8(pixel),
			detexPixel32GetR8(pixel),
			detexPixel32GetA8(pixel)
			);
		*source_pixel32_buffer = pixel;
		source_pixel32_buffer++;
	}
}

static void ConvertPixel64RGBX16ToBGRX16(uint8_t * DETEX_RESTRICT source_pixel_buffer, int nu_pixels,
uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint64_t *source_pixel64_buffer = (uint64_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		/* Swap R and B (16-bit). */
		uint64_t pixel = *source_pixel64_buffer;
		pixel = detexPack64RGBA16(
			detexPixel64GetB16(pixel),
			detexPixel64GetG16(pixel),
			detexPixel64GetR16(pixel),
			detexPixel64GetA16(pixel)
			);
		*source_pixel64_buffer = pixel;
		source_pixel64_buffer++;
	}
}

// In-place signed integer conversions (8-bit components).

static void ConvertPixel8R8ToPixel8SignedR8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	for (int i = 0; i < nu_pixels; i++) {
		int32_t red = *source_pixel_buffer;
		red -= 128;
		*source_pixel_buffer = red;
		source_pixel_buffer++;
	}
}

static void ConvertPixel16RG8ToPixel16SignedRG8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel16_buffer;
		uint32_t red = (uint8_t)((int)detexPixel32GetR8(pixel) - 128);
		uint32_t green = (uint8_t)((int)detexPixel32GetG8(pixel) - 128);
		*source_pixel16_buffer = detexPack32RG8(red, green);
		source_pixel16_buffer++;
	}
}

static void ConvertPixel8SignedR8ToPixel8R8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	for (int i = 0; i < nu_pixels; i++) {
		int32_t red = *(int8_t *)source_pixel_buffer + 128;
		*source_pixel_buffer = (uint8_t)red;
		source_pixel_buffer++;
	}
}

static void ConvertPixel16SignedRG8ToPixel16RG8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel16_buffer;
		uint32_t red = (uint8_t)(detexPixel32GetSignedR8(pixel) + 128);
		uint32_t green = (uint8_t)(detexPixel32GetSignedG8(pixel) + 128);
		*source_pixel16_buffer = detexPack32RG8(red, green);
		source_pixel16_buffer++;
	}
}

// In-place signed integer conversions (16-bit components).

static void ConvertPixel16R16ToPixel16SignedR16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		int32_t red = *source_pixel16_buffer;
		red -= 128;
		*(int16_t *)source_pixel_buffer = (int16_t)red;
		source_pixel16_buffer++;
	}
}

static void ConvertPixel32RG16ToPixel32SignedRG16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		uint32_t red = (uint16_t)((int)detexPixel32GetR16(pixel) - 32768);
		uint32_t green = (uint16_t)((int)detexPixel32GetG16(pixel) - 32768);
		*source_pixel32_buffer = detexPack32RG16(red, green);
		source_pixel32_buffer++;
	}
}

static void ConvertPixel16SignedR16ToPixel16R16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		int32_t red = *(int16_t *)source_pixel16_buffer + 32768;
		*source_pixel16_buffer = (uint16_t)red;
		source_pixel16_buffer++;
	}
}

static void ConvertPixel32SignedRG16ToPixel32RG16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		uint32_t red = (uint16_t)(detexPixel32GetSignedR16(pixel) + 32768);
		uint32_t green = (uint16_t)(detexPixel32GetSignedG16(pixel) + 32768);
		*source_pixel32_buffer = detexPack32RG16(red, green);
		source_pixel32_buffer++;
	}
}

// Reducing the number of components.

static void ConvertPixel32RGBA8ToPixel8R8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		*target_pixel_buffer = detexPixel32GetR8(pixel);
		source_pixel32_buffer++;
		target_pixel_buffer++;
	}
}

static void ConvertPixel32RGBA8ToPixel16RG8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	uint16_t *target_pixel16_buffer = (uint16_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		*target_pixel16_buffer = (uint16_t)detexPack32RG8(
			detexPixel32GetR8(pixel), detexPixel32GetG8(pixel));
		source_pixel32_buffer++;
		target_pixel16_buffer++;
	}
}

// Increasing the number of components.

static void ConvertPixel8R8ToPixel32RGBX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t red = *source_pixel_buffer;
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(red, 0, 0);
		source_pixel_buffer++;
		target_pixel32_buffer++;
	}
}

static void ConvertPixel16RG8ToPixel32RGBX8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel16_buffer;
		uint32_t red = detexPixel32GetR8(pixel);
		uint32_t green = detexPixel32GetG8(pixel);
		*target_pixel32_buffer = detexPack32RGB8Alpha0xFF(red, green, 0);
		source_pixel16_buffer++;
		target_pixel32_buffer++;
	}
}

// Conversion to component of different size.

static void ConvertPixel16R16ToPixel8R8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel16_buffer;
		*target_pixel_buffer = detexPixel32GetR16(pixel) * 255 / 65535;
		source_pixel16_buffer++;
		target_pixel_buffer++;
	}
}

static void ConvertPixel32RG16ToPixel16RG8(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint32_t *source_pixel32_buffer = (uint32_t *)source_pixel_buffer;
	uint16_t *target_pixel16_buffer = (uint16_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel32_buffer;
		*target_pixel16_buffer = (uint16_t)detexPack32RG8(
			detexPixel32GetR16(pixel) * 255 / 65535,
			detexPixel32GetG16(pixel) * 255 / 65535	
			);
		source_pixel32_buffer++;
		target_pixel16_buffer++;
	}
}

static void ConvertPixel8R8ToPixel16R16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *target_pixel16_buffer = (uint16_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel_buffer;
		*target_pixel16_buffer = pixel * 65535 / 255;
		source_pixel_buffer++;
		target_pixel16_buffer++;
	}
}

static void ConvertPixel16RG8ToPixel32RG16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	uint32_t *target_pixel32_buffer = (uint32_t *)target_pixel_buffer;
	for (int i = 0; i < nu_pixels; i++) {
		uint32_t pixel = *source_pixel16_buffer;
		*target_pixel32_buffer = detexPack32RG16(
			detexPixel32GetR8(pixel) * 65535 / 255,
			detexPixel32GetG8(pixel) * 65535 / 255
			);
		source_pixel16_buffer++;
		target_pixel32_buffer++;
	}
}

// Misc.

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

typedef void (*detexConversionFunc)(uint8_t *source_pixel_buffer, int nu_pixels,
	uint8_t *target_pixel_buffer);

typedef struct {
	uint32_t source_format;
	uint32_t target_format;
	detexConversionFunc conversion_func;
} detexConversionType;

// Conversion table. Conversions for which the source pixel size is equal to the
// target pixel size are performed in-place on the source pixel buffer.
detexConversionType detex_conversion_table[] = {
	// No-ops (in-place).
	{ DETEX_PIXEL_FORMAT_RGBX8, DETEX_PIXEL_FORMAT_RGBA8, ConvertNoop },
	{ DETEX_PIXEL_FORMAT_RGBA8, DETEX_PIXEL_FORMAT_RGBX8, ConvertNoop },
	{ DETEX_PIXEL_FORMAT_BGRX8, DETEX_PIXEL_FORMAT_BGRA8, ConvertNoop },
	{ DETEX_PIXEL_FORMAT_BGRA8, DETEX_PIXEL_FORMAT_BGRX8, ConvertNoop },
	// Swapping red and blue (in-place).
	{ DETEX_PIXEL_FORMAT_RGBX8, DETEX_PIXEL_FORMAT_BGRX8, ConvertPixel32RGBA8ToBGRA8 },
	{ DETEX_PIXEL_FORMAT_BGRX8, DETEX_PIXEL_FORMAT_RGBX8, ConvertPixel32RGBA8ToBGRA8 },
	{ DETEX_PIXEL_FORMAT_RGBA8, DETEX_PIXEL_FORMAT_BGRA8, ConvertPixel32RGBA8ToBGRA8 },
	{ DETEX_PIXEL_FORMAT_BGRA8, DETEX_PIXEL_FORMAT_RGBA8, ConvertPixel32RGBA8ToBGRA8 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RGBX16, DETEX_PIXEL_FORMAT_FLOAT_BGRX16, ConvertPixel64RGBX16ToBGRX16 },
	{ DETEX_PIXEL_FORMAT_FLOAT_BGRX16, DETEX_PIXEL_FORMAT_FLOAT_RGBX16, ConvertPixel64RGBX16ToBGRX16 },
	// Signed integer conversions (in-place).
	{ DETEX_PIXEL_FORMAT_R8, DETEX_PIXEL_FORMAT_SIGNED_R8, ConvertPixel8R8ToPixel8SignedR8 },
	{ DETEX_PIXEL_FORMAT_RG8, DETEX_PIXEL_FORMAT_SIGNED_RG8, ConvertPixel16RG8ToPixel16SignedRG8 },
	{ DETEX_PIXEL_FORMAT_SIGNED_R8, DETEX_PIXEL_FORMAT_R8, ConvertPixel8SignedR8ToPixel8R8 },
	{ DETEX_PIXEL_FORMAT_SIGNED_RG8, DETEX_PIXEL_FORMAT_RG8, ConvertPixel16SignedRG8ToPixel16RG8 },
	{ DETEX_PIXEL_FORMAT_R16, DETEX_PIXEL_FORMAT_SIGNED_R16, ConvertPixel16R16ToPixel16SignedR16 },
	{ DETEX_PIXEL_FORMAT_RG16, DETEX_PIXEL_FORMAT_SIGNED_RG16, ConvertPixel32RG16ToPixel32SignedRG16 },
	{ DETEX_PIXEL_FORMAT_SIGNED_R16, DETEX_PIXEL_FORMAT_R16, ConvertPixel16SignedR16ToPixel16R16 },
	{ DETEX_PIXEL_FORMAT_SIGNED_RG16, DETEX_PIXEL_FORMAT_RG16, ConvertPixel32SignedRG16ToPixel32RG16 },
	// Reducing the number of components.
	{ DETEX_PIXEL_FORMAT_RGBA8, DETEX_PIXEL_FORMAT_R8, ConvertPixel32RGBA8ToPixel8R8 },
	{ DETEX_PIXEL_FORMAT_RGBA8, DETEX_PIXEL_FORMAT_RG8, ConvertPixel32RGBA8ToPixel16RG8 },
	// Increasing the number of components.
	{ DETEX_PIXEL_FORMAT_R8, DETEX_PIXEL_FORMAT_RGBX8, ConvertPixel8R8ToPixel32RGBX8 },
	{ DETEX_PIXEL_FORMAT_RG8, DETEX_PIXEL_FORMAT_RGBX8, ConvertPixel16RG8ToPixel32RGBX8 },
	// Conversion to component of different size.
	{ DETEX_PIXEL_FORMAT_R16, DETEX_PIXEL_FORMAT_R8, ConvertPixel16R16ToPixel8R8 },
	{ DETEX_PIXEL_FORMAT_RG16, DETEX_PIXEL_FORMAT_RG8, ConvertPixel32RG16ToPixel16RG8 },
	{ DETEX_PIXEL_FORMAT_R8, DETEX_PIXEL_FORMAT_R16, ConvertPixel8R8ToPixel16R16 },
	{ DETEX_PIXEL_FORMAT_RG8, DETEX_PIXEL_FORMAT_RG16, ConvertPixel16RG8ToPixel32RG16 },
};

#define NU_CONVERSION_TYPES (sizeof(detex_conversion_table) / sizeof(detex_conversion_table[0]))

static uint32_t cached_source_format = -1;
static uint32_t cached_target_format = -1;
static int cached_nu_conversions = - 1;
static uint32_t cached_conversion[4];

static void CacheResult(uint32_t source_format, uint32_t target_format, int n, uint32_t *conversion) {
	cached_source_format = source_format;
	cached_target_format = target_format;
	cached_nu_conversions = n;
//	printf("Caching conversion ( ");
	for (int i = 0; i < n; i++) {
		cached_conversion[i] = conversion[i];
//		printf("%d ", conversion[i]);
	}
//	printf(")\n");
}

// Match conversion. Returns number of conversion steps, -1 if not succesful.
static int detexMatchConversion(uint32_t source_pixel_format, uint32_t target_pixel_format,
uint32_t *conversion) {
	// Immediately return if the formats are identical.
	if (source_pixel_format == target_pixel_format)
		return 0;
	// Check whether the conversion has been cached.
	if (source_pixel_format == cached_source_format && target_pixel_format == cached_target_format) {
		for (int i = 0; i < cached_nu_conversions; i++)
			conversion[i] = cached_conversion[i];
		return cached_nu_conversions;
	}
	// First check direct conversions.
	for (int i = 0; i < NU_CONVERSION_TYPES; i++)
		if (detex_conversion_table[i].target_format == target_pixel_format 
		&& detex_conversion_table[i].source_format == source_pixel_format) {
			conversion[0] = i;
			CacheResult(source_pixel_format, target_pixel_format, 1, conversion);
			return 1;
		}
	// Check two-step conversions.
	int min_components = detexGetNumberOfComponents(source_pixel_format);
	int n = detexGetNumberOfComponents(target_pixel_format);
	if (n < min_components)
		min_components = n;
	for (int i = 0; i < NU_CONVERSION_TYPES; i++)
		if (detex_conversion_table[i].target_format == target_pixel_format) {
			// Avoid loss of components.
			if (detexGetNumberOfComponents(detex_conversion_table[i].source_format) <
			min_components)
				continue;
			conversion[1] = i;
			for (int j = 0; j < NU_CONVERSION_TYPES; j++) {
				if (detex_conversion_table[j].target_format ==
				detex_conversion_table[i].source_format &&
				detex_conversion_table[j].source_format == source_pixel_format) {
					conversion[0] = j;
					CacheResult(source_pixel_format, target_pixel_format, 2, conversion);
					return 2;
				}
			}
		}
	// Check three-step conversions.
	for (int i = 0; i < NU_CONVERSION_TYPES; i++)
		// Match the first conversion with the source format.
		if (detex_conversion_table[i].source_format == source_pixel_format) {
			// Avoid loss of components.
			if (detexGetNumberOfComponents(detex_conversion_table[i].target_format) <
			min_components)
				continue;
			conversion[0] = i;
//			printf("Trying conversion ( %d ? ? )\n", conversion[0]);
			// Match the third conversion with the target format.
			for (int j = 0; j < NU_CONVERSION_TYPES; j++)
				if (detex_conversion_table[j].target_format == target_pixel_format) {
					// Avoid loss of components.
					if (detexGetNumberOfComponents(detex_conversion_table[j].source_format) <
					min_components)
						continue;
					conversion[2] = j;
//					printf("Trying conversion ( %d ? %d )\n", conversion[0], conversion[2]);
					for (int k = 0; k < NU_CONVERSION_TYPES; k++)
						if (detex_conversion_table[k].target_format ==
						detex_conversion_table[j].source_format &&
						detex_conversion_table[k].source_format ==
						detex_conversion_table[i].target_format) {
							conversion[1] = k;
							CacheResult(source_pixel_format, target_pixel_format,
								3, conversion);
							return 3;
						}
				}
		}
	// Check four-step conversions.
	for (int i = 0; i < NU_CONVERSION_TYPES; i++)
		// Match the first conversion with the source format.
		if (detex_conversion_table[i].source_format == source_pixel_format) {
			// Avoid loss of components.
			if (detexGetNumberOfComponents(detex_conversion_table[i].target_format) <
			min_components)
				continue;
			conversion[0] = i;
//			printf("Trying conversion ( %d ? ? ? )\n", conversion[0]);
			// Match the fourth conversion with the target format.
			for (int j = 0; j < NU_CONVERSION_TYPES; j++)
				if (detex_conversion_table[j].target_format == target_pixel_format) {
					// Avoid loss of components.
					if (detexGetNumberOfComponents(detex_conversion_table[j].source_format) <
					min_components)
						continue;
					conversion[3] = j;
//					printf("Trying conversion ( %d ? ? %d )\n", conversion[0], conversion[3]);
					// Match the second conversion
					for (int k = 0; k < NU_CONVERSION_TYPES; k++)
						if (detex_conversion_table[k].source_format ==
						detex_conversion_table[i].target_format) {
							// Avoid loss of components.
							if (detexGetNumberOfComponents(
							detex_conversion_table[k].target_format) < min_components)
								continue;
							conversion[1] = k;
//							printf("Trying conversion ( %d %d ? %d )\n",
//								conversion[0], conversion[1], conversion[3]);
							// March the third conversion.
							for (int l = 0; l < NU_CONVERSION_TYPES; l++) {
								if (detex_conversion_table[l].target_format ==
								detex_conversion_table[j].source_format &&
								detex_conversion_table[l].source_format ==
								detex_conversion_table[k].target_format) {
									conversion[2] = l;
									CacheResult(source_pixel_format, target_pixel_format,
										4, conversion);
									return 4;
								}
							}
						}
				}
		}
	return - 1;
}

/* Convert pixels between different formats. Return true if successful. */
// It may be a better strategy to start with the source format instead of the target format.

bool detexConvertPixels(uint8_t * DETEX_RESTRICT source_pixel_buffer, uint32_t nu_pixels,
uint32_t source_pixel_format, uint8_t * DETEX_RESTRICT target_pixel_buffer,
uint32_t target_pixel_format) {
	uint32_t conversion[4];
	int nu_conversions = detexMatchConversion(source_pixel_format, target_pixel_format, conversion);
	if (nu_conversions < 0)
		return false;
	// Perform conversions.
	int nu_non_in_place_conversions = 0;
	int last_non_in_place_conversion = - 1;
	for (int i = 0; i < nu_conversions; i++)
		if (detexGetPixelSize(detex_conversion_table[conversion[i]].source_format)
		!= detexGetPixelSize(detex_conversion_table[conversion[i]].target_format)) {
			nu_non_in_place_conversions++;
			last_non_in_place_conversion = i;
		}
	uint8_t *temp_pixel_buffer = NULL;
	for (int i = 0; i < nu_conversions; i++) {
		if (detexGetPixelSize(detex_conversion_table[conversion[i]].source_format)
		== detexGetPixelSize(detex_conversion_table[conversion[i]].target_format)) {
			// In-place conversion.
			detex_conversion_table[conversion[i]].conversion_func(
				source_pixel_buffer, nu_pixels, NULL);
		}
		else {
			if (i == last_non_in_place_conversion) {
				detex_conversion_table[conversion[i]].conversion_func(
					source_pixel_buffer, nu_pixels, target_pixel_buffer);
				source_pixel_buffer = target_pixel_buffer;
			}
			else {
				if (temp_pixel_buffer != NULL) {
					// Error: too many temporary buffers needed.
					printf("Too many temporary buffers needed.\n");
					free(temp_pixel_buffer);
					return false;
				}
				temp_pixel_buffer = malloc(nu_pixels * detexGetPixelSize(
					detex_conversion_table[conversion[i]].target_format));
				detex_conversion_table[conversion[i]].conversion_func(
					source_pixel_buffer, nu_pixels, temp_pixel_buffer);
				source_pixel_buffer = temp_pixel_buffer;
			}
		}
	
	}
	if (temp_pixel_buffer != NULL)
		free(temp_pixel_buffer);
	if (nu_non_in_place_conversions == 0) {
		memcpy(target_pixel_buffer, source_pixel_buffer,
			detexGetBlockSize(target_pixel_format));
	}
	return true;
}

