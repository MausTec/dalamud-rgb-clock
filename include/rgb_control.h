#ifndef __rgb_control_h
#define __rgb_control_h

#include <stdint.h>
#include "esp_err.h"

struct CRGBA {
    CRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) 
        : red(r), green(g), blue(b), alpha(a) {};

    CRGBA() : red(0), green(0), blue(0), alpha(255) {};
    
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

#define NUM_LEDS (6 * 12)
#define LED_DOUT GPIO_NUM_2

esp_err_t rgb_init(void);
void rgb_fill(CRGBA color);
void rgb_show(void);
void rgb_set_color(int n, CRGBA color);
void rgb_set_color_range(int start, int end, CRGBA color);

#endif