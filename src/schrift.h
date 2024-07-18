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

typedef struct Outline  Outline;
typedef struct Point   Point;
typedef struct Line    Line;
typedef struct Curve   Curve;

struct SFT_Image
{
	void *pixels;
	int   width;
	int   height;
};

struct Point { float x, y; };
struct Line  { uint_least16_t beg, end; };
struct Curve { uint_least16_t beg, end, ctrl; };

struct Outline
{
	Point *points;
	Curve *curves;
	Line  *lines;
	uint_least16_t numPoints;
	uint_least16_t capPoints;
	uint_least16_t numCurves;
	uint_least16_t capCurves;
	uint_least16_t numLines;
	uint_least16_t capLines;
};

int  init_outline(Outline *outl);
void free_outline(Outline *outl);

int add_point(Outline *outl, float x, float y);
int add_curve(Outline *outl, uint_least16_t beg, uint_least16_t ctrl, uint_least16_t end);
int add_line(Outline *outl, uint_least16_t beg, uint_least16_t end);

int render_outline(Outline *outl, float transform[6], SFT_Image image);

#ifdef __cplusplus
}
#endif

#endif

