#include "settings.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "settings";
static bool _initialized = false;

static settings_t _current_settings = DEFAULT_SETTINGS;

bool _is_nvs_err(esp_err_t err, nvs_handle nvs) {
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        nvs_close(nvs);
        return true;
    } else {
        return false;
    }
}

esp_err_t settings_init(void) {
    esp_err_t err = ESP_OK;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition resized, re-initialize please.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK) return err;

    // Initialize Local Struct
    ESP_LOGE(TAG, "Open NVS to namespace 'settings'...");

    nvs_handle nvs;
    // NVS partition opened in RW mode to create namespace if first run:
    err = nvs_open("settings", NVS_READWRITE, &nvs);
    if (err != ESP_OK) return err;
    
    ESP_LOGE(TAG, "- get: wifi_ssid");
    size_t str_len = WIFI_SSID_MAX_LEN;
    err = nvs_get_str(nvs, "wifi_ssid", _current_settings.wifi_ssid, &str_len);
    if (_is_nvs_err(err, nvs)) return err;

    ESP_LOGE(TAG, "- get: wifi_key");
    str_len = WIFI_PSK_MAX_LEN;
    err = nvs_get_str(nvs, "wifi_key", _current_settings.wifi_key, &str_len);
    if (_is_nvs_err(err, nvs)) return err;

    ESP_LOGE(TAG, "- get: flags");
    err = nvs_get_u16(nvs, "flags", &_current_settings.flags_raw);
    if (_is_nvs_err(err, nvs)) return err;

    _initialized = true;
    nvs_close(nvs);
    return ESP_OK;
}

esp_err_t settings_get_data(settings_t *data) {
    if (!_initialized) return ESP_FAIL;
    *data = _current_settings;
    return ESP_OK;
}

esp_err_t settings_set_data(settings_t *data) {
    esp_err_t err = ESP_OK;

    nvs_handle nvs;
    err = nvs_open("settings", NVS_READWRITE, &nvs);
    if (err != ESP_OK) return err;

    if (strcmp(_current_settings.wifi_ssid, data->wifi_ssid)) {
        err = nvs_set_str(nvs, "wifi_ssid", data->wifi_ssid);
        if (_is_nvs_err(err, nvs)) return err;
    }

    if (strcmp(_current_settings.wifi_key, data->wifi_key)) {
        err = nvs_set_str(nvs, "wifi_key", data->wifi_key);
        if (_is_nvs_err(err, nvs)) return err;
    }

    if (_current_settings.flags_raw != data->flags_raw) {
        err = nvs_set_u16(nvs, "flags", data->flags_raw);
        if (_is_nvs_err(err, nvs)) return err;
    }

    err = nvs_commit(nvs);
    if (_is_nvs_err(err, nvs)) return err;
    nvs_close(nvs);

    _current_settings = *data;
    return ESP_OK;
}