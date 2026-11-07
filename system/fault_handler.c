/**
 * @file fault_handler.c
 *
 * @brief Captures CPU fault context (stacked registers + fault status
 *        registers) so a debugger can inspect the actual cause of a
 *        HardFault/BusFault/UsageFault/MemManage fault instead of landing in
 *        the anonymous Default_Handler infinite loop.
 *
 * @copyright Copyright © 2026 Purple Monkey Software. All rights reserved.
 *
 */

#if defined(STM32CubeF3)
  #include "stm32f3xx_hal.h"
#elif defined(STM32CubeG4)
  #include "stm32g4xx_hal.h"
#elif defined(STM32CubeL4)
  #include "stm32l4xx_hal.h"
#else
  #error "MCU not supported!"
#endif

typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t afsr;
    uint32_t mmfar;
    uint32_t bfar;
} FaultContext;

// Inspect this in the debugger after a fault to see the real faulting
// PC/LR and the fault status registers.
volatile FaultContext g_fault_context;

static void __attribute__((used)) FaultHandler_C(uint32_t *stacked) {
    g_fault_context.r0    = stacked[0];
    g_fault_context.r1    = stacked[1];
    g_fault_context.r2    = stacked[2];
    g_fault_context.r3    = stacked[3];
    g_fault_context.r12   = stacked[4];
    g_fault_context.lr    = stacked[5];
    g_fault_context.pc    = stacked[6];
    g_fault_context.psr   = stacked[7];
    g_fault_context.cfsr  = SCB->CFSR;
    g_fault_context.hfsr  = SCB->HFSR;
    g_fault_context.dfsr  = SCB->DFSR;
    g_fault_context.afsr  = SCB->AFSR;
    g_fault_context.mmfar = SCB->MMFAR;
    g_fault_context.bfar  = SCB->BFAR;

    while (1) {
        __NOP();
    }
}

// LR bit 2 (EXC_RETURN) tells us whether the exception frame was stacked on
// MSP or PSP; this app never uses PSP, but handle it correctly regardless.
#define FAULT_TRAMPOLINE(name)                 \
void __attribute__((naked)) name(void) {       \
    __asm volatile (                           \
        "tst lr, #4        \n"                 \
        "ite eq            \n"                 \
        "mrseq r0, msp     \n"                 \
        "mrsne r0, psp     \n"                 \
        "b FaultHandler_C  \n"                 \
    );                                         \
}

FAULT_TRAMPOLINE(HardFault_Handler)
FAULT_TRAMPOLINE(MemManage_Handler)
FAULT_TRAMPOLINE(BusFault_Handler)
FAULT_TRAMPOLINE(UsageFault_Handler)
