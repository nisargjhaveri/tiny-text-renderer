#include <stdint.h>
#include <math.h>

static const uint8_t hb_scale_shift = 6;
static const uint16_t hb_scale_factor = 1 << hb_scale_shift;
static const float hb_scale_factor_divider = hb_scale_factor;

int ttr_scale_up(int value) {
    return value << hb_scale_shift;
}

int ttr_round_scaled(int value) {
    return (value + (hb_scale_factor >> 1)) & ~(hb_scale_factor - 1);
}

int ttr_floor_scaled(int value) {
    return value & ~(hb_scale_factor - 1);
}

int ttr_fraction_scaled(int value) {
    return value & (hb_scale_factor - 1);
}

float ttr_scale_down_float(float value) {
    return value / hb_scale_factor_divider;
}

float ttr_scale_down(int value) {
    return (float)value / hb_scale_factor_divider;
}

int ttr_scale_down_ceil(int value) {
    return (value + (hb_scale_factor - 1)) >> hb_scale_shift;
}

int ttr_scale_down_floor(int value) {
    return value >> hb_scale_shift;
}

int ttr_scale_down_round(int value) {
    return (value + (hb_scale_factor >> 1)) >> hb_scale_shift;
}
