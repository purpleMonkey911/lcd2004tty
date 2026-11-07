/**
 * @file lcd2004i2c.c
 * 
 * @brief Device driver and API for LCD2004 LCD screen with I2C interface
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include <stdlib.h>
#include <string.h>

#include "lcd2004i2c.h"
#include "mhal_timer.h"
#include "ring_buffer.h"

typedef enum {
    LCD2004_XFER_TYPE_CMD,
    LCD2004_XFER_TYPE_DATA,
} Lcd2004XferType;

static I2cXfer lcd2004i2c_xfer = {0};

static uint8_t lcd2004_init1_data[] = { 
    0x08,        // set pins to known state
    0x3C, 0x38   // Nibble 0x3
};
static uint8_t lcd2004_init2_data[] = { 
    0x3C, 0x38,             // Nibble 0x3
    0x3C, 0x38,             // Nibble 0x3
    0x2C, 0x28,             // Nibble 0x2; enter 4-bit mode 
    0x2C, 0x28, 0x8C, 0x88, // Function set (4-bit, 2-line, 5x8)
    0x0C, 0x08, 0x8C, 0x88, // Display off
    0x0C, 0x08, 0x1C, 0x18, // Clear screen
};
static uint8_t lcd2004_init3_data[] = {
    0x0C, 0x08, 0x6C, 0x68, // Entry mode (increment, no shift)
    0x0C, 0x08, 0xCC, 0xC8, // Display on; Cursor off
};

// Timer limits
#define LCD2004_TIMER_POWER_UP_USEC     40000
#define LCD2004_TIMER_INIT1_USEC         4100
#define LCD2004_TIMER_INIT2_USEC         1530
#define LCD2004_TIMER_CLEAR_SCREEN_USEC  1530

#define LCD2004_CMD_CLEAR_SCREEN             0x01
#define LCD2004_CMD_4BIT_2LINE_5X8           0x28
#define LCD2004_CMD_DISPLAY_OFF              0x08
#define LCD2004_CMD_ENTRY_MODE_INC_NO_SHIFT  0x06
#define LCD2004_CMD_DISPLAY_ON_CURSOR_OFF    0x0C

#define LCD2004_I2C_TRANSACTION_SZ_BYTES   4

// Each command/data byte is sent in 4 bytes.  The display is 80 (20 x 4) bytes
// total. So worse case scenario is we need 320 byte buffer to fill the entire
// LCD screen at once.
//
//  ┌──────────┬─────────────┬─────┬─────┬─────┬─────┬──────────────────────────────┐
//  │ I2C byte │    D7–D4    │ BL  │  E  │ RW  │ RS  │           Purpose            │
//  ├──────────┼─────────────┼─────┼─────┼─────┼─────┼──────────────────────────────┤
//  │ 1        │ high nibble │ 1   │ 1   │ 0   │ x   │ E rises, data presented      │
//  ├──────────┼─────────────┼─────┼─────┼─────┼─────┼──────────────────────────────┤
//  │ 2        │ high nibble │ 1   │ 0   │ 0   │ x   │ E falls, high nibble latched │
//  ├──────────┼─────────────┼─────┼─────┼─────┼─────┼──────────────────────────────┤
//  │ 3        │ low nibble  │ 1   │ 1   │ 0   │ x   │ E rises, data presented      │
//  ├──────────┼─────────────┼─────┼─────┼─────┼─────┼──────────────────────────────┤
//  │ 4        │ low nibble  │ 1   │ 0   │ 0   │ x   │ E falls, low nibble latched  │
//  └──────────┴─────────────┴─────┴─────┴─────┴─────┴──────────────────────────────┘
#define LCD2004_RING_BUFFER_SZ_BYTES 512  // ring buffer size must be power of 2
typedef union {
    uint8_t value;
    struct {
        uint8_t rs            : 1;
        uint8_t rw            : 1;
        uint8_t enable        : 1;
        uint8_t backlight_en  : 1;
        uint8_t data          : 4;
    } bits;
} Lcd2004Hd44780Byte;

typedef enum {
    LCD2004_STATE_ERROR      = -1,
    LCD2004_STATE_IDLE       = 0,
    LCD2004_STATE_POWER_UP   = 1,
    LCD2004_STATE_INIT1      = 2,
    LCD2004_STATE_INIT2      = 3,
    LCD2004_STATE_INIT3      = 4,
    LCD2004_STATE_CLEAR      = 5,
    LCD2004_STATE_SEND       = 6,  // sending I2C transaction
    LCD2004_STATE_BUSY       = 7,  // waiting for I2C transaction to finish
    LCD2004_STATE_BUSY_WAIT  = 8,  // required wait time after I2C transaction is finish (if necessary)
    LCD2004_STATE_DONE       = 9,  // I2C transaction is complete
    LCD2004_STATE_INIT_DONE  = 10, // Initalization complete
} Lcd2004State;

typedef struct {
    Lcd2004State state;
    Lcd2004State next_state;
    Lcd2004Status pending_error;
    mHalTimerObj timer;
    uint32_t wait_time_us;  // amont of time to wait after a transaction
                            // has completed (us)
} Lcd2004StateMachine;

#define LCD2004_I2C_BUFFER_SZ_BYTES  128
typedef struct _lcd2004_t {
    I2c i2c;
    uint8_t slave_addr;
    RingBuffer data_buffer;
    bool isInitialized;
    bool clear_screen_requested;
    Lcd2004Hd44780Byte i2c_buffer[LCD2004_I2C_BUFFER_SZ_BYTES];
    Lcd2004StateMachine state_machine;
} lcd2004_t;

/**
 * @brief Callback for successful I2C transfers
 * 
 * @param  ctx  context of the callback which is the Lcd2004 instance
 */ 
static void Lcd2004_I2cXferDone_cb(void* ctx) {
    if (ctx) {
        Lcd2004 lcd = (Lcd2004) ctx;
        if (lcd->state_machine.wait_time_us) {
            lcd->state_machine.state = LCD2004_STATE_BUSY_WAIT;
            mHalTimer_StartTimerObj(&lcd->state_machine.timer);
            return;
        }
        lcd->state_machine.state = LCD2004_STATE_DONE;
    }
}

/**
 * @brief Callback for I2C transfer errors
 * 
 * @param  ctx  context of the callback which is the Lcd2004 instance
 */ 
static void Lcd2004_I2cXferError_cb(void* ctx) {
    if (ctx) {
        Lcd2004 lcd = (Lcd2004) ctx;
        lcd->state_machine.pending_error = LCD2004_STATUS_ERROR_IO;
        lcd->state_machine.state = LCD2004_STATE_ERROR;
    }
}

Lcd2004Status Lcd2004_Init(Lcd2004* pLcd, Lcd2004Config* config) {
    if (!config) {
        return LCD2004_STATUS_ERROR;
    }

    Lcd2004 lcd = calloc(1, sizeof(lcd2004_t));
    if (!lcd) {
        return LCD2004_STATUS_ERROR;
    }
    if (RingBuffer_Create(&lcd->data_buffer, LCD2004_RING_BUFFER_SZ_BYTES) != MHAL_STATUS_OK) {
        free(lcd);
        return LCD2004_STATUS_ERROR;
    }
    lcd->i2c = config->i2c;
    lcd->slave_addr = config->slave_addr;

    // set up I2cXfer 
    lcd2004i2c_xfer.mode         = I2C_MODE_STANDARD;
    lcd2004i2c_xfer.address_mode = I2C_ADDRESSING_MODE_7BITS;
    lcd2004i2c_xfer.address      = config->slave_addr;
    lcd2004i2c_xfer.master_mode  = true;
    lcd2004i2c_xfer.xfer_mode    = I2C_XFER_MODE_STREAM;
    lcd2004i2c_xfer.write        = true;
    lcd2004i2c_xfer.reg          = 0; // not used for I2C_XFER_MODE_STREAM
    lcd2004i2c_xfer.reg_size     = 0; // not used for I2C_XFER_MODE_STREAM
    lcd2004i2c_xfer.done_cb      = Lcd2004_I2cXferDone_cb;
    lcd2004i2c_xfer.done_cb_ctx  = lcd;
    lcd2004i2c_xfer.error_cb     = Lcd2004_I2cXferError_cb;
    lcd2004i2c_xfer.error_cb_ctx = lcd;

    // init state machine
    lcd->state_machine.next_state    = LCD2004_STATE_DONE;
    lcd->state_machine.pending_error = LCD2004_STATUS_OK;
    lcd->state_machine.state         = LCD2004_STATE_POWER_UP;
    mHalTimer_StartTimerObj(&lcd->state_machine.timer);

    lcd->isInitialized = false;
    *pLcd = lcd;
    return LCD2004_STATUS_OK;
}

Lcd2004Status Lcd2004_ClearScreen(Lcd2004 lcd) {
    if (!lcd) {
        return LCD2004_STATUS_ERROR;
    }
    lcd->clear_screen_requested = true;

    // reset ring buffer to erase any pending data
    RingBuffer_Reset(lcd->data_buffer);

    return LCD2004_STATUS_OK;
}

static Lcd2004Status Lcd2004_ClearScreenSetup(Lcd2004 lcd) {
    if (!lcd) {
        return LCD2004_STATUS_ERROR;
    }

    Lcd2004Hd44780Byte* buffer = lcd->i2c_buffer;
    // high nibble 1st part
    buffer[0].value = LCD2004_CMD_CLEAR_SCREEN & 0xF0;
    buffer[0].bits.backlight_en = 1;
    buffer[0].bits.enable = 1;
    buffer[0].bits.rw = 0;
    buffer[0].bits.rs = 0;
    // high nibble 2nd part
    buffer[1].value = LCD2004_CMD_CLEAR_SCREEN & 0xF0;
    buffer[1].bits.backlight_en = 1;
    buffer[1].bits.enable = 0;
    buffer[0].bits.rw = 0;
    buffer[1].bits.rs = 0;
    // low nibble 1st part
    buffer[2].value = (LCD2004_CMD_CLEAR_SCREEN & 0x0F) << 4;
    buffer[2].bits.backlight_en = 1;
    buffer[2].bits.enable = 1;
    buffer[0].bits.rw = 0;
    buffer[2].bits.rs = 0;
    // low nibble 2nd part
    buffer[3].value = (LCD2004_CMD_CLEAR_SCREEN & 0x0F) << 4;
    buffer[3].bits.backlight_en = 1;
    buffer[3].bits.enable = 0;
    buffer[0].bits.rw = 0;
    buffer[3].bits.rs = 0;

    return LCD2004_STATUS_OK;    
}

Lcd2004Status Lcd2004_SetCursor(Lcd2004 lcd, uint8_t row, uint8_t col) {
    if (!lcd || (row >= LCD2004_ROW_MAX) || (col >= LCD2004_COL_MAX)) {
        return LCD2004_STATUS_ERROR;
    }
    uint8_t addr = 0; 
    switch (row) {
        case 0:
            addr = 0x00;
            break;
        case 1:
            addr = 0x40;
            break;
        case 2:
            addr = 0x14;
            break;
        case 3:
            addr = 0x54;
            break;
        default:
            break;
    }
    addr += col;
    addr |= 0x80; // bit 7 must be set
    Lcd2004Hd44780Byte buffer[4] = {0};
    // high nibble 1st part
    buffer[0].value = addr & 0xF0;
    buffer[0].bits.backlight_en = 1;
    buffer[0].bits.enable = 1;
    buffer[0].bits.rw = 0;
    buffer[0].bits.rs = 0;
    // high nibble 2nd part
    buffer[1].value = addr & 0xF0;
    buffer[1].bits.backlight_en = 1;
    buffer[1].bits.enable = 0;
    buffer[0].bits.rw = 0;
    buffer[1].bits.rs = 0;
    // low nibble 1st part
    buffer[2].value = (addr & 0x0F) << 4;
    buffer[2].bits.backlight_en = 1;
    buffer[2].bits.enable = 1;
    buffer[0].bits.rw = 0;
    buffer[2].bits.rs = 0;
    // low nibble 2nd part
    buffer[3].value = (addr & 0x0F) << 4;
    buffer[3].bits.backlight_en = 1;
    buffer[3].bits.enable = 0;
    buffer[0].bits.rw = 0;
    buffer[3].bits.rs = 0;

    if (RingBuffer_AddData(lcd->data_buffer, (uint8_t*)buffer, sizeof(buffer)) != MHAL_STATUS_OK) {
        return LCD2004_STATUS_ERROR;
    }
    return LCD2004_STATUS_OK;    
}

Lcd2004Status Lcd2004_Write(Lcd2004 lcd, uint8_t* data, size_t data_len) {
    if (!lcd || !data || !data_len) {
        return LCD2004_STATUS_ERROR;
    }
    Lcd2004Hd44780Byte buffer[LCD2004_I2C_TRANSACTION_SZ_BYTES*data_len];
    memset(buffer, 0, sizeof(buffer));
    for (int i = 0; i < data_len; i++) {
        // high nibble 1st part
        buffer[4*i].value = data[i] & 0xF0;
        buffer[4*i].bits.backlight_en = 1;
        buffer[4*i].bits.enable = 1;
        buffer[4*i].bits.rw = 0;
        buffer[4*i].bits.rs = 1;
        // high nibble 2nd part
        buffer[4*i+1].value = data[i] & 0xF0;
        buffer[4*i+1].bits.backlight_en = 1;
        buffer[4*i+1].bits.enable = 0;
        buffer[4*i+1].bits.rw = 0;
        buffer[4*i+1].bits.rs = 1;
        // low nibble 1st part
        buffer[4*i+2].value = (data[i] & 0x0F) << 4;
        buffer[4*i+2].bits.backlight_en = 1;
        buffer[4*i+2].bits.enable = 1;
        buffer[4*i+2].bits.rw = 0;
        buffer[4*i+2].bits.rs = 1;
        // low nibble 2nd part
        buffer[4*i+3].value = (data[i] & 0x0F) << 4;
        buffer[4*i+3].bits.backlight_en = 1;
        buffer[4*i+3].bits.enable = 0;
        buffer[4*i+3].bits.rw = 0;
        buffer[4*i+3].bits.rs = 1;
    }
    if (RingBuffer_AddData(lcd->data_buffer, (uint8_t*)buffer, sizeof(buffer)) != MHAL_STATUS_OK) {
        return LCD2004_STATUS_ERROR;
    }
    return LCD2004_STATUS_OK;
}

Lcd2004Status Lcd2004_StateMachine(Lcd2004 lcd) {
    Lcd2004Status retval = LCD2004_STATUS_ERROR;
    if (!lcd) {
        return retval;
    }

    switch (lcd->state_machine.state) {
        case LCD2004_STATE_IDLE:
            if (lcd->clear_screen_requested) {
                lcd->state_machine.state = LCD2004_STATE_CLEAR;
                lcd->clear_screen_requested = false;
            } else if (!RingBuffer_IsEmpty(lcd->data_buffer)) {
                lcd->state_machine.state = LCD2004_STATE_SEND;
            }
            retval = lcd->isInitialized ? LCD2004_STATUS_IDLE : LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_POWER_UP:
            if (mHalTimer_TimerObjElapsed(&lcd->state_machine.timer, LCD2004_TIMER_POWER_UP_USEC)) {
                lcd->state_machine.state = LCD2004_STATE_INIT1;
            }
            retval = LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_INIT1:
            if (mHalI2c_Xfer(lcd->i2c, &lcd2004i2c_xfer, lcd2004_init1_data, sizeof(lcd2004_init1_data)) != MHAL_STATUS_OK) {
                // will retry on next state machine iteration
                retval = LCD2004_STATUS_INIT;
                break;
            }
            lcd->state_machine.wait_time_us = LCD2004_TIMER_INIT1_USEC;
            lcd->state_machine.next_state = LCD2004_STATE_INIT2;
            lcd->state_machine.state = LCD2004_STATE_BUSY;
            retval = LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_INIT2:
            if (mHalI2c_Xfer(lcd->i2c, &lcd2004i2c_xfer, lcd2004_init2_data, sizeof(lcd2004_init2_data)) != MHAL_STATUS_OK) {
                // will retry on next state machine iteration
                retval = LCD2004_STATUS_INIT;
                break;
            }
            lcd->state_machine.wait_time_us = LCD2004_TIMER_INIT2_USEC;
            lcd->state_machine.next_state = LCD2004_STATE_INIT3;
            lcd->state_machine.state = LCD2004_STATE_BUSY;
            retval = LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_INIT3:
            if (mHalI2c_Xfer(lcd->i2c, &lcd2004i2c_xfer, lcd2004_init3_data, sizeof(lcd2004_init3_data)) != MHAL_STATUS_OK) {
                // will retry on next state machine iteration
                retval = LCD2004_STATUS_INIT;
                break;
            }
            lcd->state_machine.wait_time_us = 0;
            lcd->state_machine.next_state = LCD2004_STATE_INIT_DONE;
            lcd->state_machine.state = LCD2004_STATE_BUSY;
            retval = LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_CLEAR:
            Lcd2004_ClearScreenSetup(lcd);
            if (mHalI2c_Xfer(lcd->i2c, &lcd2004i2c_xfer, (uint8_t*)lcd->i2c_buffer, LCD2004_I2C_TRANSACTION_SZ_BYTES) != MHAL_STATUS_OK) {
                // will retry on next state machine iteration
                retval = lcd->isInitialized ? LCD2004_STATUS_BUSY : LCD2004_STATUS_INIT;
                break;
            }
            lcd->state_machine.next_state = LCD2004_STATE_IDLE;
            lcd->state_machine.state = LCD2004_STATE_BUSY;
            lcd->state_machine.wait_time_us = LCD2004_TIMER_CLEAR_SCREEN_USEC;
            retval = lcd->isInitialized ? LCD2004_STATUS_BUSY : LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_SEND:
            if (RingBuffer_IsEmpty(lcd->data_buffer)) {
                // nothing to do
                lcd->state_machine.state = LCD2004_STATE_IDLE;
                retval = LCD2004_STATUS_IDLE;
                break;
            }
            // get data from ring buffer and send
            if (RingBuffer_GetData(lcd->data_buffer, (uint8_t*)lcd->i2c_buffer, LCD2004_I2C_TRANSACTION_SZ_BYTES) != MHAL_STATUS_OK) {
                lcd->state_machine.pending_error = LCD2004_STATUS_ERROR;
                lcd->state_machine.state = LCD2004_STATE_ERROR;
                retval = lcd->isInitialized ? LCD2004_STATUS_BUSY : LCD2004_STATUS_INIT;
                break;
            }
            if (mHalI2c_Xfer(lcd->i2c, &lcd2004i2c_xfer, (uint8_t*)lcd->i2c_buffer, LCD2004_I2C_TRANSACTION_SZ_BYTES) != MHAL_STATUS_OK) {
                lcd->state_machine.pending_error = LCD2004_STATUS_ERROR_IO;
                lcd->state_machine.state = LCD2004_STATE_ERROR;
                break;
            }
            lcd->state_machine.wait_time_us = 0;
            lcd->state_machine.next_state = LCD2004_STATE_IDLE;
            lcd->state_machine.state = LCD2004_STATE_BUSY;
            retval = lcd->isInitialized ? LCD2004_STATUS_BUSY : LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_BUSY:
            // wait for mHAL I2C done or error ISR
            retval = lcd->isInitialized ? LCD2004_STATUS_BUSY : LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_BUSY_WAIT:
            if (mHalTimer_TimerObjElapsed(&lcd->state_machine.timer, lcd->state_machine.wait_time_us)) {
                lcd->state_machine.state = LCD2004_STATE_DONE;
            }
            retval = lcd->isInitialized ? LCD2004_STATUS_BUSY : LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_DONE:
            lcd->state_machine.state = lcd->state_machine.next_state;
            lcd->state_machine.next_state = LCD2004_STATE_IDLE;
            retval = lcd->isInitialized ? LCD2004_STATUS_DONE : LCD2004_STATUS_INIT;
            break;
        case LCD2004_STATE_INIT_DONE:
            lcd->state_machine.state = LCD2004_STATE_IDLE;
            lcd->isInitialized = true;
            retval = LCD2004_STATUS_INIT_DONE;
            break;
        case LCD2004_STATE_ERROR:
        default:
            retval = lcd->state_machine.pending_error;
            lcd->state_machine.pending_error = LCD2004_STATUS_OK;
            lcd->state_machine.state = LCD2004_STATE_IDLE;
            break;
    }

    return retval;
}
