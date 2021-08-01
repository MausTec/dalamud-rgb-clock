#ifndef __pinout_h
#define __pinout_h

#include "driver/gpio.h"

#define LED_DOUT_GPIO        GPIO_NUM_2
#define LIGHT_METER_GPIO     GPIO_NUM_39 // sens vn
#define AUDIO_PICKUP_GPIO    GPIO_NUM_36 // sens vp

#define BUTTON_0_GPIO        GPIO_NUM_0
#define BUTTON_1_GPIO        GPIO_NUM_12
#define BUTTON_2_GPIO        GPIO_NUM_13
#define BUTTON_3_GPIO        GPIO_NUM_14

#define TOUCH_PAD_GPIO       GPIO_NUM_4
#define RTC_INT_GPIO         GPIO_NUM_5
#define IR_RECV_GPIO         GPIO_NUM_32
#define PWM_IN_B_GPIO        GPIO_NUM_33
#define PWM_IN_R_GPIO        GPIO_NUM_34
#define PWM_IN_G_GPIO        GPIO_NUM_35

#define I2C_SDA_GPIO         GPIO_NUM_21
#define I2C_SCL_GPIO         GPIO_NUM_22

#endif