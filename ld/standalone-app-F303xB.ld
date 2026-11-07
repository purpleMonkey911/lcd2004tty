/**
 * @file      standalone-app-F303xB.ld
 * @brief     Linker script for STM32F303xB standalone application
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 */

INCLUDE standalone-memory-F303.ld

INCLUDE standalone-common.ld

SECTIONS
{
    .ccmram :
    {
        . = ALIGN(4);
        _sccmram = .;       /* create a global symbol at ccmram start */
        *(.ccmram)
        *(.ccmram*)
    
        . = ALIGN(4);
        _eccmram = .;       /* create a global symbol at ccmram end */
    } >ccram AT> app
}
