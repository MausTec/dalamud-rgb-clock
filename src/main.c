#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "clock_led.h"
#include "console.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "light_meter.h"
#include "rgb_control.h"
#include "settings.h"
#include "time_manager.h"

static const char* TAG = "main";

static TaskHandle_t ClockMainTask;
void clock_main_task(void* param);

static TaskHandle_t AmbientLightTask;
void ambient_light_task(void* param);

rgb_bg_effect_t bgfx = {
    .renderer = BG_FX_CIRCULAR_GRADIENT,
    .color_count = 1,
    .colors =
        {
            // RGB(255, 255, 255)
            HSVA(175, 255, 200, 255), HSVA(210, 255, 200, 255),
            // RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(255, 255, 255)
            //  RGB(0, 0, 255), RGB(255, 255, 0)
        },
    .speed = 0,
    .delay = 0,
    .flags = {
        .loop = true,
        .nofade = false,
    },
};

void app_main(void) {
    ESP_ERROR_CHECK(console_init());
    ESP_ERROR_CHECK(settings_init());
    ESP_ERROR_CHECK(time_manager_init());
    ESP_ERROR_CHECK(light_meter_init());

    xTaskCreatePinnedToCore(clock_main_task, "ClockMainTask",
                            4095, // stack size
                            NULL, // param
                            0,    // priority 0
                            &ClockMainTask,
                            1 // core 1
    );

    for (;;) {
        console_tick();
        vTaskDelay(1);
    }
}

void clock_main_task(void* param) {
    ESP_ERROR_CHECK(rgb_init());

    static bool _local = true;
    static bool _auto_dim = false;

    struct tm timeinfo;
    struct tm eorzea;
    uint32_t millis = 0;
    uint32_t last_light_sample = 0;

    for (;;) {
        millis = esp_timer_get_time() / 1000L;

        if (_local) {
            time_manager_get_local_time(&timeinfo);
        } else {
            time_manager_get_eorzea_time(&timeinfo);
        }

        if (_auto_dim && millis - last_light_sample > 10000) {
            clock_led_set_color(5, RGBA(0, 0, 0, 255));
            clock_led_set_color(6, RGBA(0, 0, 0, 255));
            rgb_show();
            vTaskDelay(100);
            long lumens = 0;
            light_meter_get_reading(&lumens, 8);
            printf("%ld\n", lumens);
            rgb_set_brightness((lumens / 16));
            last_light_sample = millis;
        }

        rgb_render_background_effect(&bgfx, millis);
        clock_set_time(timeinfo.tm_hour % 12, timeinfo.tm_min, timeinfo.tm_sec);
        rgb_show();

        // Delay for 1 animation frame (1/30th of a Sec)
        vTaskDelay(FRAME_RATE_MS / portTICK_PERIOD_MS);
    }
}