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

/* Texture file converter. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <getopt.h>

#include "detex.h"

static uint32_t input_format;
static uint32_t output_format;
static uint32_t option_flags;
static char *input_file;
static char *output_file;

static const uint32_t supported_formats[] = {
	// Uncompressed formats.
	DETEX_PIXEL_FORMAT_RGB8,
	DETEX_PIXEL_FORMAT_RGBA8,
	DETEX_PIXEL_FORMAT_R8,
	DETEX_PIXEL_FORMAT_SIGNED_R8,
	DETEX_PIXEL_FORMAT_RG8,
	DETEX_PIXEL_FORMAT_SIGNED_RG8,
	DETEX_PIXEL_FORMAT_R16,
	DETEX_PIXEL_FORMAT_SIGNED_R16,
	DETEX_PIXEL_FORMAT_RG16,
	DETEX_PIXEL_FORMAT_SIGNED_RG16,
	DETEX_PIXEL_FORMAT_RGBA16,
	DETEX_PIXEL_FORMAT_FLOAT_R16,
	DETEX_PIXEL_FORMAT_FLOAT_RG16,
	DETEX_PIXEL_FORMAT_FLOAT_RGB16,
	DETEX_PIXEL_FORMAT_FLOAT_RGBA16,
	DETEX_PIXEL_FORMAT_FLOAT_R32,
	DETEX_PIXEL_FORMAT_FLOAT_RG32,
	DETEX_PIXEL_FORMAT_FLOAT_RGB32,
	DETEX_PIXEL_FORMAT_FLOAT_RGBA32,
	DETEX_PIXEL_FORMAT_A8,
	// Compressed formats.
	DETEX_TEXTURE_FORMAT_BC1,
	DETEX_TEXTURE_FORMAT_BC1A,
	DETEX_TEXTURE_FORMAT_BC2,
	DETEX_TEXTURE_FORMAT_BC3,
	DETEX_TEXTURE_FORMAT_RGTC1,
	DETEX_TEXTURE_FORMAT_SIGNED_RGTC1,
	DETEX_TEXTURE_FORMAT_RGTC2,
	DETEX_TEXTURE_FORMAT_SIGNED_RGTC2,
	DETEX_TEXTURE_FORMAT_BPTC_FLOAT,
	DETEX_TEXTURE_FORMAT_BPTC_SIGNED_FLOAT,
	DETEX_TEXTURE_FORMAT_BPTC,
	DETEX_TEXTURE_FORMAT_ETC1,
	DETEX_TEXTURE_FORMAT_ETC2,
	DETEX_TEXTURE_FORMAT_ETC2_PUNCHTHROUGH,
	DETEX_TEXTURE_FORMAT_ETC2_EAC,
	DETEX_TEXTURE_FORMAT_EAC_R11,
	DETEX_TEXTURE_FORMAT_EAC_SIGNED_R11,
	DETEX_TEXTURE_FORMAT_EAC_RG11,
	DETEX_TEXTURE_FORMAT_EAC_SIGNED_RG11,
};

#define NU_SUPPORTED_FORMATS (sizeof(supported_formats) / sizeof(supported_formats[0]))

enum {
	OPTION_FLAG_OUTPUT_FORMAT = 0x1,
	OPTION_FLAG_INPUT_FORMAT = 0x2,
};

static const struct option long_options[] = {
	// Option name, argument flag, NULL, equivalent short option character.
	{ "format", required_argument, NULL, 'f' },
	{ "output-format", required_argument, NULL, 'o' },
	{ "input-format", required_argument, NULL, 'i' },
	{ NULL, 0, NULL, 0 }
};

#define NU_OPTIONS (sizeof(long_options) / sizeof(long_options[0]))

static void Message(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

static __attribute ((noreturn)) void FatalError(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	exit(1);
}

static void Usage() {
	Message("detex-convert %s\n", DETEX_VERSION);
	Message("Convert and decompress uncompressed and compressed texture files (KTX, DDS, raw)\n");
	Message("Usage: detex-convert [<OPTIONS>] <INPUTFILE> <OUTPUTFILE>\n");
	Message("Options:\n");
	for (int i = 0;; i++) {
		if (long_options[i].name == NULL)
			break;
		const char *value_str = " <VALUE>";
		if (long_options[i].has_arg)
			Message("    -%c%s, --%s%s, --%s=%s\n", long_options[i].val, value_str,
				long_options[i].name, value_str, long_options[i].name, &value_str[1]);
		else
			Message("    -%c, --%s\n", long_options[i].val, long_options[i].name);
	}
	Message("File formats supported: KTX, DDS, raw (no header)\n");
	Message("Supported formats:\n");
	int column = 0;
	for (int i = 0; i < NU_SUPPORTED_FORMATS; i++) {
		const char *format_text1 = detexGetTextureFormatText(supported_formats[i]);
		const char *format_text2 = detexGetAlternativeTextureFormatText(supported_formats[i]);
		int length1 = strlen(format_text1);
		int length2 = strlen(format_text2);
		int length = length1;
		if (length2 > 0)
			length += 3 + length2;	
		if (column + length > 78) {
			Message("\n");
			column = 0;
		}
		Message("%s", format_text1);
		if (length2 > 0)
			Message(" (%s)", format_text2);
		column += length;
		if (i < NU_SUPPORTED_FORMATS - 1) {
			Message(", ");
			column += 2;
		}
	}
	Message("\n");
}

static uint32_t ParseFormat(const char *s) {
	for (int i = 0; i < NU_SUPPORTED_FORMATS; i++) {
		const char *format_text1 = detexGetTextureFormatText(supported_formats[i]);
		if (strcasecmp(s, format_text1) == 0)
			return supported_formats[i];
		const char *format_text2 = detexGetAlternativeTextureFormatText(supported_formats[i]);
		if (strlen(format_text2) > 0 && strcasecmp(s, format_text2) == 0)
			return supported_formats[i];
	}
	FatalError("Fatal error: Format %s not recognized\n" ,s);
}

static void ParseArguments(int argc, char **argv) {
	while (true) {
	int option_index = 0;
		int c = getopt_long(argc, argv, "f:t:s:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c) {
		case 'f' :	// -f, --format
		case 'o' :	// -o, --target-format
			output_format = ParseFormat(optarg);
			option_flags |= OPTION_FLAG_OUTPUT_FORMAT;
			break;
		case 'i' :	// -i, --input-format
			input_format = ParseFormat(optarg);
			option_flags |= OPTION_FLAG_INPUT_FORMAT;
			break;
		default :
			FatalError("Unknown option.");
			break;
		}
	}

	if (optind + 1 >= argc)
		FatalError("Fatal error: Expected input and output filename arguments\n");
	input_file = strdup(argv[optind]);
	output_file = strdup(argv[optind + 1]);
	char s[80];
	if (option_flags & OPTION_FLAG_INPUT_FORMAT) {
		sprintf(s, "%s (specified)", detexGetTextureFormatText(input_format));
	}
	else
		sprintf(s, "auto-detected");
	printf("Input file: %s, format %s\n", input_file, s);
	if (option_flags & OPTION_FLAG_OUTPUT_FORMAT) {
		sprintf(s, "%s (specified)", detexGetTextureFormatText(output_format));
	}
	else
		sprintf(s, "taken from input");
	Message("Output file: %s, format %s\n", output_file, s);
}

int main(int argc, char **argv) {
	if (argc == 1) {
		Usage();
		exit(0);
	}
	Message("detex-convert %s\n", DETEX_VERSION);
	ParseArguments(argc, argv);
}

