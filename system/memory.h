/**
 * @file memory.h
 *
 * @brief Memory map
 *
 *        MCUs supported:
 *        - STM32F3xx
 *        - STM32G4xx
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 *
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stdint.h>

// Flash memory
#ifdef STM32F303xC
  #define FLASH_SZ  0x20000  // 128K
  #define RAM_SZ    0x8000   // 32K
#elif defined STM32G474xx
  #define FLASH_SZ  0x80000  // 512K
  #define RAM_SZ    0x18000  // 96K
#elif defined STM32L496xx
  #define FLASH_SZ  0x100000 // 1024K
  #define RAM_SZ    0x40000  // 256K
#elif defined STM32L432xx
  #define FLASH_SZ  0x40000  // 256K
  #define RAM_SZ    0xC000   // 48K
#else
  #error MCU not supported!
#endif

#define BOOTLOADER_NAME_LEN_MAX         32 // bytes
#define BOOTLOADER_BRANCH_LEN_MAX       16 // bytes
#define BOOTLOADER_COMMIT_HASH_LEN_MAX  16 // bytes
#define BOOTLOADER_COMMIT_DATE_LEN_MAX  32 // bytes

typedef struct {
    uint8_t name[BOOTLOADER_NAME_LEN_MAX];
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    uint8_t pad;
    uint8_t branch[BOOTLOADER_BRANCH_LEN_MAX];
    uint8_t commit_hash[BOOTLOADER_COMMIT_HASH_LEN_MAX];
    uint8_t commit_date[BOOTLOADER_COMMIT_DATE_LEN_MAX];
} BootloaderBuildInfo;

typedef struct {
    uint64_t crc64;
    uint32_t size;
    uint32_t pad;
} AppInfo;

#define FLASH_PAGE_SZ              2048  // bytes
#define BOOTLOADER_ADDR            0x08000000
#define APP_ADDR                   0x0800C000
#define APP_OFFSET                 (APP_ADDR - BOOTLOADER_ADDR)
#define BOOTLOADER_BUILD_INFO_ADDR (APP_ADDR - sizeof(BootloaderBuildInfo))
#define PARAMS_SZ                  2048  // bytes
#define PARAMS_ADDR                (BOOTLOADER_ADDR + FLASH_SZ - PARAMS_SZ)
#define PARAMS_ADDR_END            (BOOTLOADER_ADDR + FLASH_SZ - 1)
#define APP_ADDR_END               (PARAMS_ADDR - 1)
#define APP_INFO_ADDR              (PARAMS_ADDR - sizeof(AppInfo))

#define RAM_ADDR        0x20000000
#define RAM_SHARED_SZ   256    // bytes, shared RAM between bootloader and App
#define RAM_SHARED_ADDR (RAM_ADDR + RAM_SZ - RAM_SHARED_SZ)

#endif  //  __MEMORY_H__
