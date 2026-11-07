/**
 * @file i2c_inst.h
 * 
 * @brief I2c instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "app_config.h"
#include "i2c_inst.h"
#include "mhal_gpio.h"

static I2c i2c_inst = 0;

void I2c_Init(void) {
    I2cConfig config = {
        .gpio = {
            .scl = {
                .port = I2C_SCL_PORT,
                .pin = I2C_SCL_PIN,
            },
            .scl_config = {
                .mode = GPIO_MODE_ALT_OD,
                .pull = GPIO_PULL_UP,
                .speed = GPIO_SPEED_HI,
                .alt_func = GPIO_AF_4_I2C1,
            },
            .sda = {
                .port = I2C_SDA_PORT,
                .pin = I2C_SDA_PIN,
            },
            .sda_config = {
                .mode = GPIO_MODE_ALT_OD,
                .pull = GPIO_PULL_UP,
                .speed = GPIO_SPEED_HI,
                .alt_func = GPIO_AF_4_I2C1,
            },
        },
        .index = I2C_INDEX,
    };
    mHalI2c_Init(&i2c_inst, &config);
}

/**
 * @brief Accessor function to get I2c instance
 */ 
I2c I2c_GetInstance(void) {
    return i2c_inst;
}
