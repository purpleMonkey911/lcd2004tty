/**
 * @file led.h
 * 
 * @brief LED module for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#ifndef __LED_H__
#define __LED_H__

#include "mhal_gpio.h"

/**
 * @brief initialize LED module
 */ 
extern void led_init(void);

/**
 * @brief Get LED GPIO
 */ 
extern Gpio led_GetGpio(void);

#endif // __LED_H__