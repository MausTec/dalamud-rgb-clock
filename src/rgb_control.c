#include "rgb_control.h"
#include "driver/rmt.h"
#include "led_strip.h"
#include "pinout.h"
#include "esp_log.h"

static const char* TAG = "rgb_control";
static bool _initialized = false;

static led_strip_t strip = {
    .type = LED_STRIP_WS2812,
    .length = NUM_LEDS,
    .gpio = LED_DOUT_GPIO,
    .buf = NULL,
};

static rgb_t leds[NUM_LEDS] = { 0 };

void rgb_fill(color_t color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        rgb_set_color(i, color);
    }
}

esp_err_t rgb_init() {
    led_strip_install();

    auto err = led_strip_init(&strip);
    if (ESP_OK != err) return err;

    for (int i = 0; i < NUM_LEDS; i++)
        leds[i] = (rgb_t){ 0, 0, 0 };

    rgb_show();
    _initialized = true;
    return ESP_OK;
}

void rgb_show() {
    for (int i = 0; i < NUM_LEDS; i++)
        led_strip_set_pixel(&strip, i, leds[i]);
    led_strip_flush(&strip);
}

void rgb_set_color(int n, color_t color) {
    leds[n] = rgb(leds[n], color);
}

void rgb_set_color_range(int start, int end, color_t color) {
    if (end < start) return;
    for (int i = start; i <= end; i++) {
        rgb_set_color(i, color);
    }
}

rgba_t rgba(color_t color) {
    if (color.type == COLOR_TYPE_RGB)
        return color.rgba;
    else
        return (rgba_t) { hsv2rgb_rainbow(color.hsva.hsv), color.hsva.alpha };
}

rgb_t rgb(rgb_t a, color_t b) {
    rgba_t b_rgb = rgba(b);
    return rgb_blend(a, b_rgb.rgb, b_rgb.alpha);
}

color_t color_blend(color_t a, color_t b) {
    rgba_t a_rgb = rgba(a);

    color_t ret = {
        .rgba = {
            .rgb = rgb(a_rgb.rgb, b),
            .alpha = 255,
        },
        .type = COLOR_TYPE_RGB,
    };

    return ret;
}

void rgb_render_background_effect(rgb_bg_effect_t* effect, uint32_t millis) {

    switch (effect->renderer) {
    case BG_FX_COLOR_FADE: {
        // how many ms into the total effect duration we are:
        uint32_t effect_speed = effect->speed + effect->delay;
        uint32_t total_fx_speed = effect_speed * effect->color_count;
        uint32_t color_ms = millis % effect_speed;
        uint32_t effect_ms = millis % total_fx_speed;

        uint32_t effect_perc = (color_ms * 100 * 255) / effect->speed;
        uint8_t fade_perc = color_ms > effect->speed ? 255 : (effect_perc / 100);
        uint8_t start_color_idx = effect_ms / effect_speed;
        uint8_t next_color_idx = (start_color_idx + 1) % effect->color_count;

        color_t a_color = effect->colors[start_color_idx];
        color_t b_color = effect->colors[next_color_idx];
        b_color.rgba.alpha = fade_perc;

        color_t fill = color_blend(a_color, b_color);
        rgb_fill(fill);
        break;
    }

    default: {
        if (effect->color_count > 0)
            rgb_fill(effect->colors[0]);
    }
    }
}