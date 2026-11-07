/**
 * @file    system_stm32.c
 * @brief   CMSIS Cortex-M4 Device Peripheral Access Layer System Source File
 *          MCUs supported:
 *          - STM32F303xC
 *          - STM32G474xx
 *
 *   This file provides two functions and one global variable to be called from
 *   user application:
 *      - SystemInit(): This function is called at startup just after reset and
 *                      before branch to main program. This call is made inside
 *                      the "startup_stm32.S" file.
 *
 *      - SystemCoreClock variable: Contains the core clock (HCLK), it can be used
 *                                  by the user application to setup the SysTick
 *                                  timer or configure other parameters.
 *
 *      - SystemCoreClockUpdate(): Updates the variable SystemCoreClock and must
 *                                 be called whenever the core clock is changed
 *                                 during program execution.
 *
 *   After each device reset the HSI is used as system clock source.
 *   Then SystemInit() function is called, in "startup_stm32.S" file, to
 *   configure the system clock before to branch to main program.
 *
 *   This file configures the system clock as follows:
 *=============================================================================
 *-----------------------------------------------------------------------------
 *        System Clock source                    | HSI
 *-----------------------------------------------------------------------------
 *        SYSCLK(Hz)                             | 16000000 (STM32G474xx HSI)
 *                                               |  8000000 (STM32F303xC HSI)
 *                                               | 16000000 (STM32L496xx HSI)
 *                                               | 16000000 (STM32L432xx HSI)
 *-----------------------------------------------------------------------------
 *        HCLK(Hz)                               | 16000000 (STM32G474xx)
 *                                               |  8000000 (STM32F303xC)
 *                                               | 64000000 (STM32L496xx)
 *                                               | 64000000 (STM32L432xx)
 *-----------------------------------------------------------------------------
 *        AHB Prescaler                          | 1
 *-----------------------------------------------------------------------------
 *        APB1 Prescaler                         | 1
 *-----------------------------------------------------------------------------
 *        APB2 Prescaler                         | 1
 *-----------------------------------------------------------------------------
 *        PLL_M                                  | 1 (STM32G474xx and STM32L432xx only)
 *-----------------------------------------------------------------------------
 *        PLL_N                                  | 16 (STM32G474xx and STM32L432xxonly)
 *-----------------------------------------------------------------------------
 *        PLL_P                                  | 7 (STM32G474xx and STM32L432xx only)
 *-----------------------------------------------------------------------------
 *        PLL_Q                                  | 2 (STM32G474xx and STM32L432xx only)
 *-----------------------------------------------------------------------------
 *        PLL_R                                  | 2 (STM32G474xx and STM32L432xx only)
 *-----------------------------------------------------------------------------
 *        Require 48MHz for RNG                  | Disabled (STM32G474xx only)
 *-----------------------------------------------------------------------------
 *        USB Clock                              | 48000000 (STM32L496xx only)
 *-----------------------------------------------------------------------------
 */

#if defined(STM32F303xC)
  #include "stm32f3xx.h"
#elif defined(STM32G474xx)
  #include "stm32g4xx.h"
#elif defined(STM32L496xx) || defined(STM32L432xx)
  #include "stm32l4xx.h"
#else
#error MCU NOT SUPPORTED
#endif

#ifndef BOOTLOADER
  #include "memory.h"
#endif // !BOOTLOADER

// Default value of external oscillator (Hz)
#if !defined  (HSE_VALUE)
  #if defined(STM32F303xC)
    #define HSE_VALUE      8000000UL
  #elif defined(STM32G474xx)
    #define HSE_VALUE     24000000UL
  #elif defined(STM32L496xx)
    // HSE not yet defined for this MCU
  #elif defined(STM32L432xx)
    #define HSE_VALUE      8000000UL
  #else
    #error MCU NOT SUPPORTED
  #endif
#endif // HSE_VALUE

// Default value of internal oscillator (Hz)
#if !defined  (HSI_VALUE)
  #if defined(STM32F303xC)
    #define HSI_VALUE     8000000UL
  #elif defined(STM32G474xx)
    #define HSI_VALUE    16000000UL
  #elif defined(STM32L496xx)
    #define HSI_VALUE    16000000UL
  #elif defined(STM32L432xx)
    #define HSI_VALUE    16000000UL
  #else
    #error MCU NOT SUPPORTED
  #endif
#endif // HSI_VALUE


// Uncomment the following line if you need to relocate your vector Table in
//     Internal SRAM.
// #define VECT_TAB_SRAM
#if defined(VECT_TAB_SRAM)
#define VECT_TAB_BASE_ADDRESS   SRAM_BASE       /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
#define VECT_TAB_OFFSET         0x00000000UL    /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
#else
#define VECT_TAB_BASE_ADDRESS   FLASH_BASE      /*!< Vector Table base address field.
                                                     This value must be a multiple of 0x200. */
  #if defined (BOOTLOADER) || (STANDALONE)
  #define VECT_TAB_OFFSET         0x00000000UL    /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
  #else
  #define VECT_TAB_OFFSET         APP_OFFSET      /*!< Vector Table base offset field.
                                                     This value must be a multiple of 0x200. */
  #endif // BOOTLOADER
#endif /* VECT_TAB_SRAM */

// The SystemCoreClock variable is updated in three ways:
// 1) by calling CMSIS function SystemCoreClockUpdate()
// 2) by calling HAL API function HAL_RCC_GetHCLKFreq()
// 3) each time HAL_RCC_ClockConfig() is called to configure the system clock frequency
// Note: If you use this function to configure the system clock; then there
//       is no need to call the 2 first functions listed above, since SystemCoreClock
//       variable is updated automatically.
uint32_t SystemCoreClock = HSI_VALUE;

const uint8_t AHBPrescTable[16] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U, 6U, 7U, 8U, 9U};
const uint8_t APBPrescTable[8] =  {0U, 0U, 0U, 0U, 1U, 2U, 3U, 4U};
#if defined(STM32L496xx) || defined(STM32L432xx)
const uint32_t MSIRangeTable[12] = {100000, 200000, 400000, 800000, 1000000, 2000000, \
  4000000, 8000000, 16000000, 24000000, 32000000, 48000000};
#endif

/**
  * @brief  Setup the microcontroller system.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
    // FPU settings
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << (10*2))|(3UL << (11*2)));  /* set CP10 and CP11 Full Access */
#endif

    // Configure the Vector Table location add offset address
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif
}

/**
  * @brief  Update SystemCoreClock variable according to Clock Register Values.
  *         The SystemCoreClock variable contains the core clock (HCLK), it can
  *         be used by the user application to setup the SysTick timer or configure
  *         other parameters.
  *
  * @note   Each time the core clock (HCLK) changes, this function must be called
  *         to update SystemCoreClock variable value. Otherwise, any configuration
  *         based on this variable will be incorrect.
  *
  * @note   - The system frequency computed by this function is not the real
  *           frequency in the chip. It is calculated based on the predefined
  *           constant and the selected clock source:
  *
  *           - If SYSCLK source is HSI, SystemCoreClock will contain the HSI_VALUE(**)
  *
  *           - If SYSCLK source is HSE, SystemCoreClock will contain the HSE_VALUE(***)
  *
  *           - If SYSCLK source is PLL, SystemCoreClock will contain the HSE_VALUE(***)
  *             or HSI_VALUE(*) or MSI_VALUE(*) multiplied/divided by the PLL factors.
  *
  *         (**) HSI_VALUE is a constant defined in stm32g4xx_hal.h file (default value
  *              16 MHz) but the real value may vary depending on the variations
  *              in voltage and temperature.
  *
  *         (***) HSE_VALUE is a constant defined in stm32g4xx_hal.h file (default value
  *              24 MHz), user has to ensure that HSE_VALUE is same as the real
  *              frequency of the crystal used. Otherwise, this function may
  *              have wrong result.
  *
  *         - The result of this function could be not correct when using fractional
  *           value for HSE crystal.
  */
void SystemCoreClockUpdate(void)
{
    uint32_t tmp = 0UL, pllsource = 0UL;
#if defined(STM32F303xC)
    uint32_t pllmull = 0UL, predivfactor = 0UL;
#elif defined(STM32G474xx)
    uint32_t pllvco = 0UL, pllr = 0UL, pllm = 0UL;
#elif defined(STM32L496xx) || defined(STM32L432xx)
    uint32_t msirange = 0UL, pllvco = 0UL, pllr = 0UL, pllm = 0UL;
#else
#error  MCU NOT SUPPORTED
#endif

#if defined(STM32L496xx) || defined(STM32L432xx)
    /* Get MSI Range frequency--------------------------------------------------*/
    if((RCC->CR & RCC_CR_MSIRGSEL) == RESET) {
        /* MSISRANGE from RCC_CSR applies */
        msirange = (RCC->CSR & RCC_CSR_MSISRANGE) >> 8;
    } else {
        /* MSIRANGE from RCC_CR applies */
        msirange = (RCC->CR & RCC_CR_MSIRANGE) >> 4;
    }
    /*MSI frequency range in HZ*/
    msirange = MSIRangeTable[msirange];
#endif

    // Get SYSCLK source
    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp) {

#if defined(STM32L496xx) || defined(STM32L432xx)
    case RCC_CFGR_SWS_MSI:
        SystemCoreClock = msirange;
        break;
#endif
    case RCC_CFGR_SWS_HSI:  // HSI used as system clock
        SystemCoreClock = HSI_VALUE;
        break;
    case RCC_CFGR_SWS_HSE:  // HSE used as system clock
        SystemCoreClock = HSE_VALUE;
        break;
    case RCC_CFGR_SWS_PLL:  // PLL used as system clock
#if defined(STM32F303xC)
        // Get PLL clock source and multiplication factor
        pllmull = RCC->CFGR & RCC_CFGR_PLLMUL;
        pllsource = RCC->CFGR & RCC_CFGR_PLLSRC;
        pllmull = ( pllmull >> 18) + 2;

  #if defined (STM32F302xE) || defined (STM32F303xE) || defined (STM32F398xx)
        predivfactor = (RCC->CFGR2 & RCC_CFGR2_PREDIV) + 1;
        if (pllsource == RCC_CFGR_PLLSRC_HSE_PREDIV)
        {
            // HSE oscillator clock selected as PREDIV1 clock entry
            SystemCoreClock = (HSE_VALUE / predivfactor) * pllmull;
        }
        else
        {
            // HSI oscillator clock selected as PREDIV1 clock entry
            SystemCoreClock = (HSI_VALUE / predivfactor) * pllmull;
        }
  #else
        if (pllsource == RCC_CFGR_PLLSRC_HSI_DIV2)
        {
            // HSI oscillator clock divided by 2 selected as PLL clock entry
            SystemCoreClock = (HSI_VALUE >> 1) * pllmull;
        }
        else
        {
            predivfactor = (RCC->CFGR2 & RCC_CFGR2_PREDIV) + 1;
            // HSE oscillator clock selected as PREDIV1 clock entry
            SystemCoreClock = (HSE_VALUE / predivfactor) * pllmull;
        }
  #endif /* STM32F302xE || STM32F303xE || STM32F398xx */
#elif defined(STM32G474xx) || defined(STM32L496xx) || defined(STM32L432xx)
        // PLL_VCO = (HSE_VALUE or HSI_VALUE / PLLM) * PLLN
        // SYSCLK = PLL_VCO / PLLR
        pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC);
        pllm = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> 4) + 1U ;
        switch (pllsource) {
        case RCC_PLLCFGR_PLLSRC_HSI: // HSI used as PLL clock source
            pllvco = (HSI_VALUE / pllm);
            break;
        case RCC_PLLCFGR_PLLSRC_HSE:
            pllvco = (HSE_VALUE / pllm);
            break;
#if defined(STM32L496xx) || defined(STM32L432xx)
        case RCC_PLLCFGR_PLLSRC_MSI:
            pllvco = (msirange / pllm);
            break;
#endif
        default:
            break;
        }
        pllvco = pllvco * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 8);
        pllr = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLR) >> 25) + 1U) * 2U;
        SystemCoreClock = pllvco/pllr;
#else
#error MCU NOT SUPPORTED
#endif
         break;
    default: /* HSI used as system clock */
        SystemCoreClock = HSI_VALUE;
        break;
    }

    // Compute HCLK clock frequency
    // Get HCLK prescaler
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    // HCLK clock frequency
    SystemCoreClock >>= tmp;
}
