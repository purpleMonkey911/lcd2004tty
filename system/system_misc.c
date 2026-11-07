/**
 * @file system_misc.c
 * 
 * @brief Miscellaneous system calls 
 * 
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 *
 */

#include <stdint.h>

#if defined (STM32F303xC)
  #include "stm32f3xx_hal.h"
  #include "stm32f3xx_hal_rcc.h"
#elif defined (STM32G474xx)
  #include "stm32g4xx_hal.h"
  #include "stm32g4xx_hal_rcc.h"
#elif defined(STM32L496xx) || defined(STM32L432xx)
  #include "stm32l4xx_hal.h"
  #include "stm32l4xx_hal_rcc.h"
#else
  #error MCU NOT SUPPORTED
#endif

#include "mhal_interrupt.h"
#include "memory.h"

/**
  * @brief  Jump to an address in code in flash.
  *         It is assumed we are jumping to an entry point of a valid binary image
  *
  * @param [in] pc    program counter
  * @param [in] sp    stack pointer
  */
static void __attribute__((naked)) System_JumpToCode(uint32_t pc, uint32_t sp) {
    __asm("                                            \n\
          msr msp, r1 /* load r1 into MSP */           \n\
          bx r0       /* branch to the address at r0 */\n\
    ");
}

/**
  * @brief  Sets up stack pointer and program counter to faciliate the jump to
  *         the bootloader.
  */
void System_JumpToBootloader(void) {
    uint32_t *boot      = (uint32_t*)BOOTLOADER_ADDR;
    uint32_t boot_sp    = boot[0];
    uint32_t boot_start = boot[1];

    HAL_RCC_DeInit();
    HAL_DeInit();
    System_JumpToCode(boot_start, boot_sp);
}

/**
  * @brief  Sets up stack pointer and program counter to faciliate the jump to
  *         the app.
  */
void System_JumpToApp(void) {
    uint32_t *app      = (uint32_t*)APP_ADDR;
    uint32_t app_sp    = app[0];
    uint32_t app_start = app[1];

    mHalIntr_Disable();
    HAL_RCC_DeInit();
    HAL_DeInit();
    System_JumpToCode(app_start, app_sp);
}
