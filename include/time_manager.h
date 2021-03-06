#ifndef __time_manager_h
#define __time_manager_h

#include "esp_err.h"
#include <sys/time.h>

#define EORZEAN_TIME_CONSTANT (3600.0 / 175.0)

typedef long gmt_offset_t;
typedef long dst_offset_t;

esp_err_t time_manager_init(void);
esp_err_t time_manager_update_ntp(void);
esp_err_t time_manager_update_rtc(void);
esp_err_t time_manager_set_time(struct tm time);
esp_err_t time_manager_set_zone(gmt_offset_t gmt_offset_s, dst_offset_t dst_offset_s, uint8_t is_dst);
esp_err_t time_manager_get_local_time(struct tm* time);
esp_err_t time_manager_get_eorzea_time(struct tm* time);

#endif