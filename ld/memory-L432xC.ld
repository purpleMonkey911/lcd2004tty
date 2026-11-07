/**
 * @file      memory-L432xC.ld
 * @brief     Memory layout for STM32L432xC
 *                      256Kbytes flash
 *                      48bytes RAM (ram) + 16Kbytes RAM (sram2)
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 */


/* Memories definition */
MEMORY
{
    bootloader (rx)     : ORIGIN = 0x08000000,            LENGTH = 48K - 100
    bootloader_info (rx): ORIGIN = 0x08000000+(48K-100),  LENGTH = 100
    app    (rx)         : ORIGIN = 0x0800C000,            LENGTH = 256K-48K-2K
    params (rx)         : ORIGIN = 0x08000000+254K,       LENGTH = 2K
    ram    (rwx)        : ORIGIN = 0x20000000,            LENGTH = 48K-256
    app_bl_shared (rwx) : ORIGIN = 0x20000000+(256K-256), LENGTH = 256

    sram2  (rwx)        : ORIGIN = 0x10000000,            LENGTH = 16K
}

SECTIONS
{
    _sisram2 = LOADADDR(.sram2);

    .sram2 :
    {
      . = ALIGN(4);
      _ssram2 = .;       /* create a global symbol at sram2 start */
      *(.sram2)
      *(.sram2*)

      . = ALIGN(4);
      _esram2 = .;       /* create a global symbol at sram2 end */
    } >sram2 AT> app
}
