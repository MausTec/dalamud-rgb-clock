#include "rgb_control.h"
#include "driver/rmt.h"

static const char *TAG = "rgb_control";
static bool _initialized = false;
static CRGBA leds[NUM_LEDS];

void rgb_fill(CRGBA color) {
    for (int i = 0; i < NUM_LEDS; i++) {
        rgb_set_color(i, color);
    }
}

esp_err_t rgb_init() {
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_DOUT, RMT_CHANNEL_0);
    config.clk_div = 2;

    auto err = rmt_config(&config);
    if (ESP_OK != err) return err;

    err = rmt_driver_install(config.channel, 0, 0);
    if (ESP_OK != err) return err;

    rgb_fill({0, 0, 0, 255});
    rgb_show();
    _initialized = true;
    return ESP_OK;
}

void rgb_show() {
    
}

void rgb_set_color(int n, CRGBA color) {
    leds[n] = color;
}

void rgb_set_color_range(int start, int end, CRGBA color) {
    if (end < start) return;

    for (int i = start; i <= end; i++) {
        rgb_set_color(i, color);
    }
}