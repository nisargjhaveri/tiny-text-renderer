#include "tiny_text_renderer.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "scale.h"
#include "glyph.h"

#define max(a, b) ({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a > _b ? _a : _b; \
})

#define min(a, b) ({ \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a < _b ? _a : _b; \
})


hb_font_t* ttr_create_font(const char* font_data, unsigned int font_data_size, unsigned int height) {
    hb_blob_t *blob = hb_blob_create((const char*)font_data, font_data_size, HB_MEMORY_MODE_READONLY, NULL, NULL);
    hb_face_t *face = hb_face_create(blob, 0);
    hb_font_t *font = hb_font_create(face);

    hb_font_set_scale(font, ttr_scale_up(height), ttr_scale_up(height));

    hb_blob_destroy(blob);
    hb_face_destroy(face);

    return font;
}

void ttr_destroy_font(hb_font_t* font) {
    hb_font_destroy(font);
}


static void ttr_measure_internal(hb_font_t *font, hb_direction_t direction, unsigned int glyph_count, hb_glyph_info_t *glyph_info, hb_glyph_position_t *glyph_pos, unsigned int *width, unsigned int *height, unsigned int *baseline) {
    bool calculate_width_height = (width != NULL && height != NULL);
    bool calculate_baseline = (baseline != NULL);

    int x_min = 0, x_max = 0;
    int y_min = 0, y_max = 0;

    // if (calculate_baseline) {
    //     hb_font_extents_t extents;
    //     if (hb_font_get_h_extents(font, &extents)) {
    //         *baseline = extents.ascender;
    //         calculate_baseline = false;
    //     }
    // }

    if (!calculate_width_height && !calculate_baseline) {
        return;
    }

    int cursor_x = 0;
    int cursor_y = 0;
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid  = glyph_info[i].codepoint;

        hb_glyph_extents_t extents;
        if (hb_font_get_glyph_extents(font, glyphid, &extents)) {
            y_min = min(y_min, cursor_y + glyph_pos[i].y_offset + extents.y_bearing + extents.height);
            y_max = max(y_max, cursor_y + glyph_pos[i].y_offset + extents.y_bearing);

            x_min = min(x_min, cursor_x + glyph_pos[i].x_offset + extents.x_bearing);
            x_max = max(x_max, cursor_x + glyph_pos[i].x_offset + extents.x_bearing + extents.width);
        }

        cursor_x += glyph_pos[i].x_advance;
        cursor_y += glyph_pos[i].y_advance;
    }

    if (calculate_width_height) {
        *width = ttr_scale_down_ceil(x_max - x_min);
        *height = ttr_scale_down_ceil(y_max - y_min);
    }

    if (calculate_baseline) {
        if (HB_DIRECTION_IS_VERTICAL(direction)) {
            *baseline = ttr_scale_down_round(-x_min);
        } else {
            *baseline = ttr_scale_down_round(y_max);
        }
    }
}

void ttr_measure_text(hb_font_t* font, const char *text, unsigned int *width, unsigned int *height, unsigned int *baseline) {
    hb_buffer_t *buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, text, -1, 0, -1);

    hb_buffer_guess_segment_properties(buf);

    hb_shape(font, buf, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
    hb_direction_t direction = hb_buffer_get_direction(buf);

    ttr_measure_internal(font, direction, glyph_count, glyph_info, glyph_pos, width, height, baseline);

    hb_buffer_destroy(buf);
}

typedef struct draw_glyph_pixel_data {
    unsigned int width;
    unsigned int height;

    unsigned int offset_x;
    unsigned int offset_y;

    void (*draw_pixel_at)(unsigned int x, unsigned int y, uint8_t mask, void* user_data);
    void* user_data;
} draw_glyph_pixel_data;

static void ttr_draw_glyph_pixel_at(unsigned int x, unsigned int y, uint8_t mask, void* user_data) {
    draw_glyph_pixel_data* data = (draw_glyph_pixel_data*)user_data;

    unsigned int width = data->width;
    unsigned int height = data->height;

    const int image_x = x + data->offset_x;
    const int image_y = y + data->offset_y;

    if (image_x < 0 || image_y < 0
        || (width > 0 && image_x >= width)
        || (height > 0 && image_y >= height)) {
        return;
    }

    data->draw_pixel_at(image_x, image_y, mask, data->user_data);
}

void ttr_draw_text_with_callback(
    hb_font_t* font,
    const char *text,
    unsigned int x_offset,
    unsigned int y_offset,
    unsigned int width,
    unsigned int height,
    void (*draw_pixel_at)(unsigned int x, unsigned int y, uint8_t mask, void* user_data),
    void* user_data)
{
    hb_buffer_t *buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, text, -1, 0, -1);

    hb_buffer_guess_segment_properties(buf);

    hb_shape(font, buf, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
    hb_direction_t direction = hb_buffer_get_direction(buf);

    unsigned int baseline = 0;
    ttr_measure_internal(font, direction, glyph_count, glyph_info, glyph_pos, NULL, NULL, &baseline);

    int cursor_x = ttr_scale_up(x_offset + (HB_DIRECTION_IS_VERTICAL(direction) ? baseline : 0));
    int cursor_y = ttr_scale_up(y_offset + (HB_DIRECTION_IS_HORIZONTAL(direction) ? baseline : 0));
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid  = glyph_info[i].codepoint;

        hb_glyph_extents_t extents;
        if (!hb_font_get_glyph_extents(font, glyphid, &extents)) {
            // Error?
            continue;
        }

        int glyph_start_x = cursor_x + glyph_pos[i].x_offset + extents.x_bearing;
        int glyph_start_y = cursor_y - glyph_pos[i].y_offset - extents.y_bearing;

        draw_glyph_pixel_data data = {
            .width = width,
            .height = height,
            .offset_x = ttr_scale_down_floor(glyph_start_x),
            .offset_y = ttr_scale_down_floor(glyph_start_y),
            .draw_pixel_at = draw_pixel_at,
            .user_data = user_data
        };
        ttr_draw_glyph(font, glyphid, extents, ttr_fraction_scaled(glyph_start_x), ttr_fraction_scaled(glyph_start_y), ttr_draw_glyph_pixel_at, &data);

        cursor_x += glyph_pos[i].x_advance;
        cursor_y += glyph_pos[i].y_advance;
    }

    hb_buffer_destroy(buf);
}

typedef struct draw_pixel_on_buffer_data {
    uint8_t* pixels;
    unsigned int width;
} draw_pixel_on_buffer_data;

static void ttr_draw_pixel_on_buffer(unsigned int x, unsigned int y, uint8_t mask, void* user_data) {
    draw_pixel_on_buffer_data* data = (draw_pixel_on_buffer_data*)user_data;

    const int image_i = (y * data->width) + x;
    data->pixels[image_i] = min(data->pixels[image_i] + mask, 255);
}

void ttr_draw_text_on_buffer(hb_font_t* font, const char *text, unsigned int x_offset, unsigned int y_offset, unsigned int width, unsigned int height, uint8_t* pixels) {
    draw_pixel_on_buffer_data data = { pixels, width };
    ttr_draw_text_with_callback(font, text, x_offset, y_offset, width, height, ttr_draw_pixel_on_buffer, &data);
}
