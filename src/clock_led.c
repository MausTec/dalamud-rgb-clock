#include "clock_led.h"
#include "rgb_control.h"
#include "esp_log.h"
#include <math.h>

static const char* TAG = "clock_led";

#define CLOCK_COLOR_HOUR_HSV    HSVA(0, 0, 255, 32)
#define CLOCK_COLOR_MINUTE_HSV  HSVA(0, 0, 255, 20)
#define CLOCK_COLOR_SECOND_HSV  HSVA(0, 0, 255, 16)

static uint8_t _brightness = 128;
static bool _show_seconds = true;

static clock_effect_t _effect = {
    CLOCK_COLOR_HOUR_HSV, CLOCK_COLOR_HOUR_HSV, CLOCK_COLOR_HOUR_HSV,
    CLOCK_COLOR_MINUTE_HSV, COLOR_TRANSPARENT, COLOR_TRANSPARENT,
    COLOR_TRANSPARENT, COLOR_TRANSPARENT, CLOCK_COLOR_SECOND_HSV
};

void clock_led_set_split_color(int hour, color_t inner_left_color, color_t inner_right_color, color_t mid_left_color, color_t mid_right_color, color_t outer_left_color, color_t outer_right_color) {
    int offset = 12 - ((hour + 7) % 12);
    int start = (offset % 12) * 6;

    ESP_LOGD(TAG, "hour: %d, offset:%d, start:%d", hour, offset, start);

    rgb_set_color(start + 0, outer_left_color);
    rgb_set_color(start + 1, mid_left_color);
    rgb_set_color(start + 2, inner_left_color);
    rgb_set_color(start + 3, inner_right_color);
    rgb_set_color(start + 4, mid_right_color);
    rgb_set_color(start + 5, outer_right_color);
}

void clock_led_set_io_color(int hour, color_t inner_color, color_t outer_color) {
    clock_led_set_split_color(hour, inner_color, inner_color, outer_color, outer_color, outer_color, outer_color);
}

void clock_led_set_color(int hour, color_t color) {
    clock_led_set_split_color(hour, color, color, color, color, color, color);
}

void clock_led_set_lr_color(int hour, color_t left_color, color_t right_color) {
    clock_led_set_split_color(hour, left_color, right_color, left_color, right_color, left_color, right_color);
}

void clock_set_time(int hour, int min, int sec) {
    ESP_LOGD(TAG, "%02d:%02d:%02d", hour, min, sec);

    if (_show_seconds) {
        uint8_t left_sec, right_sec;
        right_sec = floor((double) sec / 5);
        if (sec % 5 > 2) {
            left_sec = ceil((double) sec / 5);
        } else {
            left_sec = right_sec;
        }

        clock_led_set_split_color(
            right_sec,
            _effect.second_inner,
            COLOR_TRANSPARENT,
            _effect.second_mid,
            COLOR_TRANSPARENT,
            _effect.second_outer,
            COLOR_TRANSPARENT
        );

        clock_led_set_split_color(
            left_sec,
            COLOR_TRANSPARENT,
            _effect.second_inner,
            COLOR_TRANSPARENT,
            _effect.second_mid,
            COLOR_TRANSPARENT,
            _effect.second_outer
        );
    }

    uint8_t left_min, right_min;
    right_min = floor((double) min / 5);
    if (min % 5 > 2) {
        left_min = ceil((double) min / 5);
    } else {
        left_min = right_min;
    }

    // Split Minutes
    clock_led_set_split_color(
        right_min,
        _effect.minute_inner,
        COLOR_TRANSPARENT,
        _effect.minute_mid,
        COLOR_TRANSPARENT,
        _effect.minute_outer,
        COLOR_TRANSPARENT
    );

    clock_led_set_split_color(
        left_min,
        COLOR_TRANSPARENT,
        _effect.minute_inner,
        COLOR_TRANSPARENT,
        _effect.minute_mid,
        COLOR_TRANSPARENT,
        _effect.minute_outer
    );

    // Hour
    clock_led_set_split_color(
        hour,
        _effect.hour_inner,
        _effect.hour_inner,
        _effect.hour_mid,
        _effect.hour_mid,
        _effect.hour_outer,
        _effect.hour_outer
    );
}