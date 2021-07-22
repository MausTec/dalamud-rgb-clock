#include "rgb_control.h"
#include "driver/rmt.h"
#include "led_strip.h"

static const char *TAG = "rgb_control";
static bool _initialized = false;
static CRGBA leds[NUM_LEDS];

static led_strip_t strip = {
    .type = LED_STRIP_WS2812,
    .length = NUM_LEDS,
    .gpio = LED_DOUT,
    .buf = NULL,
};

void rgb_fill(CRGBA color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        rgb_set_color(i, color);
    }
}

esp_err_t rgb_init() {
    led_strip_install();

    auto err = led_strip_init(&strip);
    if (ESP_OK != err) return err;

    rgb_fill({0, 0, 0, 255});
    rgb_show();
    _initialized = true;
    return ESP_OK;
}

void rgb_show() {
    led_strip_flush(&strip);
}

void rgb_set_color(int n, CRGBA color) {
    rgb_t rgb = { color.red, color.green, color.blue };
    led_strip_set_pixel(&strip, n, rgb);
}

void rgb_set_color_range(int start, int end, CRGBA color) {
    if (end < start) return;

    for (int i = start; i <= end; i++) {
        rgb_set_color(i, color);
    }
}