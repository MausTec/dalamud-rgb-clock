#ifndef __settings_h
#define __settings_h

#include "esp_err.h"

#define WIFI_SSID_MAX_LEN    32
#define WIFI_PSK_MAX_LEN     63
#define NTP_SERVER_MAX_LEN   63

struct settings {
    char wifi_ssid[WIFI_SSID_MAX_LEN + 1] = "Here Be Dragons";
    char wifi_key[WIFI_PSK_MAX_LEN + 1] = "RAWR!barkbark";
    char ntp_server[NTP_SERVER_MAX_LEN + 1] = "";

    long gmt_offset_s = -6 * 3600; // Default to US/Central
    long dst_offset_s = 3600;
    
    union {
        struct {
            uint16_t wifi_on : 1;
            uint16_t ntp_on  : 1;
            uint16_t is_dst  : 1;
        } flags;
        uint16_t flags_raw = 0b0000000000000111;
    };
};

typedef struct settings settings_t;

esp_err_t settings_init(void);
esp_err_t settings_get_data(settings_t *data);
esp_err_t settings_set_data(settings_t *data);

#endif