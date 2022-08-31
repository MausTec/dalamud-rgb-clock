#ifndef __rgb_control_h
#define __rgb_control_h

#include "color.h"
#include "esp_err.h"
#include <stdint.h>

#define NUM_LEDS (6 * 12)
#define BG_FX_MAX_COLORS 12
#define FRAMES_PER_SECOND 30
#define FRAME_RATE_MS (1000 / FRAMES_PER_SECOND)

/**
 * HSV + Alpha values for color blends.
 */
struct hsva {
    hsv_t hsv;
    uint8_t alpha;
};

typedef struct hsva hsva_t;

/**
 * RGB + Alpha values for color blends.
 */
struct rgba {
    rgb_t rgb;
    uint8_t alpha;
};

typedef struct rgba rgba_t;

/**
 * Used to identify a color struct as HSV or RGB
 */
enum color_type {
    COLOR_TYPE_RGB,
    COLOR_TYPE_HSV,
    COLOR_TYPE_NONE,
};

typedef enum color_type color_type_t;

/**
 * Unified color struct.
 */
struct color {
    union {
        rgba_t rgba;
        hsva_t hsva;
    };

    color_type_t type;
};

typedef struct color color_t;

/**
 * A collection of different renderers for applying background effects.
 */
enum rgb_bg_fx_renderer {
    BG_FX_LINEAR_GRADIENT,
    BG_FX_RADIAL_GRADIENT,
    BG_FX_CIRCULAR_GRADIENT,
    BG_FX_RAINDROPS,
    BG_FX_SOLID_COLOR,
    BG_FX_COLOR_FADE,
};

typedef enum rgb_bg_fx_renderer rgb_bg_fx_renderer_t;

/**
 * Definition for a background rendering effect, containing all the
 * parameters the renderer will need to generate animation frames.
 */
struct rgb_bg_effect {
    // Specify the renderer to use
    rgb_bg_fx_renderer_t renderer;

    // Starting rotation of the animation in byte-mapped degrees (0-255)
    uint8_t rotation;

    // Duration of the animation in ms
    uint32_t speed;

    // Delay between repetitions of the animation
    uint32_t delay;

    // Listing of available colors for the animation
    color_t colors[BG_FX_MAX_COLORS];

    // Number of colors to use
    size_t color_count;

    union {
        struct {
            // Mirror the animation
            uint8_t mirror : 1;

            // Loop the animation
            uint8_t loop : 1;

            // Disable rendering gradients between color stops
            uint8_t nofade : 1;
        } flags;

        uint8_t flags_raw;
    };
};

typedef struct rgb_bg_effect rgb_bg_effect_t;

/// Useful Color Constants

#define HSVA(h, s, v, a) ((color_t){ h, s, v, a, COLOR_TYPE_HSV })
#define RGBA(r, g, b, a) ((color_t){ r, g, b, a, COLOR_TYPE_RGB })
#define HSV(h, s, v) HSVA(h, s, v, 255)
#define RGB(r, g, b) RGBA(r, g, b, 255)

#define COLOR_TRANSPARENT RGBA(0, 0, 0, 0)
#define COLOR_WHITE RGBA(255, 255, 255, 255)
#define COLOR_BLACK RGBA(0, 0, 0, 255)
#define COLOR_NONE ((color_t){ 0, 0, 0, 0, COLOR_TYPE_NONE })

/// Okay that's it.

esp_err_t rgb_init(void);
void rgb_show(void);

void rgb_set_brightness(uint8_t brightness);

void rgb_fill(color_t color);
void rgb_set_color(int n, color_t color);
void rgb_set_color_range(int start, int end, color_t color);

// Background Effects

/**
 * Render the current frame of a background effect. This will overwrite the
 * buffer, but not immediately display.
 *
 * @param effect Pointer to the background effect struct.
 * @param frame Range 0-1800, 30 frames / sec within 1 minute.
 */
void rgb_render_background_effect(rgb_bg_effect_t* effect, uint32_t millis);

/**
 * Convert a generic color type to RGBA, if it is not, or return the RGB
 * component if it was.
 */
rgba_t rgba(color_t color);

/**
 * Apply color B on A, return opaque RGB. Useful when populating the LED buffer.
 */
rgb_t rgb(rgb_t a, color_t b);

/**
 * Do an opaque blend between two color types, color A is made opaque.
 */
color_t color_blend(color_t a, color_t b);

#endif