/**
 * @file uart_inst.h
 * 
 * @brief UART instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#ifndef __UART_INST_H__
#define __UART_INST_H__

#include "mhal_uart.h"

/**
 * @brief initialize Uart instance
 */ 
extern void Uart_Init(void);

/**
 * @brief Accessor function to get Uart instance
 */ 
extern Uart Uart_GetInstance(void);

#endif // __UART_INST_H__
