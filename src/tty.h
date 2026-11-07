/**
 * @file tty.h
 * 
 * @brief TTY module for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#ifndef __TTY_H__
#define __TTY_H__

#include "lcd2004i2c.h"
#include "mhal_uart.h"

typedef struct {
    Uart uart_in;
    Lcd2004 lcd_out;
} TtyConfig;

/**
 * @brief Initialize TTY module
 * 
 * @param [in] config  pointer to TtyConfig
 * 
 */ 
extern void Tty_Init(TtyConfig* config);

/**
 * @brief Process any TTY data.  Take in UART data and display on LCD.
 * 
 */ 
extern void Tty_Process(void);

#endif // __TTY_H__