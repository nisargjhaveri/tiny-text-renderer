#ifndef TINY_FONT_RENDERER_H
#define TINY_FONT_RENDERER_H 1

#include <hb.h>

#ifdef __cplusplus
extern "C" {
#endif

hb_font_t* ttr_create_font(const char* font_data, unsigned int font_data_size, unsigned int height);
void ttr_destroy_font(hb_font_t* font);

void ttr_measure_text(hb_font_t* font, const char *text, unsigned int *width, unsigned int *height, unsigned int *baseline);

void ttr_draw_text_on_buffer(hb_font_t* font, const char *text, unsigned int x_offset, unsigned int y_offset, unsigned int width, unsigned int height, uint8_t* pixels);
void ttr_draw_text_with_callback(hb_font_t* font, const char *text, unsigned int x_offset, unsigned int y_offset, unsigned int width, unsigned int height, void (*draw_pixel_at)(unsigned int x, unsigned int y, uint8_t mask, void* user_data), void* user_data);

#ifdef __cplusplus
}
#endif

#endif /* TINY_FONT_RENDERER_H */
