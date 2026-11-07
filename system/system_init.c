/**
 * @file system_init.c
 * @brief System initializaiton
 *        Dependencies:
 *        - STM32 HAL
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 *
 */

#if defined(STM32CubeF3)
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_uart.h"
#elif defined(STM32CubeG4)
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_uart.h"
#elif defined(STM32CubeL4)
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_uart.h"
#else
#error "MCU not supported!"
#endif

#include "system_init.h"

// CPU_CLOCK_HZ must be defined in the project Makefile.
#ifndef CPU_CLOCK_HZ
  #error "CPU_CLOCK_HZ not defined in Makefile"
#endif  // #ifndef CPU_CLOCK_HZ

#if defined(STM32F303xC)
  #if CPU_CLOCK_HZ > 64000000
    #error "CPU_CLOCK_HZ must be 16000000 or 64000000"
  #endif // CPU_CLOCK_HZ
  #if HSE_VALUE != 8000000
    #error "HSE_VALUE must be 8000000"
  #endif
  #if (CPU_CLOCK_HZ == 16000000)
    #define PLL_MUL       RCC_PLL_MUL2
    #define APB1_CLK_DIV  RCC_HCLK_DIV1
  #else
    #define PLL_MUL       RCC_PLL_MUL8
    #define APB1_CLK_DIV  RCC_HCLK_DIV2
  #endif // CPU_CLK_HZ
#elif defined(STM32G474xx)
// The system assumes an external clock input of 24MHz. Based on this assumption
// the system core clock is calculated by this formula:
//   System Clock = HSE_VALUE / PLL_M * PLL_N / PLL_R
//
//   Example:   168MHz = 24MHz / 3 * 42 / 2

  #if HSE_VALUE != 24000000
    #error "HSE_VALUE must be 24000000"
  #endif

  #define PLL_M    (HSE_VALUE / 8000000) // Divide 24MHz external clock down to multiple of 8MHz

// PLL_N must be between 96MHz and 344MHz.  To satisfy this requirement and still
// achieve our desired CPU clock frequency, we must adjust PLL_R.
  #if (CPU_CLOCK_HZ == 16000000)
    #define PLL_R    RCC_PLLR_DIV8
  #else
    #define PLL_R    RCC_PLLR_DIV2
  #endif // CPU_CLK_HZ
#elif defined(STM32L496xx)
// nothing to do
#elif defined(STM32L432xx)
  #if (CPU_CLOCK_HZ != 64000000)
    #error "CPU_CLOCK_HZ must be 64000000"
  #endif
  #define PLL_M    2
  #define PLL_R    RCC_PLLR_DIV2
#else
  #error "MCU not supported!"
#endif

/**
  * @brief System Clock Configuration.
  * @retval HAL_STATUS_OK for success, else HAL_STATUS_ERROR
  */
static mHalStatus SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

#ifdef STM32G474xx
    // Configure the main internal regulator output voltage.
    // For clock speeds greater than 150MHz, the boost scaling mode must be enabled.
    if (CPU_CLOCK_HZ > 150000000) {
        HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
    }
    else {
        HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    }
#endif
#if defined(STM32L496xx) || defined(STM32L432xx)
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif

    // Initializes the RCC Oscillators according to the specified parameters
    // in the RCC_OscInitTypeDef structure.
#if defined(USE_EXT_CLOCK)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  #ifdef STM32L432xx
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  #else
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  #endif
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  #if defined(STM32F303xC)
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLMUL = PLL_MUL;
  #else
    RCC_OscInitStruct.PLL.PLLM = PLL_M;
    RCC_OscInitStruct.PLL.PLLN = CPU_CLOCK_HZ * PLL_M * PLL_R / HSE_VALUE;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = PLL_R;
  #endif // STM32F303xC
#else
  #if defined(STM32L496xx) // STM32L496 uses MSI for internal clock
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 16;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
  #elif defined(STM32L432xx) // STM32L432 uses HSI for internal clock
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = PLL_M;
    RCC_OscInitStruct.PLL.PLLN = 16;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  #else // STM32F3 and STM32G4 use HSI for internal clock
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  #endif
#endif // #ifdef USE_EXT_CLOCK

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return MHAL_STATUS_ERROR;
    }
    // Initializes the CPU, AHB and APB buses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
#ifdef USE_EXT_CLOCK
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
#else
  #if defined(STM32L496xx) || defined(STM32L432xx)
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  #else
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  #endif
#endif  // USE_EXT_CLOCK
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
#if defined(STM32F303xC)
    RCC_ClkInitStruct.APB1CLKDivider = APB1_CLK_DIV;
#else
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
#endif // STM32F303xC
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    uint32_t flash_latency = FLASH_LATENCY_0;

#if defined(USE_EXT_CLOCK)
    uint32_t hclk = CPU_CLOCK_HZ;
    if (hclk <= 30000000) {
        flash_latency = FLASH_LATENCY_0;
    }
    else if (hclk <= 60000000) {
        flash_latency = FLASH_LATENCY_1;
    }
    else if (hclk <= 90000000) {
        flash_latency = FLASH_LATENCY_2;
    }
  #if defined(STM32G474xx) || defined(STM32L496xx) || defined(STM32L432xx)
    else if (hclk <= 120000000) {
        flash_latency = FLASH_LATENCY_3;
    }
    else {
        flash_latency = FLASH_LATENCY_4;
    }
  #endif  // STM32474xx || STM32L496xx || STM32L432xx
#else
  #if defined(STM32L496xx) || defined(STM32L432xx)
    uint32_t hclk = CPU_CLOCK_HZ;
    if (hclk <= 120000000) {
      flash_latency = FLASH_LATENCY_3;
    }
    else {
      flash_latency = FLASH_LATENCY_4;
    }
  #endif // STM32L496xx
#endif  // #ifdef USE_EXT_CLOCK

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, flash_latency) != HAL_OK)
    {
        return MHAL_STATUS_ERROR;
    }

    // Initialize the peripherals clocks
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_I2C1;
#ifndef STM32L432xx
    PeriphClkInit.PeriphClockSelection += RCC_PERIPHCLK_USART3;
#endif // !STM32L432xx
#if defined(STM32F303xC) || defined(STM32G474xx)
    PeriphClkInit.PeriphClockSelection |= RCC_PERIPHCLK_ADC12;
#elif defined(STM32L496xx) || defined(STM32L432xx)
    PeriphClkInit.PeriphClockSelection |= RCC_PERIPHCLK_ADC;
#else
#error "MCU not supported!"
#endif

#ifdef STM32G474xx
    PeriphClkInit.PeriphClockSelection |= RCC_PERIPHCLK_FDCAN;
    PeriphClkInit.FdcanClockSelection  = RCC_FDCANCLKSOURCE_PCLK1;
#elif STM32L496xx
    PeriphClkInit.PeriphClockSelection |= RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
#endif // STM32G474xx || STM32L496xx
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
#ifndef STM32L432xx
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
#endif // !STM32L432xx
#if defined(STM32F303xC)
    PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK;
    PeriphClkInit.Adc12ClockSelection  = RCC_ADC12PLLCLK_DIV1;
#elif defined(STM32G474xx)
    PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.Adc12ClockSelection  = RCC_ADC12CLKSOURCE_SYSCLK;
#elif defined(STM32L496xx)
    PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.AdcClockSelection    = RCC_ADCCLKSOURCE_SYSCLK;
#elif defined(STM32L432xx)
    PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.AdcClockSelection    = RCC_ADCCLKSOURCE_SYSCLK;
#endif
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        return MHAL_STATUS_ERROR;
    }

    return MHAL_STATUS_OK;
}

mHalStatus system_init(void) {
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    // Enable divide by zero trap
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;

    /* Configure the system clock */
    return SystemClock_Config();
}
