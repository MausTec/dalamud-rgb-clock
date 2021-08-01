#ifndef __clock_led_h
#define __clock_led_h

#include "rgb_control.h"
#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

struct clock_effect {
    color_t hour_inner;
    color_t hour_mid;
    color_t hour_outer;

    color_t minute_inner;
    color_t minute_mid;
    color_t minute_outer;

    color_t second_inner;
    color_t second_mid;
    color_t second_outer;
};

typedef struct clock_effect clock_effect_t;

#define SIMPLE_CLOCK_EFFECT(hour, minute, second) { \
    hour, hour, hour, \
    minute, minute, minute, \
    second, second, second, \
}

void clock_led_set_color(int hour, color_t color);
void clock_set_time(int hour, int minute, int second);
void clock_led_set_split_color(int hour, color_t inner_left_color, color_t inner_right_color, color_t mid_left_color, color_t mid_right_color, color_t outer_left_color, color_t outer_right_color);
void clock_led_set_io_color(int hour, color_t inner_color, color_t outer_color);
void clock_led_set_lr_color(int hour, color_t left_color, color_t right_color);

#ifdef __cplusplus
};
#endif

#endif