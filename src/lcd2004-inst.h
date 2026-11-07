/**
 * @file lcd2004-inst.h
 * 
 * @brief Lcd2004 instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#ifndef LCD2004_INST_H__
#define LCD2004_INST_H__

#include "lcd2004i2c.h"

/**
 * @brief initialize Lcd2004 instance
 */ 
extern void Lcd_Init(Lcd2004Config* config);

/**
 * @brief Accessor function to get Lcd2004 instance
 */ 
extern Lcd2004 Lcd_GetInstance(void);

#endif // LCD2004_INST_H__
