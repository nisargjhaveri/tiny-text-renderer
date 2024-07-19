#ifndef TINY_FONT_RENDERER_H
#define TINY_FONT_RENDERER_H 1

#include <hb.h>

#ifdef __cplusplus
extern "C" {
#endif

hb_font_t* create_font(const char* font_data, unsigned int font_data_size, unsigned int height);
void destroy_font(hb_font_t* font);

void measure_text(hb_font_t* font, const char *text, unsigned int *width, unsigned int *height, unsigned int *baseline);

void draw_text(hb_font_t* font, const char *text, unsigned int x_offset, unsigned int y_offset, uint8_t* pixels, unsigned int width, unsigned int height);

#ifdef __cplusplus
}
#endif

#endif /* TINY_FONT_RENDERER_H */
