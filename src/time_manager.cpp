#include "time_manager.h"
#include "settings.h"

#include <string.h>
#include "esp_sntp.h"
#include "esp_log.h"

extern "C" {
    #include <time64.h>
}

#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "math.h"

const char *TAG = "time_manager";
const char *DEFAULT_NTP_SERVER = "pool.ntp.org";

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            auto err = esp_wifi_connect();
            s_retry_num++;
            wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t*) event_data;
            if (event) {
                ESP_LOGI(TAG, "Disconnect reason: %d, AP: \"%s\" (%d)", event->reason, event->ssid, event->ssid_len);
            }
            ESP_LOGI(TAG, "WiFi Disconnected, reconnect attempt: %s", esp_err_to_name(err));
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGI(TAG, "WiFi doesn't want to work right now.");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t time_manager_init(void) {
    settings_t settings;
    auto err = settings_get_data(&settings);
    if (ESP_OK != err) return err;

    if (settings.flags.wifi_on) {
        if (settings.wifi_ssid[0] == 0 || settings.wifi_key[0] == 0) {
            ESP_LOGE(TAG, "No WiFi Credentials Set");
        } else {
            s_wifi_event_group = xEventGroupCreate();

            err = esp_netif_init();
            if (ESP_OK != err) return err;

            err = esp_event_loop_create_default();
            if (ESP_OK != err) return err;

            esp_netif_create_default_wifi_sta();
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

            err = esp_wifi_init(&cfg);
            if (ESP_OK != err) return err;

            esp_event_handler_instance_t instance_any_id;
            esp_event_handler_instance_t instance_got_ip;

            err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id);
            if (ESP_OK != err) return err;

            err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip);

            wifi_config_t wifi_config = {0};
            strncpy((char*)wifi_config.sta.ssid, settings.wifi_ssid, WIFI_SSID_MAX_LEN);
            strncpy((char*)wifi_config.sta.password, settings.wifi_key, WIFI_PSK_MAX_LEN);
            //wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            wifi_config.sta.pmf_cfg.capable = true;
            wifi_config.sta.pmf_cfg.required = false;

            err = esp_wifi_set_mode(WIFI_MODE_STA);
            if (ESP_OK != err) return err;

            err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
            if (ESP_OK != err) return err;

            err = esp_wifi_start();
            if (ESP_OK != err) return err;

            // now we wait
            EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
            if (bits & WIFI_CONNECTED_BIT) {
                ESP_LOGI(TAG, "WiFi Connected.");
            } else if (bits & WIFI_FAIL_BIT) {
                ESP_LOGI(TAG, "Failed to connect to %s (%s).", wifi_config.sta.ssid, wifi_config.sta.password);
            } else {
                ESP_LOGI(TAG, "なに？！");
            }

            err = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
            if (ESP_OK != err) return err;

            err = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
            if (ESP_OK != err) return err;

            vEventGroupDelete(s_wifi_event_group);

            time_manager_update_ntp();
        }
    } else {
        ESP_LOGI(TAG, "WiFi is OFF.");
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
    
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Calculate Eorzean Time
    double then = (double)now * (double)EORZEAN_TIME_CONSTANT;
    then += ((double)tv.tv_usec * (double)EORZEAN_TIME_CONSTANT) / 1000000L;

    // For some reason, Eorzea time is 7 minutes behind, which probably has to do with our
    // bad time64 implementation. Here, let's try fixing that:
    then += 7 * 60;

    Time64_T then_time = floor(then);
    gmtime64_r(&then_time, timeinfo);
    
    ESP_LOGE(TAG, "local_time: %ld, eorzean_time: %llf (%lld)", now, then, then_time);
    return ESP_OK;
}
