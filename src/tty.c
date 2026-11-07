/**
 * @file tty.c
 * 
 * @brief TTY module for the app
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include <string.h>
#include "tty.h"

typedef struct _tty_t {
    Uart uart_in;
    Lcd2004 lcd_out;
    uint8_t buffer[LCD2004_ROW_MAX][LCD2004_COL_MAX];
    uint8_t current_row;
    uint8_t current_col;
    uint8_t row0;
} Tty_t;
typedef Tty_t* Tty;

static Tty_t tty_app;
static Tty ptty = &tty_app;

static char blank_line[LCD2004_COL_MAX] = "                    ";

void Tty_Init(TtyConfig* config) {
    if (!config) return;

    ptty->uart_in = config->uart_in;
    ptty->lcd_out = config->lcd_out;
    ptty->current_row = 0;
    ptty->current_col = 0;
    ptty->row0 = 0;
}

void Tty_Process(void) {
    uint8_t data = 0;
    if (mHalUart_GetByte(ptty->uart_in, &data) == MHAL_STATUS_OK) {
        switch (data) {
            case 0x0A:   // line feed
            case 0x08:   // backspace
            case 0x7F:   // delete (DEL)
                // ignore these characters
                break;
            case 0x0D:
                // fill in the rest of the line with spaces
                for (int i = ptty->current_col; i < LCD2004_COL_MAX; i++) {
                    ptty->buffer[ptty->current_row][ptty->current_col++] = 0x20;
                }
                ptty->current_row++;
                ptty->current_col = 0;
                if (ptty->current_row < LCD2004_ROW_MAX) {
                    Lcd2004_SetCursor(ptty->lcd_out, ptty->current_row, 0);
                }
                break;
            default:
                ptty->buffer[ptty->current_row][ptty->current_col++] = data;
                Lcd2004_Write(ptty->lcd_out, &data, sizeof(data));

                // check column bounds
                if (ptty->current_col >= LCD2004_COL_MAX) {
                    ptty->current_col = 0;
                    ptty->current_row++;
                    if (ptty->current_row < LCD2004_ROW_MAX) {
                        Lcd2004_SetCursor(ptty->lcd_out, ptty->current_row, 0);
                    }
                }
                break;
        }

        // check screen redraw
        if (ptty->current_row >= LCD2004_ROW_MAX) {
#if 0
            ptty->row0++;
            ptty->current_row &= 0x3;
            ptty->row0 &= 0x3;

            // redraw screen
            for (int i = 0; i < (LCD2004_ROW_MAX - 1); i++) {
                Lcd2004_SetCursor(ptty->lcd_out, i, 0);
                Lcd2004_Write(ptty->lcd_out, &ptty->buffer[(ptty->row0 + i) & 0x3][0], LCD2004_COL_MAX);
//                ptty->current_row++;
            }
            Lcd2004_SetCursor(ptty->lcd_out, 3, 0);
            Lcd2004_Write(ptty->lcd_out, (uint8_t*)blank_line, LCD2004_COL_MAX);
            Lcd2004_SetCursor(ptty->lcd_out, 3, 0);
#endif
            // copy
            memcpy(&ptty->buffer[0][0], &ptty->buffer[1][0], LCD2004_COL_MAX);
            memcpy(&ptty->buffer[1][0], &ptty->buffer[2][0], LCD2004_COL_MAX);
            memcpy(&ptty->buffer[2][0], &ptty->buffer[3][0], LCD2004_COL_MAX);

            // redraw screen
            for (int i = 0; i < (LCD2004_ROW_MAX - 1); i++) {
                Lcd2004_SetCursor(ptty->lcd_out, i, 0);
                Lcd2004_Write(ptty->lcd_out, &ptty->buffer[i][0], LCD2004_COL_MAX);
            }
            Lcd2004_SetCursor(ptty->lcd_out, 3, 0);
            Lcd2004_Write(ptty->lcd_out, (uint8_t*)blank_line, LCD2004_COL_MAX);
            Lcd2004_SetCursor(ptty->lcd_out, 3, 0);
            ptty->current_row = 3;
        }
    }
}
