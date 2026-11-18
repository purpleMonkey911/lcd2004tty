/**
 * @file tty.c
 *
 * @brief TTY module for the app
 *
 *        Incoming UART characters update an in-memory copy of the screen
 *        (screen[][]).  A second copy (shown[][]) tracks what is actually on
 *        the LCD.  Whenever the LCD is idle, Tty_Refresh() sends the changed
 *        part of one row, so at most one row is ever queued to the LCD.
 *
 *        This decouples the input rate from the (much slower) LCD:
 *        - the LCD ring buffer can't overflow
 *        - a burst of input that scrolls several lines while the LCD is busy
 *          only costs the final screen's worth of I2C traffic; intermediate
 *          screens are skipped, but the display always ends up correct
 *        - scrolling only sends the cells that actually changed
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 *
 */

#include <string.h>
#include "tty.h"

typedef struct _tty_t {
    Uart uart_in;
    Lcd2004 lcd_out;
    uint8_t screen[LCD2004_ROW_MAX][LCD2004_COL_MAX];  // what should be displayed
    uint8_t shown[LCD2004_ROW_MAX][LCD2004_COL_MAX];   // what the LCD displays
    uint8_t current_row;
    uint8_t current_col;
    uint8_t lcd_row;     // where the LCD's address counter points, so
    uint8_t lcd_col;     // SetCursor can be skipped for sequential writes
} Tty_t;
typedef Tty_t* Tty;

static Tty_t tty_app;
static Tty ptty = &tty_app;

void Tty_Init(TtyConfig* config) {
    if (!config) return;

    ptty->uart_in = config->uart_in;
    ptty->lcd_out = config->lcd_out;
    ptty->current_row = 0;
    ptty->current_col = 0;

    // the LCD init sequence clears the display, which also homes the
    // address counter to row 0, col 0
    ptty->lcd_row = 0;
    ptty->lcd_col = 0;
    memset(ptty->screen, ' ', sizeof(ptty->screen));
    memset(ptty->shown, ' ', sizeof(ptty->shown));
}

/**
 * @brief Move the cursor to the start of the next row, scrolling the screen
 *        up one row if the cursor is already on the last row.
 */
static void Tty_NewLine(void) {
    ptty->current_col = 0;
    ptty->current_row++;
    if (ptty->current_row >= LCD2004_ROW_MAX) {
        memmove(&ptty->screen[0][0], &ptty->screen[1][0],
                (LCD2004_ROW_MAX - 1) * LCD2004_COL_MAX);
        memset(&ptty->screen[LCD2004_ROW_MAX - 1][0], ' ', LCD2004_COL_MAX);
        ptty->current_row = LCD2004_ROW_MAX - 1;
    }
}

/**
 * @brief Apply one received character to screen[][].  No LCD I/O.
 */
static void Tty_PutChar(uint8_t data) {
    switch (data) {
        case 0x0A:   // line feed
        case 0x08:   // backspace
        case 0x7F:   // delete (DEL)
            // ignore these characters
            break;
        case 0x0D:   // carriage return
            // blank the rest of the line, then move to the next line
            memset(&ptty->screen[ptty->current_row][ptty->current_col], ' ',
                   LCD2004_COL_MAX - ptty->current_col);
            Tty_NewLine();
            break;
        default:
            ptty->screen[ptty->current_row][ptty->current_col++] = data;
            if (ptty->current_col >= LCD2004_COL_MAX) {
                Tty_NewLine();
            }
            break;
    }
}

/**
 * @brief If the LCD is idle, send the changed span of the first out-of-date
 *        row.  The next row goes out on a later call, once this one is done.
 */
static void Tty_Refresh(void) {
    if (!Lcd2004_IsIdle(ptty->lcd_out)) {
        return;
    }

    for (uint8_t row = 0; row < LCD2004_ROW_MAX; row++) {
        // find the first and last changed column in this row
        int first = 0;
        int last = LCD2004_COL_MAX - 1;
        while ((first <= last) && (ptty->screen[row][first] == ptty->shown[row][first])) {
            first++;
        }
        if (first > last) {
            continue;   // row is up to date
        }
        while (ptty->screen[row][last] == ptty->shown[row][last]) {
            last--;
        }

        // A span never crosses a row: LCD DDRAM rows are not contiguous
        // (row 0 runs on into row 2), so each row needs its own SetCursor.
        // The LCD auto-increments its address after each character, so the
        // SetCursor is skipped when continuing where the last write ended
        // (e.g. typing).  lcd_col == LCD2004_COL_MAX never matches, so a
        // write that ended at the end of a row always gets a SetCursor.
        size_t len = (size_t)(last - first + 1);
        if ((row != ptty->lcd_row) || (first != ptty->lcd_col)) {
            if (Lcd2004_SetCursor(ptty->lcd_out, row, (uint8_t)first) != LCD2004_STATUS_OK) {
                return;
            }
        }
        if (Lcd2004_Write(ptty->lcd_out, &ptty->screen[row][first], len) == LCD2004_STATUS_OK) {
            memcpy(&ptty->shown[row][first], &ptty->screen[row][first], len);
            ptty->lcd_row = row;
            ptty->lcd_col = (uint8_t)(first + len);
        }
        return;
    }
}

void Tty_Process(void) {
    // Drain everything the UART has received.  This only updates screen[][],
    // so it is fast no matter how slow the LCD is.
    uint8_t data = 0;
    while (mHalUart_GetByte(ptty->uart_in, &data) == MHAL_STATUS_OK) {
        Tty_PutChar(data);
    }

    Tty_Refresh();
}
