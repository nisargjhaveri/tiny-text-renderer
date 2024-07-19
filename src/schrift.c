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

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "schrift.h"

/* macros */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define SIGN(x)   (((x) > 0) - ((x) < 0))

/* structs */
typedef struct Cell    Cell;
typedef struct Raster  Raster;

struct Cell  { float area, cover; };

struct Raster
{
	Cell *cells;
	int   width;
	int   height;
};

/* function declarations */
static inline int fast_floor(float x);
static inline int fast_ceil (float x);
/* simple mathematical operations */
static SFT_Point midpoint(SFT_Point a, SFT_Point b);
static void transform_points(unsigned int numPts, SFT_Point *points, float trf[6]);
static void clip_points(unsigned int numPts, SFT_Point *points, int width, int height);
/* 'outline' data structure management */
// static int  init_outline(SFT_Outline *outl);
// static void free_outline(SFT_Outline *outl);
static int  grow_points (SFT_Outline *outl);
static int  grow_curves (SFT_Outline *outl);
static int  grow_lines  (SFT_Outline *outl);
/* tesselation */
static int  is_flat(SFT_Outline *outl, SFT_Curve curve);
static int  tesselate_curve(SFT_Curve curve, SFT_Outline *outl);
static int  tesselate_curves(SFT_Outline *outl);
/* silhouette rasterization */
static void draw_line(Raster buf, SFT_Point origin, SFT_Point goal);
static void draw_lines(SFT_Outline *outl, Raster buf);
/* post-processing */
static void post_process(Raster buf, SFT_Image *image);
/* glyph rendering */
// static int  render_outline(SFT_Outline *outl, float transform[6], SFT_Image image);

/* function implementations */

/* TODO maybe we should use long here instead of int. */
static inline int
fast_floor(float x)
{
	int i = (int) x;
	return i - (i > x);
}

static inline int
fast_ceil(float x)
{
	int i = (int) x;
	return i + (i < x);
}

static SFT_Point
midpoint(SFT_Point a, SFT_Point b)
{
	return (SFT_Point) {
		0.5 * (a.x + b.x),
		0.5 * (a.y + b.y)
	};
}

/* Applies an affine linear transformation matrix to a set of points. */
static void
transform_points(unsigned int numPts, SFT_Point *points, float trf[6])
{
	SFT_Point pt;
	unsigned int i;
	for (i = 0; i < numPts; ++i) {
		pt = points[i];
		points[i] = (SFT_Point) {
			pt.x * trf[0] + pt.y * trf[2] + trf[4],
			pt.x * trf[1] + pt.y * trf[3] + trf[5]
		};
	}
}

static void
clip_points(unsigned int numPts, SFT_Point *points, int width, int height)
{
	SFT_Point pt;
	unsigned int i;

	for (i = 0; i < numPts; ++i) {
		pt = points[i];

		if (pt.x < 0.0) {
			points[i].x = 0.0;
		}
		if (pt.x >= width) {
			points[i].x = nextafter(width, 0.0);
		}
		if (pt.y < 0.0) {
			points[i].y = 0.0;
		}
		if (pt.y >= height) {
			points[i].y = nextafter(height, 0.0);
		}
	}
}

int
sft_init_outline(SFT_Outline *outl)
{
	/* TODO Smaller initial allocations */
	outl->numPoints = 0;
	outl->capPoints = 64;
	if (!(outl->points = malloc(outl->capPoints * sizeof *outl->points)))
		return -1;
	outl->numCurves = 0;
	outl->capCurves = 64;
	if (!(outl->curves = malloc(outl->capCurves * sizeof *outl->curves)))
		return -1;
	outl->numLines = 0;
	outl->capLines = 64;
	if (!(outl->lines = malloc(outl->capLines * sizeof *outl->lines)))
		return -1;
	return 0;
}

void
sft_free_outline(SFT_Outline *outl)
{
	free(outl->points);
	free(outl->curves);
	free(outl->lines);
}

static int
grow_points(SFT_Outline *outl)
{
	void *mem;
	uint_fast16_t cap;
	assert(outl->capPoints);
	/* Since we use uint_fast16_t for capacities, we have to be extra careful not to trigger integer overflow. */
	if (outl->capPoints > UINT16_MAX / 2)
		return -1;
	cap = (uint_fast16_t) (2U * outl->capPoints);
	if (!(mem = realloc(outl->points, cap * sizeof *outl->points)))
		return -1;
	outl->capPoints = (uint_least16_t) cap;
	outl->points    = mem;
	return 0;
}

static int
grow_curves(SFT_Outline *outl)
{
	void *mem;
	uint_fast16_t cap;
	assert(outl->capCurves);
	if (outl->capCurves > UINT16_MAX / 2)
		return -1;
	cap = (uint_fast16_t) (2U * outl->capCurves);
	if (!(mem = realloc(outl->curves, cap * sizeof *outl->curves)))
		return -1;
	outl->capCurves = (uint_least16_t) cap;
	outl->curves    = mem;
	return 0;
}

static int
grow_lines(SFT_Outline *outl)
{
	void *mem;
	uint_fast16_t cap;
	assert(outl->capLines);
	if (outl->capLines > UINT16_MAX / 2)
		return -1;
	cap = (uint_fast16_t) (2U * outl->capLines);
	if (!(mem = realloc(outl->lines, cap * sizeof *outl->lines)))
		return -1;
	outl->capLines = (uint_least16_t) cap;
	outl->lines    = mem;
	return 0;
}

/* A heuristic to tell whether a given curve can be approximated closely enough by a line. */
static int
is_flat(SFT_Outline *outl, SFT_Curve curve)
{
	const float maxArea2 = 2.0;
	SFT_Point a = outl->points[curve.beg];
	SFT_Point b = outl->points[curve.ctrl];
	SFT_Point c = outl->points[curve.end];
	SFT_Point g = { b.x-a.x, b.y-a.y };
	SFT_Point h = { c.x-a.x, c.y-a.y };
	float area2 = fabs(g.x*h.y-h.x*g.y);
	return area2 <= maxArea2;
}

static int
tesselate_curve(SFT_Curve curve, SFT_Outline *outl)
{
	/* From my tests I can conclude that this stack barely reaches a top height
	 * of 4 elements even for the largest font sizes I'm willing to support. And
	 * as space requirements should only grow logarithmically, I think 10 is
	 * more than enough. */
#define STACK_SIZE 10
	SFT_Curve stack[STACK_SIZE];
	unsigned int top = 0;
	for (;;) {
		if (is_flat(outl, curve) || top >= STACK_SIZE) {
			if (outl->numLines >= outl->capLines && grow_lines(outl) < 0)
				return -1;
			outl->lines[outl->numLines++] = (SFT_Line) { curve.beg, curve.end };
			if (top == 0) break;
			curve = stack[--top];
		} else {
			uint_least16_t ctrl0 = outl->numPoints;
			if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
				return -1;
			outl->points[ctrl0] = midpoint(outl->points[curve.beg], outl->points[curve.ctrl]);
			++outl->numPoints;

			uint_least16_t ctrl1 = outl->numPoints;
			if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
				return -1;
			outl->points[ctrl1] = midpoint(outl->points[curve.ctrl], outl->points[curve.end]);
			++outl->numPoints;

			uint_least16_t pivot = outl->numPoints;
			if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0)
				return -1;
			outl->points[pivot] = midpoint(outl->points[ctrl0], outl->points[ctrl1]);
			++outl->numPoints;

			stack[top++] = (SFT_Curve) { curve.beg, pivot, ctrl0 };
			curve = (SFT_Curve) { pivot, curve.end, ctrl1 };
		}
	}
	return 0;
#undef STACK_SIZE
}

static int
tesselate_curves(SFT_Outline *outl)
{
	unsigned int i;
	for (i = 0; i < outl->numCurves; ++i) {
		if (tesselate_curve(outl->curves[i], outl) < 0)
			return -1;
	}
	return 0;
}

/* Draws a line into the buffer. Uses a custom 2D raycasting algorithm to do so. */
static void
draw_line(Raster buf, SFT_Point origin, SFT_Point goal)
{
	SFT_Point delta;
	SFT_Point nextCrossing;
	SFT_Point crossingIncr;
	float halfDeltaX;
	float prevDistance = 0.0, nextDistance;
	float xAverage, yDifference;
	struct { int x, y; } pixel;
	struct { int x, y; } dir;
	int step, numSteps = 0;
	Cell *restrict cptr, cell;

	delta.x = goal.x - origin.x;
	delta.y = goal.y - origin.y;
	dir.x = SIGN(delta.x);
	dir.y = SIGN(delta.y);

	if (!dir.y) {
		return;
	}
	
	crossingIncr.x = dir.x ? fabs(1.0 / delta.x) : 1.0;
	crossingIncr.y = fabs(1.0 / delta.y);

	if (!dir.x) {
		pixel.x = fast_floor(origin.x);
		nextCrossing.x = 100.0;
	} else {
		if (dir.x > 0) {
			pixel.x = fast_floor(origin.x);
			nextCrossing.x = (origin.x - pixel.x) * crossingIncr.x;
			nextCrossing.x = crossingIncr.x - nextCrossing.x;
			numSteps += fast_ceil(goal.x) - fast_floor(origin.x) - 1;
		} else {
			pixel.x = fast_ceil(origin.x) - 1;
			nextCrossing.x = (origin.x - pixel.x) * crossingIncr.x;
			numSteps += fast_ceil(origin.x) - fast_floor(goal.x) - 1;
		}
	}

	if (dir.y > 0) {
		pixel.y = fast_floor(origin.y);
		nextCrossing.y = (origin.y - pixel.y) * crossingIncr.y;
		nextCrossing.y = crossingIncr.y - nextCrossing.y;
		numSteps += fast_ceil(goal.y) - fast_floor(origin.y) - 1;
	} else {
		pixel.y = fast_ceil(origin.y) - 1;
		nextCrossing.y = (origin.y - pixel.y) * crossingIncr.y;
		numSteps += fast_ceil(origin.y) - fast_floor(goal.y) - 1;
	}

	nextDistance = MIN(nextCrossing.x, nextCrossing.y);
	halfDeltaX = 0.5 * delta.x;

	for (step = 0; step < numSteps; ++step) {
		xAverage = origin.x + (prevDistance + nextDistance) * halfDeltaX;
		yDifference = (nextDistance - prevDistance) * delta.y;
		cptr = &buf.cells[pixel.y * buf.width + pixel.x];
		cell = *cptr;
		cell.cover += yDifference;
		xAverage -= (float) pixel.x;
		cell.area += (1.0 - xAverage) * yDifference;
		*cptr = cell;
		prevDistance = nextDistance;
		int alongX = nextCrossing.x < nextCrossing.y;
		pixel.x += alongX ? dir.x : 0;
		pixel.y += alongX ? 0 : dir.y;
		nextCrossing.x += alongX ? crossingIncr.x : 0.0;
		nextCrossing.y += alongX ? 0.0 : crossingIncr.y;
		nextDistance = MIN(nextCrossing.x, nextCrossing.y);
	}

	xAverage = origin.x + (prevDistance + 1.0) * halfDeltaX;
	yDifference = (1.0 - prevDistance) * delta.y;
	cptr = &buf.cells[pixel.y * buf.width + pixel.x];
	cell = *cptr;
	cell.cover += yDifference;
	xAverage -= (float) pixel.x;
	cell.area += (1.0 - xAverage) * yDifference;
	*cptr = cell;
}

static void
draw_lines(SFT_Outline *outl, Raster buf)
{
	unsigned int i;
	for (i = 0; i < outl->numLines; ++i) {
		SFT_Line  line   = outl->lines[i];
		SFT_Point origin = outl->points[line.beg];
		SFT_Point goal   = outl->points[line.end];
		draw_line(buf, origin, goal);
	}
}

/* Integrate the values in the buffer to arrive at the final grayscale image. */
static void
post_process(Raster buf, SFT_Image *image)
{
	Cell cell;
	float accum = 0.0, value;
	unsigned int i, num;
	num = (unsigned int) buf.width * (unsigned int) buf.height;
	for (i = 0; i < num; ++i) {
		cell     = buf.cells[i];
		value    = fabs(accum + cell.area);
		value    = MIN(value, 1.0);
		value    = value * 255.0 + 0.5;
		accum   += cell.cover;

		image->draw_pixel_at(i % image->width, i / image->width, (uint8_t)value, image->user_data);
	}
}

int
sft_add_point(SFT_Outline *outl, float x, float y)
{
	if (outl->numPoints >= outl->capPoints && grow_points(outl) < 0) {
		return -1;
	}

	outl->points[outl->numPoints++] = (SFT_Point) { x, y };
	return 0;
}

int
sft_add_curve(SFT_Outline *outl, uint_least16_t beg, uint_least16_t ctrl, uint_least16_t end)
{
	if (outl->numCurves >= outl->capCurves && grow_curves(outl) < 0) {
		return -1;
	}

	outl->curves[outl->numCurves++] = (SFT_Curve) { beg, end, ctrl };
	return 0;
}

int
sft_add_line(SFT_Outline *outl, uint_least16_t beg, uint_least16_t end)
{
	if (outl->numLines >= outl->capLines && grow_lines(outl) < 0) {
		return -1;
	}

	outl->lines[outl->numLines++] = (SFT_Line) { beg, end };
	return 0;
}

int
sft_render_outline(SFT_Outline *outl, float transform[6], SFT_Image image)
{
	Cell *cells = NULL;
	Raster buf;
	unsigned int numPixels;

	numPixels = (unsigned int) image.width * (unsigned int) image.height;

	cells = malloc(numPixels * sizeof *cells);
	if (!cells) {
		return -1;
	}
	memset(cells, 0, numPixels * sizeof *cells);
	buf.cells  = cells;
	buf.width  = image.width;
	buf.height = image.height;

	transform_points(outl->numPoints, outl->points, transform);

	clip_points(outl->numPoints, outl->points, image.width, image.height);

	if (tesselate_curves(outl) < 0) {
		free(cells);
		return -1;
	}

	draw_lines(outl, buf);

	post_process(buf, &image);

	free(cells);
	return 0;
}
