#ifndef TTR_GLYPH_H
#define TTR_GLYPH_H 1

#include <hb.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Draw a glyph to a pixel buffer.
 * 
 * @param font The font to use.
 * @param glyph Glyph id to draw.
 * @param extents Extents of the glyph.
 * @param offset_x Fractional part of x offset adjustment for the glyph.
 * @param offset_y Fractional part of y offset adjustment for the glyph.
 * @param draw_pixel_at Callback to draw a pixel at a given position.
 * @param user_data User data to pass to the callback.
 */
int ttr_draw_glyph(
    hb_font_t* font,
    hb_codepoint_t glyph,
    hb_glyph_extents_t extents,
    unsigned int offset_x,
    unsigned int offset_y,
    void (*draw_pixel_at)(unsigned int x, unsigned int y, uint8_t mask, void* user_data),
    void* user_data
);

#ifdef __cplusplus
}
#endif

#endif /* TTR_GLYPH_H */
