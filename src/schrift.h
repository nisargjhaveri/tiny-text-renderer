/* This file is adapted from libschrift (https://github.com/tomolt/libschrift/tree/24737d2922b23df4a5692014f5ba03da0c296112)
 *
 * © 2019-2022 Thomas Oltmann and contributors
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#ifndef SCHRIFT_H
#define SCHRIFT_H 1

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint_fast32_t, uint_least32_t */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SFT_Image    SFT_Image;

typedef struct SFT_Outline  SFT_Outline;
typedef struct SFT_Point   SFT_Point;
typedef struct SFT_Line    SFT_Line;
typedef struct SFT_Curve   SFT_Curve;

struct SFT_Image
{
	int   width;
	int   height;

	void (*draw_pixel_at)(unsigned int x, unsigned int y, uint8_t mask, void* user_data);
	void* user_data;
};

struct SFT_Point { float x, y; };
struct SFT_Line  { uint_least16_t beg, end; };
struct SFT_Curve { uint_least16_t beg, end, ctrl; };

struct SFT_Outline
{
	SFT_Point *points;
	SFT_Curve *curves;
	SFT_Line  *lines;
	uint_least16_t numPoints;
	uint_least16_t capPoints;
	uint_least16_t numCurves;
	uint_least16_t capCurves;
	uint_least16_t numLines;
	uint_least16_t capLines;
};

int  sft_init_outline(SFT_Outline *outl);
void sft_free_outline(SFT_Outline *outl);

int sft_add_point(SFT_Outline *outl, float x, float y);
int sft_add_curve(SFT_Outline *outl, uint_least16_t beg, uint_least16_t ctrl, uint_least16_t end);
int sft_add_line(SFT_Outline *outl, uint_least16_t beg, uint_least16_t end);

int sft_render_outline(SFT_Outline *outl, float transform[6], SFT_Image image);

#ifdef __cplusplus
}
#endif

#endif

