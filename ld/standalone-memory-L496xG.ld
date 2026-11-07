/**
 * @file      standalone-memory-L496xG.ld
 * @brief     Memory layout for STM32L496xG for standalone applications
 *                      1024Kbytes flash
 *                      256Kbytes RAM (ram) + 64Kbytes RAM (sram2)
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 */


/* Memories definition */
MEMORY
{
    app    (rx)         : ORIGIN = 0x08000000,            LENGTH = 1024K
    ram    (rwx)        : ORIGIN = 0x20000000,            LENGTH = 256K

    sram2  (rwx)        : ORIGIN = 0x10000000,            LENGTH = 64K
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
