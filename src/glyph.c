#include "glyph.h"
#include "scale.h"
#include "schrift.h"

#include <stddef.h>

static void sft_move_to(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    float to_x,
    float to_y,
    void *user_data
) {
    SFT_Outline* outline = (SFT_Outline*)draw_data;

    sft_add_point(outline, ttr_scale_down_float(to_x), ttr_scale_down_float(to_y));
}

static void sft_line_to(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    float to_x,
    float to_y,
    void *user_data
) {
    SFT_Outline* outline = (SFT_Outline*)draw_data;

    sft_add_point(outline, ttr_scale_down_float(to_x), ttr_scale_down_float(to_y));
    sft_add_line(outline, outline->numPoints - 2, outline->numPoints - 1);
}

static void sft_quadratic_to(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    float control_x,
    float control_y,
    float to_x,
    float to_y,
    void *user_data
) {
    SFT_Outline* outline = (SFT_Outline*)draw_data;

    sft_add_point(outline, ttr_scale_down_float(control_x), ttr_scale_down_float(control_y));
    sft_add_point(outline, ttr_scale_down_float(to_x), ttr_scale_down_float(to_y));
    sft_add_curve(outline, outline->numPoints - 3, outline->numPoints - 2, outline->numPoints - 1);
}

static void sft_cubic_to(
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

static void sft_close_path(
    hb_draw_funcs_t *dfuncs,
    void *draw_data,
    hb_draw_state_t *st,
    void *user_data
) {
    // Nothing to do.
}

static hb_draw_funcs_t* ttr_create_draw_funcs() {
    hb_draw_funcs_t *funcs = hb_draw_funcs_create();

    hb_draw_funcs_set_move_to_func(funcs, sft_move_to, NULL, NULL);
    hb_draw_funcs_set_line_to_func(funcs, sft_line_to, NULL, NULL);
    hb_draw_funcs_set_quadratic_to_func(funcs, sft_quadratic_to, NULL, NULL);
    hb_draw_funcs_set_cubic_to_func(funcs, sft_cubic_to, NULL, NULL);
    hb_draw_funcs_set_close_path_func(funcs, sft_close_path, NULL, NULL);

    return funcs;
}

int ttr_draw_glyph(
    hb_font_t* font,
    hb_codepoint_t glyph,
    hb_glyph_extents_t extents,
    unsigned int offset_x,
    unsigned int offset_y,
    void (*draw_pixel_at)(unsigned int x, unsigned int y, uint8_t mask, void* user_data),
    void* user_data
) {
    if (extents.width == 0 || extents.height == 0) {
        // Nothing to be done
        return 0;
    }

    hb_draw_funcs_t *funcs = ttr_create_draw_funcs();

    SFT_Outline outline;
    sft_init_outline(&outline);

    hb_font_draw_glyph(font, glyph, funcs , &outline);

    SFT_Image image = {
        .width = ttr_scale_down_ceil(extents.width),
        .height = ttr_scale_down_ceil(-1 * extents.height),

        .draw_pixel_at = draw_pixel_at,
        .user_data = user_data
    };
    float transform[6] = {1, 0, 0, -1, ttr_scale_down(offset_x - extents.x_bearing), ttr_scale_down(offset_y + extents.y_bearing)};
    sft_render_outline(&outline, transform, image);

    sft_free_outline(&outline);
    hb_draw_funcs_destroy(funcs);
    return 0;
}
