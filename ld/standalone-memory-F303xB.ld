/**
 * @file      standalone-memory-F303xB.ld
 * @brief     Memory layout for STM32F303xB for standalone applications
 *                      128Kbytes flash
 *                      32Kbytes RAM (SRAM) + 8Kbytes RAM (CCRAM)
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 */


/* Memories definition */
MEMORY
{
    app    (rx)         : ORIGIN = 0x08000000,            LENGTH = 128K
    ram    (rwx)        : ORIGIN = 0x20000000,            LENGTH = 32K

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
