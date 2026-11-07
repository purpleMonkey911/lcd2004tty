/**
 * @file lcd2004-inst.c
 * 
 * @brief Lcd2004 instance for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "lcd2004-inst.h"

static Lcd2004 lcd_out = 0;

void Lcd_Init(Lcd2004Config* config) {
    Lcd2004_Init(&lcd_out, config);
}

Lcd2004 Lcd_GetInstance(void) {
    return lcd_out;
}
