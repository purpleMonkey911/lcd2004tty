/**
 * @file      memory-F303xB.ld
 * @brief     Memory layout for STM32F303xB
 *                      128Kbytes flash
 *                      32Kbytes RAM (SRAM) + 8Kbytes RAM (CCRAM)
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 */


/* Memories definition */
MEMORY
{
    bootloader (rx)     : ORIGIN = 0x08000000,            LENGTH = 48K - 100
    bootloader_info (rx): ORIGIN = 0x08000000+(48K-100),  LENGTH = 100
    app    (rx)         : ORIGIN = 0x0800C000,            LENGTH = 128K-48K-2K
    params (rx)         : ORIGIN = 0x08000000+126K,       LENGTH = 2K
    ram    (rwx)        : ORIGIN = 0x20000000,            LENGTH = 32K-256
    app_bl_shared (rwx) : ORIGIN = 0x20000000+(32K-256),  LENGTH = 256

    ccram  (rwx)        : ORIGIN = 0x10000000,            LENGTH = 8K
}

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
