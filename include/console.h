#ifndef __console_h
#define __console_h

#include "esp_err.h"

esp_err_t console_init(void);
esp_err_t console_tick(void);

#endif