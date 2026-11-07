/**
 * @file      memory-G474xE.ld
 * @brief     Memory layout for STM32G474xE
 *                      512Kbytes flash
 *                      96Kbytes SRAM + 32Kbytes CCRAM
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 */


/* Memories definition */
MEMORY
{
    bootloader (rx)     : ORIGIN = 0x08000000,            LENGTH = 48K - 100
    bootloader_info (rx): ORIGIN = 0x08000000+(48K-100),  LENGTH = 100
    app (rx)            : ORIGIN = 0x0800C000,            LENGTH = 512K-48K-2K
    params (rx)         : ORIGIN = 0x08000000+510K,       LENGTH = 2K
    ram    (rwx)        : ORIGIN = 0x20000000,            LENGTH = 96K-256
    app_bl_shared (rwx) : ORIGIN = 0x20000000+(96K-256),  LENGTH = 256

    ccram  (rwx)        : ORIGIN = 0x10000000,            LENGTH = 32K
}
