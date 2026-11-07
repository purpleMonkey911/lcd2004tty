/**
 * @file      standalone-memory-G4.ld
 * @brief     Memory layout for STM32G474xE for standalone applications
 *                      512Kbytes flash
 *                      128Kbytes RAM
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 */


/* Memories definition */
MEMORY
{
    app (rx)            : ORIGIN = 0x08000000,            LENGTH = 512K
    ram (rwx)           : ORIGIN = 0x20000000,            LENGTH = 96K
    ccram  (rwx)        : ORIGIN = 0x10000000,            LENGTH = 32K
}
