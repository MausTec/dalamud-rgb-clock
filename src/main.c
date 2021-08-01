#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "rgb_control.h"
#include "console.h"
#include "settings.h"
#include "clock_led.h"
#include "time_manager.h"
#include "light_meter.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "main";

static TaskHandle_t ClockMainTask;
void clock_main_task(void* param);

static TaskHandle_t AmbientLightTask;
void ambient_light_task(void* param);

static rgb_bg_effect_t bgfx = {
    .renderer = BG_FX_COLOR_FADE,
    .color_count = 8,
    .colors = {
        HSVA(32 * 0, 255, 200, 255),
        HSVA(32 * 1, 255, 200, 255),
        HSVA(32 * 2, 255, 200, 255),
        HSVA(32 * 3, 255, 200, 255),
        HSVA(32 * 4, 255, 200, 255),
        HSVA(32 * 5, 255, 200, 255),
        HSVA(32 * 6, 255, 200, 255),
        HSVA(32 * 7, 255, 200, 255),
    },
    .speed = 5000,
    .delay = 0000,
    .flags = {
        .loop = true
    },
};

void app_main(void) {
    ESP_ERROR_CHECK(console_init());
    ESP_ERROR_CHECK(settings_init());
    ESP_ERROR_CHECK(time_manager_init());
    ESP_ERROR_CHECK(light_meter_init());

    xTaskCreatePinnedToCore(
        clock_main_task,
        "ClockMainTask",
        4095,  // stack size
        NULL,  // param
        0,     // priority 0
        &ClockMainTask,
        1      // core 1
    );

    xTaskCreatePinnedToCore(
        ambient_light_task,
        "AmbientLightTask",
        4095,
        NULL,
        0,
        &AmbientLightTask,
        0
    );

    while (true) {
        console_tick();
        vTaskDelay(1);
    }
}

void clock_main_task(void* param) {
    ESP_ERROR_CHECK(rgb_init());

    static long _local = true;

    struct tm timeinfo;
    struct tm eorzea;
    uint32_t millis = 0;

    while (true) {
        millis = esp_timer_get_time() / 1000L;

        if (_local) {
            time_manager_get_local_time(&timeinfo);
        } else {
            time_manager_get_eorzea_time(&timeinfo);
        }

        rgb_render_background_effect(&bgfx, millis);
        clock_set_time(timeinfo.tm_hour % 12, timeinfo.tm_min, timeinfo.tm_sec);
        rgb_show();

        // Delay for 1 animation frame (1/30th of a Sec)
        vTaskDelay((1000 / 30) / portTICK_PERIOD_MS);
    }
}

void ambient_light_task(void* param) {
    while (true) {
        // Debug Light Meter
        long lumens = 0;
        light_meter_get_reading(&lumens, 8);
        printf("%ld\n", lumens);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}