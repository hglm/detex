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

// Integer division using look-up tables, used by BC1/2/3 and RGTC (BC4/5)
// decompression.

#include <stdlib.h>

extern const uint8_t detex_division_by_3_table[768];

static DETEX_INLINE_ONLY uint32_t detexDivide0To767By3(uint32_t value) {
	return detex_division_by_3_table[value];
}

extern const uint8_t detex_division_by_7_table[1792];

static DETEX_INLINE_ONLY uint32_t detexDivide0To1791By7(uint32_t value) {
	return detex_division_by_7_table[value];
}

static DETEX_INLINE_ONLY int8_t detexSignInt32(int v) {
	return (int8_t)((v >> 31) | - (- v >> 31));
}

static DETEX_INLINE_ONLY int detexDivideMinus895To895By7(int value) {
	return (int8_t)detex_division_by_7_table[abs(value)] * detexSignInt32(value);
}

extern const uint8_t detex_division_by_5_table[1280];

static DETEX_INLINE_ONLY uint32_t detexDivide0To1279By5(uint32_t value) {
	return detex_division_by_5_table[value];
}

static DETEX_INLINE_ONLY int detexDivideMinus639To639By5(int value) {
	return (int8_t)detex_division_by_5_table[abs(value)] * detexSignInt32(value);
}

