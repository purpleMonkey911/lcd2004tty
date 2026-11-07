/**
 * @file lcd2004i2c.h
 * 
 * @brief Device driver and API for LCD2004 LCD screen with I2C interface
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#ifndef __LCD2004_H__
#define __LCD2004_H__

#include "mhal_i2c.h"

typedef struct _lcd2004_t lcd2004_t;
typedef lcd2004_t* Lcd2004;

typedef enum {
    LCD2004_STATUS_ERROR_IO  = -2,
    LCD2004_STATUS_ERROR     = -1,
    LCD2004_STATUS_OK        = 0,
    LCD2004_STATUS_IDLE      = 0,
    LCD2004_STATUS_INIT      = 1,
    LCD2004_STATUS_BUSY      = 2,
    LCD2004_STATUS_DONE      = 3,
    LCD2004_STATUS_INIT_DONE = 4,
} Lcd2004Status;

typedef struct {
    I2c i2c;
    uint8_t slave_addr;
} Lcd2004Config;

#define LCD2004_ROW_MAX   4  // row count is 0-based
#define LCD2004_COL_MAX   20 // col count is 0-based

/**
 * @brief Start initializaition of Lcd2004 instance
 * 
 * @param pLcd   pointer to Lcd2004 instance
 * @param config pointer to Lcd2004Configi Lcd2004
 * 
 * @note this function assumes that the I2C bus is already instantiated
 * 
 * @return LCD2004_STATUS_OK if initialization successfully started
 */ 
extern Lcd2004Status Lcd2004_Init(Lcd2004* pLcd, Lcd2004Config* config);

/**
 * @brief Send command to clear LCD screen
 * 
 * @param lcd   Lcd2004 instance
 * 
 * @return LCD2004_STATUS_OK if command successfully queued
 */ 
extern Lcd2004Status Lcd2004_ClearScreen(Lcd2004 lcd);

/**
 * @brief Send command to set cursor of LCD screen
 * 
 * @param lcd   Lcd2004 instance
 * 
 * @note row and col values are 0-based
 * 
 * @return LCD2004_STATUS_OK if command successfully queued
 */ 
extern Lcd2004Status Lcd2004_SetCursor(Lcd2004 lcd, uint8_t row, uint8_t col);

/**
 * @brief Write data to LCD screen
 * 
 * @param lcd   Lcd2004 instance
 * 
 * @return LCD2004_STATUS_OK if data successfully queued
 */ 
extern Lcd2004Status Lcd2004_Write(Lcd2004 lcd, uint8_t* data, size_t data_len);

/**
 * @brief Exercise Lcd2004 state machine
 * 
 * @param lcd   Lcd2004 instance
 * 
 * @return Lcd2004Status of state machine execution
 */ 
extern Lcd2004Status Lcd2004_StateMachine(Lcd2004 lcd);

#endif // __LCD2004_H__
