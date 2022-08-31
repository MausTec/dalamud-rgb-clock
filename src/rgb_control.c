#include "rgb_control.h"
#include "clock_led.h"
#include "driver/rmt.h"
#include "esp_log.h"
#include "led_strip.h"
#include "pinout.h"

static const char* TAG = "rgb_control";
static bool _initialized = false;
static uint8_t _brightness = 255;

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
    if (ESP_OK != err)
        return err;

    for (int i = 0; i < NUM_LEDS; i++)
        leds[i] = (rgb_t){ 0, 0, 0 };

    rgb_show();
    _initialized = true;
    return ESP_OK;
}

void rgb_show() {
    for (int i = 0; i < NUM_LEDS; i++) {
        float coeff = (float)_brightness / 255;

        rgb_t color = leds[i];
        color.r = (float)color.r * coeff;
        color.g = (float)color.g * coeff;
        color.b = (float)color.b * coeff;

        led_strip_set_pixel(&strip, i, color);
    }

    led_strip_flush(&strip);
}

void rgb_set_color(int n, color_t color) {
    if (color.type != COLOR_TYPE_NONE)
        leds[n] = rgb(leds[n], color);
}

void rgb_set_color_range(int start, int end, color_t color) {
    if (end < start)
        return;
    for (int i = start; i <= end; i++) {
        rgb_set_color(i, color);
    }
}

void rgb_set_brightness(uint8_t brightness) {
    _brightness = brightness;
    rgb_show();
}

rgba_t rgba(color_t color) {
    if (color.type == COLOR_TYPE_RGB)
        return color.rgba;
    else
        return (rgba_t){ hsv2rgb_rainbow(color.hsva.hsv), color.hsva.alpha };
}

rgb_t rgb(rgb_t a, color_t b) {
    rgba_t b_rgb = rgba(b);
    return rgb_blend(a, b_rgb.rgb, b_rgb.alpha);
}

color_t color_blend(color_t a, color_t b) {
    rgba_t a_rgb = rgba(a);

    color_t ret = {
      .rgba =
          {
              .rgb = rgb(a_rgb.rgb, b),
              .alpha = 255,
          },
      .type = COLOR_TYPE_RGB,
  };

    return ret;
}

void rgb_render_background_effect(rgb_bg_effect_t* effect, uint32_t millis) {
    // how many ms into the total effect duration we are:
    uint32_t effect_speed = effect->speed + effect->delay;
    uint32_t total_fx_speed = effect_speed * effect->color_count;
    uint32_t color_ms = effect_speed > 0 ? millis % effect_speed : 0;
    uint32_t effect_ms = total_fx_speed > 0 ? millis % total_fx_speed : 0;

    switch (effect->renderer) {
    case BG_FX_COLOR_FADE: {
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

    case BG_FX_CIRCULAR_GRADIENT: {
        /**
         * there are 24 possible positions to spread out n colors
         * thus every color gets 24/n positions, in the case of a
         * rainbow that'd be 24/8 or 3 clock positions per color,
         *
         * within each of those we have a step of 255 / (w - 1)
         * blend which can be calculated by subhour % gradient_width;
         *
         * rotation needs to increment by 24 per total fx step,
         * so we know totalfx / 24 = ms per step, then step =
         *
         * okay so, for fading. We've got each rotation taking n ms,
         * so we'll be x = fx_ms % n, and x is the percent we need to
         * advance the fade within the two colors, it's actually possible
         * that we'll go PAST the next color here, so we need to add
         * x / 255 to the starting color index
         */

        uint8_t gradient_width = 24 / effect->color_count;
        uint32_t blend_step = gradient_width > 1 ? 25500 / (gradient_width - 1) : 25500;
        uint8_t rotation = effect->rotation;

        // Calculate Animation
        uint32_t rotation_step_ms = effect_speed / 24;
        double rotation_steps = rotation_step_ms > 0 ? (double)effect_ms / rotation_step_ms : 0;

        // Calculate intermediary fade increment:
        uint32_t fade_step_us = (rotation_step_ms * 1000) / 255;
        uint16_t fade_steps = floor(255.0 * fmod(rotation_steps, 1));

        rotation = fmod(rotation + rotation_steps, 24);

        if (true) { // invert rotation
                    //   rotation = 24 - rotation;
                    //   fade_steps = 255 - fade_steps;
        }

        for (uint8_t subhour = 0; subhour < 24; subhour++) {
            uint8_t hour = ((subhour + 1) / 2) % 12;

            uint8_t rotated_sh = (subhour + rotation) % 24;
            uint16_t alpha =
                gradient_width > 0 ? (blend_step * (rotated_sh % gradient_width)) / 100 : 0;
            alpha += gradient_width > 0 ? fade_steps / gradient_width : 0;

            // if (subhour == 0)
            //     printf("rotation_fade_ms: %d, fade_step_us: %ld, alpha: %d, "
            //            "fade_steps: %d\n",
            //            rotation_fade_ms, fade_step_us, alpha, fade_steps);

            uint8_t start_color_idx =
                (uint8_t)((gradient_width > 0 ? floor((float)rotated_sh / gradient_width) : 0) +
                          (alpha / 255)) %
                effect->color_count;

            uint8_t next_color_idx = (start_color_idx + 1) % effect->color_count;

            // printf("h: %d, sh: %d, rot: %d, steps: %f, start_color_idx: %d, alpha: %d\n", hour,
            //        subhour, rotation, rotation_steps, start_color_idx, alpha);

            color_t a_color = effect->colors[start_color_idx];
            color_t b_color = effect->colors[next_color_idx];

            if (effect->flags.nofade) {
                b_color.rgba.alpha = alpha > 128 ? 255 : 0;
            } else {
                b_color.rgba.alpha = alpha % 255;
            }

            color_t set = color_blend(a_color, b_color);

            if (subhour % 2) {
                clock_led_set_lr_color(hour, COLOR_NONE, set);
            } else {
                clock_led_set_lr_color(hour, set, COLOR_NONE);
            }
        }
        break;
    }

    default: {
        if (effect->color_count > 0)
            rgb_fill(effect->colors[0]);
    }
    }
}