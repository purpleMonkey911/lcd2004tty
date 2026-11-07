/**
 * @file i2c_inst.h
 * 
 * @brief I2c instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#ifndef __I2C_INST_H__
#define __I2C_INST_H__

#include "mhal_i2c.h"

/**
 * @brief initialize I2c instance
 */ 
extern void I2c_Init(void);

/**
 * @brief Accessor function to get I2c instance
 */ 
extern I2c I2c_GetInstance(void);

#endif // __UART_INST_H__
