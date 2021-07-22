#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "time.h"
#include "rgb_control.h"
#include "console.h"
#include "settings.h"
#include "clock_led.h"
#include "time_manager.h"
#include "esp_timer.h"

static const char* TAG = "main";

extern "C" void app_main(void) {
  // Serial.begin(115200);

  ESP_ERROR_CHECK(console_init());
  ESP_ERROR_CHECK(rgb_init());
  ESP_ERROR_CHECK(settings_init());
  ESP_ERROR_CHECK(time_manager_init());

  static long _clk_update_ms = 0;
  static long _local = false;

  while (true) {
    console_tick();
    long millis = esp_timer_get_time() / 1000L;

    if (millis - _clk_update_ms > 1000) {
      _clk_update_ms = millis;

      struct tm timeinfo;
      struct tm eorzea;

      if (_local) {
        time_manager_get_local_time(&timeinfo);
      } else {
        time_manager_get_eorzea_time(&timeinfo);
      }

      clock_set_time(timeinfo.tm_hour % 12, timeinfo.tm_min, timeinfo.tm_sec);
      rgb_show();
    }

    vTaskDelay(1);
  }
}