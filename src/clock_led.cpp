#include "clock_led.h"
#include "rgb_control.h"
#include "esp_log.h"

static const char *TAG = "clock_led";

void clock_led_set_split_color(int hour, CRGBA inner_left_color, CRGBA inner_right_color, CRGBA outer_left_color, CRGBA outer_right_color) {
    int offset = 12 - ((hour + 7) % 12);
    int start = offset * 6;
    rgb_set_color_range(start, start + 1, outer_left_color);
    rgb_set_color(start + 2, inner_left_color);
    rgb_set_color(start + 3, inner_right_color);
    rgb_set_color_range(start + 4, start + 5, outer_right_color);
}

void clock_led_set_io_color(int hour, CRGBA inner_color, CRGBA outer_color) {
    clock_led_set_split_color(hour, inner_color, inner_color, outer_color, outer_color);
}

void clock_led_set_color(int hour, CRGBA color) {
    clock_led_set_split_color(hour, color, color, color, color);
}

void clock_led_set_lr_color(int hour, CRGBA left_color, CRGBA right_color) {
    clock_led_set_split_color(hour, left_color, right_color, left_color, right_color);
}

void clock_set_time(int hour, int min, int sec) {
    ESP_LOGE(TAG, "%02d:%02d:%02d", hour, min, sec);
    rgb_fill(CLOCK_COLOR_BACKGROUND);
    clock_led_set_io_color(sec / 5, CLOCK_COLOR_BACKGROUND, sec % 2 == 0 ? CLOCK_COLOR_SECOND : CLOCK_COLOR_SECOND_DIM);
    clock_led_set_io_color(min / 5, CLOCK_COLOR_BACKGROUND, CLOCK_COLOR_MINUTE);
    clock_led_set_color(hour,    CLOCK_COLOR_HOUR);
}