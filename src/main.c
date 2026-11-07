/**
 * @file           : main.c
 * @brief          : Entry point for lcd2004tty application
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 * 
 */

#include "app_config.h"
#include "i2c_inst.h"
#include "lcd2004-inst.h"
#include "led.h"
#include "mhal_gpio.h"
#include "mhal_i2c.h"
#include "mhal_interrupt.h"
#include "mhal_timer.h"
#include "system_init.h"
#include "tty.h"
#include "uart_inst.h"

#define LED_TOGGLE_INTERVAL_US   1000000

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    system_init();
    mHalTimer_SystemTimerInit();

    // Clear and enable interrupts
    mHalIntr_Clear();
    mHalIntr_Enable();

    // setup debug LED
    mHalTimerObj led_timer = {0};
    mHalTimer_StartTimerObj(&led_timer);
    led_init();

    // create and init I/O instances
    Uart_Init();
    I2c_Init();

    // create and init LCD
    Lcd2004Config lcd_config = {
        .i2c = I2c_GetInstance(),
        .slave_addr = LCD2004_I2C_SLAVE_ADDR,
    };
    Lcd_Init(&lcd_config);

    // create and init TTY module
    TtyConfig tty_config = {
        .uart_in = Uart_GetInstance(),
        .lcd_out = Lcd_GetInstance(),
    };
    Tty_Init(&tty_config);

    while (1)
    {
        if (mHalTimer_TimerObjElapsed(&led_timer, LED_TOGGLE_INTERVAL_US)) {
            mHalGpio_Toggle(led_GetGpio());
            mHalTimer_StartTimerObj(&led_timer);
        }

        // exercise state machines
        if (mHalUart_StateMachine(Uart_GetInstance()) < MHAL_STATUS_OK) {
            // set off howling alarm!
        }
        if (mHalI2c_StateMachine(I2c_GetInstance()) < MHAL_STATUS_OK) {
            // set off howling alarm!
        }
        if (Lcd2004_StateMachine(Lcd_GetInstance()) < LCD2004_STATUS_OK) {
            // set off howling alarm
        }

        Tty_Process();
    }

    // we should never get here!
    return 0;
}
