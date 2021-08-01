#ifndef __light_meter_h
#define __light_meter_h

#include "pinout.h"
#include "esp_err.h"

esp_err_t light_meter_init(void);
esp_err_t light_meter_get_reading(long *val, uint8_t sample_count);

#endif