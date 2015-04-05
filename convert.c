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
#include "half-float.h"
#include "hdr.h"

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
		red -= 32768;
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
		*target_pixel_buffer = (detexPixel32GetR16(pixel) + 127) * 255 / 65535;
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
			(detexPixel32GetR16(pixel) + 127) * 255 / 65535,
			(detexPixel32GetG16(pixel) + 127) * 255 / 65535	
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

// Float to half-float conversion.

static void ConvertPixel32FloatR32ToPixel16FloatR16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertFloatToHalfFloat((float *)source_pixel_buffer, nu_pixels, (uint16_t *)target_pixel_buffer);
}

static void ConvertPixel64FloatRG32ToPixel32FloatRG16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertFloatToHalfFloat((float *)source_pixel_buffer, nu_pixels * 2, (uint16_t *)target_pixel_buffer);
}

static void ConvertPixel128FloatRGBX32ToPixel64FloatRGBX16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertFloatToHalfFloat((float *)source_pixel_buffer, nu_pixels * 4, (uint16_t *)target_pixel_buffer);
}

// Half-float to float conversion.

static void ConvertPixel16FloatR16ToPixel32FloatR32(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertHalfFloatToFloat((uint16_t *)source_pixel_buffer, nu_pixels, (float *)target_pixel_buffer);
}

static void ConvertPixel32FloatRG16ToPixel64FloatRG32(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertHalfFloatToFloat((uint16_t *)source_pixel_buffer, nu_pixels * 2, (float *)target_pixel_buffer);
}

static void ConvertPixel64FloatRGBX16ToPixel128FloatRGBX32(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertHalfFloatToFloat((uint16_t *)source_pixel_buffer, nu_pixels * 4, (float *)target_pixel_buffer);
}

// Conversion from 16-bit integer to half-float (in-place).

static void ConvertPixel16R16ToPixel16FloatR16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	float float_buffer[64];
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	// Split conversion into stages.
	for (int i = 0; i < nu_pixels; i += 64) {
		float *target_pixelf_buffer = (float *)float_buffer;
		int nu_stage_pixels = 64;
		if (i + 64 > nu_pixels)
			nu_stage_pixels = nu_pixels - i;
		for (int j = 0; j < nu_stage_pixels; j++) {
			int16_t red = *source_pixel16_buffer;
			float redf = red * (1.0f / 65535.0f);
			*target_pixelf_buffer = redf;
			source_pixel16_buffer++;
			target_pixelf_buffer++;
		}
		ConvertPixel32FloatR32ToPixel16FloatR16((uint8_t *)float_buffer,
			nu_stage_pixels, source_pixel_buffer);
		source_pixel_buffer += nu_stage_pixels * 2;
	}
}

static void ConvertPixel32RG16ToPixel32FloatRG16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	float float_buffer[128];
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	// Split conversion into stages.
	for (int i = 0; i < nu_pixels; i += 64) {
		float *target_pixelf_buffer = (float *)float_buffer;
		int nu_stage_pixels = 64;
		if (i + 64 > nu_pixels)
			nu_stage_pixels = nu_pixels - i;
		for (int j = 0; j < nu_stage_pixels; j++) {
			int16_t red = source_pixel16_buffer[0];
			int16_t green = source_pixel16_buffer[1];
			float redf = red * (1.0f / 65535.0f);
			float greenf = green * (1.0f / 65535.0f);
			target_pixelf_buffer[0] = redf;
			target_pixelf_buffer[1] = greenf;
			source_pixel16_buffer += 2;
			target_pixelf_buffer += 2;
		}
		ConvertPixel64FloatRG32ToPixel32FloatRG16((uint8_t *)float_buffer,
			nu_stage_pixels, source_pixel_buffer);
		source_pixel_buffer += nu_stage_pixels * 4;
	}
}

static void ConvertPixel64RGBX16ToPixel64FloatRGBX16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	float float_buffer[128];
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	// Split conversion into stages.
	for (int i = 0; i < nu_pixels; i += 32) {
		float *target_pixelf_buffer = (float *)float_buffer;
		int nu_stage_pixels = 32;
		if (i + 32 > nu_pixels)
			nu_stage_pixels = nu_pixels - i;
		for (int j = 0; j < nu_stage_pixels; j++) {
			int16_t red = source_pixel16_buffer[0];
			int16_t green = source_pixel16_buffer[1];
			int16_t blue = source_pixel16_buffer[2];
			float redf = red * (1.0f / 65535.0f);
			float greenf = green * (1.0f / 65535.0f);
			float bluef = blue  * (1.0f / 65535.0f);
			target_pixelf_buffer[0] = redf;
			target_pixelf_buffer[1] = greenf;
			target_pixelf_buffer[2] = bluef;
			target_pixelf_buffer[3] = 1.0f;
			source_pixel16_buffer += 4;
			target_pixelf_buffer += 4;
		}
		ConvertPixel128FloatRGBX32ToPixel64FloatRGBX16((uint8_t *)float_buffer,
			nu_stage_pixels, source_pixel_buffer);
		source_pixel_buffer += nu_stage_pixels * 8;
	}
}

// Conversion from normalized half-float to 16-bit integer (in-place).

static void ConvertPixel16FloatR16ToPixel16R16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertNormalizedHalfFloatToUInt16((uint16_t *)source_pixel_buffer, nu_pixels);
}

static void ConvertPixel32FloatRG16ToPixel32RG16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertNormalizedHalfFloatToUInt16((uint16_t *)source_pixel_buffer, nu_pixels * 2);
}

static void ConvertPixel64FloatRGBX16ToPixel64RGBX16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	detexConvertNormalizedHalfFloatToUInt16((uint16_t *)source_pixel_buffer, nu_pixels * 4);
}

// Conversion from HDR half-float to 16-bit integer (in-place). Depends on gamma parameters.

static void ConvertPixel16FloatR16HDRToPixel16R16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	detexConvertHDRHalfFloatToUInt16(source_pixel16_buffer, nu_pixels);
}

static void ConvertPixel32FloatRG16HDRToPixel32RG16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	detexConvertHDRHalfFloatToUInt16(source_pixel16_buffer, nu_pixels * 2);
}

static void ConvertPixel64FloatRGBX16HDRToPixel64RGBX16(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	uint16_t *source_pixel16_buffer = (uint16_t *)source_pixel_buffer;
	detexConvertHDRHalfFloatToUInt16(source_pixel16_buffer, nu_pixels * 4);
}

// Conversion HDR float to float (in_place).

static void ConvertPixel32FloatR32HDRToPixel32FloatR32(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	float *source_pixelf_buffer = (float *)source_pixel_buffer;
	detexConvertHDRFloatToFloat(source_pixelf_buffer, nu_pixels);
}

static void ConvertPixel64FloatRG32HDRToPixel64FloatRG32(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	float *source_pixelf_buffer = (float *)source_pixel_buffer;
	detexConvertHDRFloatToFloat(source_pixelf_buffer, nu_pixels * 2);
}

static void ConvertPixel128FloatRGBX32HDRToPixel128FloatRGBX32(uint8_t * DETEX_RESTRICT source_pixel_buffer,
int nu_pixels, uint8_t * DETEX_RESTRICT target_pixel_buffer) {
	float *source_pixelf_buffer = (float *)source_pixel_buffer;
	detexConvertHDRFloatToFloat(source_pixelf_buffer, nu_pixels * 4);
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
	// Integer to half-float conversion (in-place).
	{ DETEX_PIXEL_FORMAT_R16, DETEX_PIXEL_FORMAT_FLOAT_R16, ConvertPixel16R16ToPixel16FloatR16 },
	{ DETEX_PIXEL_FORMAT_RG16, DETEX_PIXEL_FORMAT_FLOAT_RG16, ConvertPixel32RG16ToPixel32FloatRG16 },
	{ DETEX_PIXEL_FORMAT_RGBX16, DETEX_PIXEL_FORMAT_FLOAT_RGBX16, ConvertPixel64RGBX16ToPixel64FloatRGBX16 },
	// Half-float to integer conversion (in-place).
	{ DETEX_PIXEL_FORMAT_FLOAT_R16, DETEX_PIXEL_FORMAT_R16, ConvertPixel16FloatR16ToPixel16R16 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RG16, DETEX_PIXEL_FORMAT_RG16, ConvertPixel32FloatRG16ToPixel32RG16 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RGBX16, DETEX_PIXEL_FORMAT_RGBX16, ConvertPixel64FloatRGBX16ToPixel64RGBX16 },
	// HDR half-float to integer conversion (in-place).
	{ DETEX_PIXEL_FORMAT_FLOAT_R16_HDR, DETEX_PIXEL_FORMAT_R16, ConvertPixel16FloatR16HDRToPixel16R16 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RG16_HDR, DETEX_PIXEL_FORMAT_RG16, ConvertPixel32FloatRG16HDRToPixel32RG16 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RGBX16_HDR, DETEX_PIXEL_FORMAT_RGBX16, ConvertPixel64FloatRGBX16HDRToPixel64RGBX16 },
	// Float to half-float conversion.
	{ DETEX_PIXEL_FORMAT_FLOAT_R32, DETEX_PIXEL_FORMAT_FLOAT_R16, ConvertPixel32FloatR32ToPixel16FloatR16 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RG32, DETEX_PIXEL_FORMAT_FLOAT_RG16, ConvertPixel64FloatRG32ToPixel32FloatRG16 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RGBX32, DETEX_PIXEL_FORMAT_FLOAT_RGBX16, ConvertPixel128FloatRGBX32ToPixel64FloatRGBX16 },
	// Half-float to float conversion.
	{ DETEX_PIXEL_FORMAT_FLOAT_R16, DETEX_PIXEL_FORMAT_FLOAT_R32, ConvertPixel16FloatR16ToPixel32FloatR32 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RG32, DETEX_PIXEL_FORMAT_FLOAT_RG32, ConvertPixel32FloatRG16ToPixel64FloatRG32 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RGBX32, DETEX_PIXEL_FORMAT_FLOAT_RGBX32, ConvertPixel64FloatRGBX16ToPixel128FloatRGBX32 },
	// HDR Float to float conversion.
	{ DETEX_PIXEL_FORMAT_FLOAT_R32_HDR, DETEX_PIXEL_FORMAT_FLOAT_R32, ConvertPixel32FloatR32HDRToPixel32FloatR32 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RG32_HDR, DETEX_PIXEL_FORMAT_FLOAT_RG32, ConvertPixel64FloatRG32HDRToPixel64FloatRG32 },
	{ DETEX_PIXEL_FORMAT_FLOAT_RGBX32_HDR, DETEX_PIXEL_FORMAT_FLOAT_RGBX32,
		ConvertPixel128FloatRGBX32HDRToPixel128FloatRGBX32 },
};

#define NU_CONVERSION_TYPES (sizeof(detex_conversion_table) / sizeof(detex_conversion_table[0]))

static __thread uint32_t cached_source_format = -1;
static __thread uint32_t cached_target_format = -1;
static __thread int cached_nu_conversions = - 1;
static __thread uint32_t cached_conversion[4];

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

// Temporary pixel buffer management for conversion function.

#define DETEX_MAX_TEMP_PIXEL_BUFFERS 3

typedef struct {
	uint8_t *pixel_buffer[DETEX_MAX_TEMP_PIXEL_BUFFERS];
	uint32_t size[DETEX_MAX_TEMP_PIXEL_BUFFERS];
	int nu_buffers;
} TempPixelBufferInfo;

static void InitTemporaryPixelBuffers(TempPixelBufferInfo *info) {
	info->nu_buffers = 0;
}

static uint8_t *AllocateTemporaryPixelBuffer(TempPixelBufferInfo *info, uint32_t size) {
	if (info->nu_buffers == DETEX_MAX_TEMP_PIXEL_BUFFERS)
		return NULL;
	uint8_t *buffer = (uint8_t *)malloc(size);
	info->pixel_buffer[info->nu_buffers] = buffer;
	info->nu_buffers++;
	return buffer;
}

static void FreeTemporaryPixelBuffers(TempPixelBufferInfo *info) {
	for (int i = 0; i < info->nu_buffers; i++)
		free(info->pixel_buffer[i]);
}

// Convert pixels between different formats. Return true if successful.
// If target_pixel_format is NULL, the conversion will be attempted in-place, without
// allocating any temporary buffer.
// In its current form, it may modify the source buffer.

bool detexConvertPixels(uint8_t * DETEX_RESTRICT source_pixel_buffer, uint32_t nu_pixels,
uint32_t source_pixel_format, uint8_t * DETEX_RESTRICT target_pixel_buffer,
uint32_t target_pixel_format) {
	uint32_t conversion[4];
	int nu_conversions = detexMatchConversion(source_pixel_format, target_pixel_format, conversion);
	if (nu_conversions < 0)
		return false;
	// Count in place/non-place steps.
	int nu_non_in_place_conversions = 0;
	int last_non_in_place_conversion = - 1;
	int first_non_in_place_conversion = - 1;
	for (int i = 0; i < nu_conversions; i++)
		if (detexGetPixelSize(detex_conversion_table[conversion[i]].source_format)
		!= detexGetPixelSize(detex_conversion_table[conversion[i]].target_format)) {
			nu_non_in_place_conversions++;
			last_non_in_place_conversion = i;
			if (first_non_in_place_conversion < 0)
				first_non_in_place_conversion = i;
		}
	if (target_pixel_buffer == NULL && nu_non_in_place_conversions > 0)
		return false;
	// Perform conversions.
	TempPixelBufferInfo temp_pixel_buffer_info;
	InitTemporaryPixelBuffers(&temp_pixel_buffer_info);
	if (first_non_in_place_conversion > 0) {
		// When doing a non-place conversion and the first conversion step is place,
		// allocate a temporary buffer to avoid corrupting the source buffer.
		uint8_t *temp_pixel_buffer = AllocateTemporaryPixelBuffer(&temp_pixel_buffer_info,
			detexGetPixelSize(source_pixel_format) * nu_pixels);
		memcpy(temp_pixel_buffer, source_pixel_buffer,
			detexGetPixelSize(source_pixel_format) * nu_pixels);
		source_pixel_buffer = temp_pixel_buffer;
	}
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
				uint8_t *temp_pixel_buffer = AllocateTemporaryPixelBuffer(&temp_pixel_buffer_info,
					nu_pixels * detexGetPixelSize(
						detex_conversion_table[conversion[i]].target_format));
				if (temp_pixel_buffer == NULL) {
					// Error: Too many temporary buffers needed.
					printf("Too many temporary buffers needed.\n");
					FreeTemporaryPixelBuffers(&temp_pixel_buffer_info);
					return false;
				}
				detex_conversion_table[conversion[i]].conversion_func(
					source_pixel_buffer, nu_pixels, temp_pixel_buffer);
				source_pixel_buffer = temp_pixel_buffer;
			}
		}
	
	}
	FreeTemporaryPixelBuffers(&temp_pixel_buffer_info);
	if (nu_non_in_place_conversions == 0) {
		memcpy(target_pixel_buffer, source_pixel_buffer,
			detexGetPixelSize(target_pixel_format) * 16);
	}
	return true;
}

bool detexConvertPixelsInPlace(uint8_t * DETEX_RESTRICT source_pixel_buffer, uint32_t nu_pixels,
uint32_t source_pixel_format, uint32_t target_pixel_format) {
	return detexConvertPixels(source_pixel_buffer, nu_pixels, source_pixel_format, NULL, target_pixel_format);
}

