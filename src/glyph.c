#include "glyph.h"
#include "schrift.h"

#include <stddef.h>

void sft_move_to(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    float to_x,
    float to_y,
    void *user_data
) {
    Outline* outline = (Outline*)draw_data;

    add_point(outline, to_x, to_y);
}

void sft_line_to(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    float to_x,
    float to_y,
    void *user_data
) {
    Outline* outline = (Outline*)draw_data;

    add_point(outline, to_x, to_y);
    add_line(outline, outline->numPoints - 2, outline->numPoints - 1);
}

void sft_quadratic_to(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    float control_x,
    float control_y,
    float to_x,
    float to_y,
    void *user_data
) {
    Outline* outline = (Outline*)draw_data;

    add_point(outline, control_x, control_y);
    add_point(outline, to_x, to_y);
    add_curve(outline, outline->numPoints - 3, outline->numPoints - 2, outline->numPoints - 1);
}

void sft_cubic_to(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    float control1_x,
    float control1_y,
    float control2_x,
    float control2_y,
    float to_x,
    float to_y,
    void *user_data
) {
    // Not implemented
}

void sft_close_path(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    void *user_data
) {
    // Nothing to do.
}

hb_draw_funcs_t* create_draw_funcs() {
    hb_draw_funcs_t *funcs = hb_draw_funcs_create();

    hb_draw_funcs_set_move_to_func(funcs, sft_move_to, NULL, NULL);
    hb_draw_funcs_set_line_to_func(funcs, sft_line_to, NULL, NULL);
    hb_draw_funcs_set_quadratic_to_func(funcs, sft_quadratic_to, NULL, NULL);
    hb_draw_funcs_set_cubic_to_func(funcs, sft_cubic_to, NULL, NULL);
    hb_draw_funcs_set_close_path_func(funcs, sft_close_path, NULL, NULL);

    return funcs;
}

int draw_glyph(hb_font_t* font, hb_codepoint_t glyph, hb_glyph_extents_t extents, uint8_t* pixels) {
    if (extents.width == 0 || extents.height == 0) {
        // Nothing to be done
        return 0;
    }

    hb_draw_funcs_t *funcs = create_draw_funcs();

    Outline outline;
    init_outline(&outline);

    hb_font_draw_glyph(font, glyph, funcs , &outline);

    SFT_Image image = {
        .pixels = pixels,
        .height = -1 * extents.height,
        .width = extents.width
    };
    float transform[6] = {1, 0, 0, -1, -extents.x_bearing, extents.y_bearing};
    render_outline(&outline, transform, image);

    free_outline(&outline);
    hb_draw_funcs_destroy(funcs);
    return 0;
}
