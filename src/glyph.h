#ifndef TTR_GLYPH_H
#define TTR_GLYPH_H 1

#include <hb.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Draw a glyph to a pixel buffer.
 */
int ttr_draw_glyph(
    hb_font_t* font,
    hb_codepoint_t glyph,
    hb_glyph_extents_t extents,
    void (*draw_pixel_at)(unsigned int x, unsigned int y, uint8_t mask, void* user_data),
    void* user_data
);

#ifdef __cplusplus
}
#endif

#endif /* TTR_GLYPH_H */
