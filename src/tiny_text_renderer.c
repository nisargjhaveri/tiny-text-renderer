#include "tiny_text_renderer.h"

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "harfbuzz_schrift.h"

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


hb_font_t* create_font(const char* font_data, unsigned int font_data_size, unsigned int height) {
    hb_blob_t *blob = hb_blob_create((const char*)font_data, font_data_size, HB_MEMORY_MODE_READONLY, NULL, NULL);
    hb_face_t *face = hb_face_create(blob, 0);
    hb_font_t *font = hb_font_create(face);

    hb_font_set_scale(font, height, height);

    hb_blob_destroy(blob);
    hb_face_destroy(face);

    return font;
}

void destroy_font(hb_font_t* font) {
    hb_font_destroy(font);
}


static void measure_internal(hb_font_t *font, unsigned int glyph_count, hb_glyph_info_t *glyph_info, hb_glyph_position_t *glyph_pos, unsigned int *width, unsigned int *height, unsigned int *baseline) {
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

    int x_offset = 0;
    int y_offset = 0;
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid  = glyph_info[i].codepoint;

        hb_glyph_extents_t extents;
        if (hb_font_get_glyph_extents(font, glyphid, &extents)) {
            y_min = min(y_min, extents.y_bearing + extents.height);
            y_max = max(y_max, extents.y_bearing);

            x_min = min(x_min, x_offset + extents.x_bearing);
            x_max = max(x_max, x_offset + extents.x_bearing + extents.width);
        }

        x_offset += glyph_pos[i].x_advance;
        y_offset += glyph_pos[i].y_advance;
    }

    if (calculate_width_height) {
        *width = x_max - x_min;
        *height = y_max - y_min;
    }

    if (calculate_baseline) {
        *baseline = y_max;
    }
}

void measure_text(hb_font_t* font, const char *text, unsigned int *width, unsigned int *height, unsigned int *baseline) {
    hb_buffer_t *buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, text, -1, 0, -1);

    hb_buffer_guess_segment_properties(buf);

    hb_shape(font, buf, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    measure_internal(font, glyph_count, glyph_info, glyph_pos, width, height, baseline);

    hb_buffer_destroy(buf);
}

void draw_text(hb_font_t* font, const char *text, unsigned int x_offset, unsigned int y_offset, uint8_t* pixels, unsigned int width, unsigned int height) {
    hb_buffer_t *buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, text, -1, 0, -1);

    hb_buffer_guess_segment_properties(buf);

    hb_shape(font, buf, NULL, 0);

    unsigned int glyph_count;
    hb_glyph_info_t *glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    unsigned int baseline = 0;
    measure_internal(font, glyph_count, glyph_info, glyph_pos, NULL, NULL, &baseline);

    int cursor_x = x_offset;
    int cursor_y = y_offset + baseline;
    for (unsigned int i = 0; i < glyph_count; i++) {
        hb_codepoint_t glyphid  = glyph_info[i].codepoint;

        hb_glyph_extents_t extents;
        if (!hb_font_get_glyph_extents(font, glyphid, &extents)) {
            // Error?
            continue;
        }

        unsigned int glyph_width = extents.width;
        unsigned int glyph_height = -1 * extents.height;

        int glyph_x_offset = glyph_pos[i].x_offset + extents.x_bearing;
        int glyph_y_offset = glyph_pos[i].y_offset + extents.y_bearing;

        uint8_t* glyph_pixels = (uint8_t*)malloc(glyph_width * glyph_height);
        memset(glyph_pixels, 0, glyph_width * glyph_height);

        draw_glyph(font, glyphid, extents, glyph_pixels);

        for (int y = 0; y < glyph_height; ++y) {
            for (int x = 0; x < glyph_width; ++x) {
                // Todo: support non-monochrome displays
                const int image_x = cursor_x + x + glyph_x_offset;
                const int image_y = cursor_y + y - glyph_y_offset;

                if (image_x < 0 || image_x >= width || image_y < 0 || image_y >= height) {
                    continue;
                }

                const int image_i = (image_y * width) + image_x;
                pixels[image_i] = max(pixels[image_i], glyph_pixels[y * glyph_width + x]);
            }
        }

        free(glyph_pixels);

        cursor_x += glyph_pos[i].x_advance;
        cursor_y += glyph_pos[i].y_advance;
    }

    hb_buffer_destroy(buf);
}
