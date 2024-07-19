#ifndef TTR_GLYPH_H
#define TTR_GLYPH_H 1

#include <hb.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Draw a glyph to a pixel buffer.
 */
int ttr_draw_glyph(hb_font_t* font, hb_codepoint_t glyph, hb_glyph_extents_t extents, uint8_t* pixels);

#ifdef __cplusplus
}
#endif

#endif /* TTR_GLYPH_H */
