/**
 * @file led.c
 * 
 * @brief LED module for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "app_config.h"
#include "led.h"

static Gpio led_gpio = {
    .port = LED_GPIO_PORT,
    .pin = LED_GPIO_PIN,
};

void led_init(void) {
    GpioConfig led_config = {
        .mode = GPIO_MODE_OUT_PP,
        .pull = GPIO_PULL_NONE,
        .speed = GPIO_SPEED_LO,
        .alt_func = GPIO_AF_0_NONE,
    };
    mHalGpio_Init(led_gpio, &led_config);
    mHalGpio_Write(led_gpio, GPIO_STATE_LOW);    
}

Gpio led_GetGpio(void) {
    return led_gpio;
}
