#ifndef __clock_led_h
#define __clock_led_h

#include "rgb_control.h"

static const CRGBA CLOCK_COLOR_HOUR       = CRGBA(32, 0, 0, 255);
static const CRGBA CLOCK_COLOR_MINUTE     = CRGBA(32, 4, 0, 255);
static const CRGBA CLOCK_COLOR_BACKGROUND = CRGBA(16, 4, 0x00, 255);
static const CRGBA CLOCK_COLOR_SECOND     = CRGBA(16, 4, 4, 255);
static const CRGBA CLOCK_COLOR_SECOND_DIM = CRGBA(16, 4, 4, 255);

void clock_led_set_color(int hour, CRGBA color);
void clock_set_time(int hour, int minute, int second);
void clock_led_set_split_color(int hour, CRGBA inner_left_color, CRGBA inner_right_color, CRGBA outer_left_color, CRGBA outer_right_color);
void clock_led_set_io_color(int hour, CRGBA inner_color, CRGBA outer_color);
void clock_led_set_lr_color(int hour, CRGBA left_color, CRGBA right_color);

#endif