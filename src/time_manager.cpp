#include "time_manager.h"
#include "settings.h"

#include <string.h>
#include "esp_sntp.h"
#include "esp_log.h"

const char *TAG = "time_manager";
const char *DEFAULT_NTP_SERVER = "pool.ntp.org";

esp_err_t time_manager_init(void) {
    settings_t settings;
    auto err = settings_get_data(&settings);
    if (ESP_OK != err) return err;

    if (settings.flags.wifi_on) {
        if (settings.wifi_ssid[0] == 0 || settings.wifi_key[0] == 0) {
            // Serial.println("No WiFi Credentials Set");
        } else {
            // This wouldn't necessarily be needed if we had the RTC enabled. We could
            // poll time from either Internets or the RTC to keep the ESP's RTC in sync.
            //
            // WiFi.begin(settings.wifi_ssid, settings.wifi_key);
            // while (WiFi.status() != WL_CONNECTED) {
            //     delay(500);
            //     Serial.print(".");
            // }
            // Serial.println("");
            // Serial.println("WiFi Connected.");
        }
    } else {
        // Serial.println("WiFi is OFF.");
    }

    err = time_manager_set_zone(settings.gmt_offset_s, settings.dst_offset_s, settings.flags.is_dst);
    if (ESP_OK != err) return err;

    if (settings.flags.ntp_on) {
        err = time_manager_update_ntp();
        if (ESP_OK != err) return err;
    }

    return ESP_OK;
}

esp_err_t time_manager_update_ntp(void) {
    settings_t settings;
    auto err = settings_get_data(&settings);
    if (ESP_OK != err) return err;

    if(sntp_enabled()){
        sntp_stop();
    }

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, (char*)DEFAULT_NTP_SERVER);
    sntp_setservername(1, (char*)settings.ntp_server);
    sntp_init();
    
    return ESP_OK;
}

esp_err_t time_manager_update_rtc(void) {
    return ESP_FAIL;
}

esp_err_t time_manager_set_time(struct tm time) {
    return ESP_FAIL;
}

esp_err_t time_manager_set_zone(gmt_offset_t offset, dst_offset_t dst_offset_s, bool is_dst) {
    dst_offset_t daylight = is_dst ? dst_offset_s : 0;

    char cst[17] = {0};
    char cdt[17] = "DST";
    char tz[33] = {0};

    if(offset % 3600){
        sprintf(cst, "UTC%ld:%02u:%02u", offset / 3600, abs((offset % 3600) / 60), abs(offset % 60));
    } else {
        sprintf(cst, "UTC%ld", offset / 3600);
    }

    if(daylight != 3600){
        long tz_dst = offset - daylight;
        if(tz_dst % 3600){
            sprintf(cdt, "DST%ld:%02u:%02u", tz_dst / 3600, abs((tz_dst % 3600) / 60), abs(tz_dst % 60));
        } else {
            sprintf(cdt, "DST%ld", tz_dst / 3600);
        }
    }

    sprintf(tz, "%s%s", cst, cdt);
    setenv("TZ", tz, 1);
    tzset();

    return ESP_OK;
}

esp_err_t time_manager_get_local_time(struct tm* timeinfo) {
    time_t now;
    time(&now);
    localtime_r(&now, timeinfo);
    ESP_LOGE(TAG, "local_time: %ld", now);
    return ESP_OK;
}

esp_err_t time_manager_get_eorzea_time(struct tm* timeinfo) {
    time_t now;
    time(&now);
    
    // Calculate Eorzean Time
    time_t then = now * (double)EORZEAN_TIME_CONSTANT;
    localtime_r(&then, timeinfo);
    ESP_LOGE(TAG, "local_time: %ld, eorzean_time: %ld (%d)", now, then, sizeof(then));
    return ESP_OK;
}
