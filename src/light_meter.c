#include "light_meter.h"
#include "driver/adc.h"

static const adc1_channel_t CHANNEL = ADC1_CHANNEL_3;

esp_err_t light_meter_init(void) {
    const adc_bits_width_t width = ADC_WIDTH_BIT_12;
    const adc_atten_t atten = ADC_ATTEN_DB_11;

    adc1_config_width(width);
    adc1_config_channel_atten(CHANNEL, atten);

    return ESP_OK;
}

esp_err_t light_meter_get_reading(long *val, uint8_t sample_count) {
    uint32_t reading = 0;

    for (int i = 0; i < sample_count; i++) {
        uint32_t raw = adc1_get_raw(CHANNEL);
        reading += raw;
    }

    *val = (reading / sample_count);
    return ESP_OK;
}