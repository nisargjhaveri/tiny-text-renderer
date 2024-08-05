#ifndef TTR_SCALE_H
#define TTR_SCALE_H 1

#include <hb.h>

#ifdef __cplusplus
extern "C" {
#endif

int ttr_scale_up(int value);

int ttr_round_scaled(int value);
int ttr_floor_scaled(int value);
int ttr_fraction_scaled(int value);

float ttr_scale_down_float(float value);
float ttr_scale_down(int value);
int ttr_scale_down_ceil(int value);
int ttr_scale_down_floor(int value);
int ttr_scale_down_round(int value);

#ifdef __cplusplus
}
#endif

#endif /* TTR_SCALE_H */
