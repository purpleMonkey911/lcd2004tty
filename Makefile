################################################################################
# Makefile for lcd2004tty project
#
# Hardware: Nucleo STM32G474RE
# STM32 Family: G4
# MCU: STM32G474RE
# Peripherals:
#   Timer
#   UART
#
# Copyright © 2026 Purple Monkey Software. All rights reserved.
################################################################################

# Project
PROJECT := lcd2004tty

# select the proper MCU
# uncomment one of the choices below:
#STM32_FAMILY := F3
STM32_FAMILY := G4

#STM32_CHIP   := F303xB
STM32_CHIP   := G474xE
#STM32_CHIP   := L432xC
#STM32_CHIP   := L496xG

#STM32_MCU    := STM32F303xC # use this for STM32F303xB
STM32_MCU    := STM32G474xx
#STM32_MCU    := STM32L432xx
#STM32_MCU    := STM32L496xx

# We support the following CPU clock freqs: 16MHz, 64MHz, and 168MHz.
# NOTE: For STM32F303, we only support 16MHz and 64MHz
# NOTE: For STM32L432, we only support 64MHz
# NOTE: For STM32L496, we only support 64MHz (TODO: support other freq?)
# Uncomment one of the choices below:
#CPU_CLOCK_HZ := 16000000
#CPU_CLOCK_HZ := 64000000
CPU_CLOCK_HZ := 168000000

# standalone application (i.e. does not need a bootloader) (enable or disable)
STANDALONE_ENABLE = enable

###############################################################################
# ARM Toolchain
###############################################################################
TOOLCHAIN := arm-none-eabi
CC := $(TOOLCHAIN)-gcc
CXX := $(TOOLCHAIN)-g++
CPP := $(TOOLCHAIN)-cpp
AS := $(TOOLCHAIN)-as
OBJCOPY := $(TOOLCHAIN)-objcopy
OBJDUMP := $(TOOLCHAIN)-objdump
SIZE := $(TOOLCHAIN)-size

###############################################################################
# Python
###############################################################################
PYTHON := python3

###############################################################################
# other commands
###############################################################################
RM := rm -rf

###############################################################################
# build directory directory structure
###############################################################################
BUILD_DIR   = ./build

###############################################################################
# STM32 compilation variables
###############################################################################
STM32_HAL_DIR    := STM32Cube$(STM32_FAMILY)
STM32_INCLUDES    = -I./system
STM32_INCLUDES   += -I./$(STM32_HAL_DIR)/Drivers/CMSIS/Include
STM32_INCLUDES   += -I./$(STM32_HAL_DIR)/Drivers/CMSIS/Device/ST/STM32$(STM32_FAMILY)xx/Include
STM32_INCLUDES   += -I./$(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Inc
ifeq ($(strip $(STM32_FAMILY)),F3)
STM32_HAL_SRCS   = $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_cortex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_dma.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_can.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_flash.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_flash_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_gpio.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_i2c.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_i2c_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_pwr_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_rcc.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_rcc_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_tim.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_tim_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_uart.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32f3xx_hal_uart_ex.c
else ifeq ($(strip $(STM32_FAMILY)),G4)
STM32_HAL_SRCS   = $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_cortex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_dma.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_fdcan.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_flash.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_flash_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_gpio.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_i2c.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_i2c_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_pwr_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_rcc.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_rcc_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_tim.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_tim_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_uart.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32g4xx_hal_uart_ex.c
else ifeq ($(strip $(STM32_FAMILY)),L4)
STM32_HAL_SRCS   = $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_can.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_flash.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_tim.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_uart.c
STM32_HAL_SRCS   += $(STM32_HAL_DIR)/Drivers/STM32$(STM32_FAMILY)xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c
else
$(error Unsupported STM32_FAMILY: $(STM32_FAMILY))
endif

STM32_HAL_OBJLIST = $(addprefix $(BUILD_DIR)/, $(STM32_HAL_SRCS:.c=.o))
# STM32 HAL compiler flags
STM32_HAL_FLAGS  = -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -D$(STM32_MCU)
STM32_HAL_FLAGS += -DDEBUG -c $(STM32_INCLUDES) -O2
STM32_HAL_FLAGS += -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD
STM32_HAL_FLAGS += -MP --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb

###############################################################################
# STM32 SYSTEM includes
###############################################################################
SYS_INCLUDES  = -I./system

###############################################################################
# monkeyHAL variables
###############################################################################
MHAL_INCLUDES  = -I./monkeyHAL/common
MHAL_INCLUDES += -I./monkeyHAL/stm32

###############################################################################
# project compilation variables
###############################################################################
PROJECT_INCLUDES  = -I./src
PROJECT_INCLUDES += -I./dev

# C source files
C_SRCS    = $(shell find src -name "*.c")
C_SRCS   += $(shell find dev -name "*.c")
C_SRCS   += $(shell find monkeyHAL/common -maxdepth 1 -name "*.c")
C_SRCS   += $(shell find monkeyHAL/stm32 -maxdepth 1 -name "*.c")
C_SRCS   += $(shell find system -name "*.c")
C_OBJLIST = $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))

# C compiler flags
C_FLAGS  = -mcpu=cortex-m4 -std=gnu11 -g3 -DUSE_HAL_DRIVER -D$(STM32_MCU) -DCPU_CLOCK_HZ=$(CPU_CLOCK_HZ)
ifneq ($(STM32_FAMILY),L4)
C_FLAGS += -DUSE_EXT_CLOCK
endif
ifeq ($(strip $(DEBUG_BUILD)),enable)
C_FLAGS += -DDEBUG
endif # DEBUG_BUILD
C_FLAGS += -D$(STM32_HAL_DIR) -c $(MHAL_INCLUDES) $(PROJECT_INCLUDES) $(SYS_INCLUDES) $(STM32_INCLUDES)
C_FLAGS += -O2 -ffunction-sections 
C_FLAGS += -fdata-sections -Wall -fstack-usage -MMD -MP --specs=nano.specs 
C_FLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
ifeq ($(strip $(STANDALONE_ENABLE)),enable)
C_FLAGS += -DSTANDALONE
endif # STANDLONE_ENABLE

# ASM source files and objects
ifeq ($(STM32_CHIP), F303xB)
ASM_SRCS    += system/startup_stm32f303xc.s
else ifeq ($(STM32_CHIP), G474xE)
ASM_SRCS    += system/startup_stm32g474xx.s
else ifeq ($(STM32_CHIP), L496xG)
ASM_SRCS    += system/startup_stm32l496xx.s
else ifeq ($(STM32_CHIP), L432xC)
ASM_SRCS    += system/startup_stm32l432xx.s
else
$(error Unsupported STM32_CHIP: $(STM32_CHIP))
endif
ASM_FLAGS    = -mcpu=cortex-m4 -g3 -D$(STM32_MCU) -c -MMD -MP
ASM_FLAGS   += --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
ASM_OBJLIST += $(addprefix $(BUILD_DIR)/, $(ASM_SRCS:.s=.o))

###############################################################################
# libraries
###############################################################################
LIBS :=

###############################################################################
# linker file
###############################################################################
LD_DIR    = ./ld
ifeq ($(strip $(STANDALONE_ENABLE)),enable)
LD_FILE   = standalone-app-$(STM32_CHIP).ld
else ifeq ($(strip $(BOOTLOADER_BUILD)),enable)
LD_FILE   = bootloader-$(STM32_CHIP).ld
else
LD_FILE   = app-$(STM32_CHIP).ld
endif # STANDALONE_ENABLE
LD        = $(LD_DIR)/$(LD_FILE)
LD_FLAGS  = -mcpu=cortex-m4 -x c -T$(LD) --specs=nosys.specs -Wl,-Map="$(BUILD_DIR)/$(PROJECT).map"
LD_FLAGS += -Wl,--gc-sections -static -L$(LD_DIR) --specs=nano.specs -mfpu=fpv4-sp-d16
LD_FLAGS += -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
LD_FLAGS += -Wl,--no-warn-rwx-segments

###############################################################################
# output file(s) variables
###############################################################################
OUTPUT_ELF  := $(BUILD_DIR)/$(PROJECT).elf
OUTPUT_BIN  := $(BUILD_DIR)/$(PROJECT).bin
OUTPUT_LIST := $(BUILD_DIR)/$(PROJECT).list

###############################################################################
# Targets
###############################################################################
all: print-compiler-version $(OUTPUT_ELF) size-output $(OUTPUT_LIST) $(OUTPUT_BIN)

print-compiler-version:
	@echo "Building $(PROJECT) with compiler:"
	@echo ""
	@$(CC) --version
	@echo ""

# Object files are written under $(BUILD_DIR), mirroring the source tree.
# Each rule creates its own output directory, so no separate build-dir step
# is needed (and parallel builds with -j are safe).  Static pattern rules are
# used because the STM32 HAL and project sources need different flags.

# compile STM32 HAL
$(STM32_HAL_OBJLIST): $(BUILD_DIR)/%.o : %.c
	@echo compiling $<
	@mkdir -p $(dir $@)
	@$(CC) $< $(STM32_HAL_FLAGS) -o $@

# compile project source code
$(C_OBJLIST): $(BUILD_DIR)/%.o : %.c
	@echo compiling $<
	@mkdir -p $(dir $@)
	@$(CC) $< $(C_FLAGS) -o $@

# compile project assembly code
$(ASM_OBJLIST): $(BUILD_DIR)/%.o : %.s
	@echo compiling $<
	@mkdir -p $(dir $@)
	@$(CC) $< $(ASM_FLAGS) -o $@

# pull in auto-generated header dependencies from -MMD -MP
# (missing on first build is OK)
-include $(STM32_HAL_OBJLIST:.o=.d) $(C_OBJLIST:.o=.d) $(ASM_OBJLIST:.o=.d)

# link all object code to create elf file and map file
# (only relinks when an object file or the linker script changes)
$(OUTPUT_ELF): $(STM32_HAL_OBJLIST) $(C_OBJLIST) $(ASM_OBJLIST) $(LD)
	@echo ' '
	@echo "Linking ($(LD))..."
	@echo ' '
	@$(CC) $(STM32_HAL_OBJLIST) $(C_OBJLIST) $(ASM_OBJLIST) $(LIBS) $(LD_FLAGS) -o $@
	@echo 'Finished building target: $@'
	@echo ' '

# display elf file stats
size-output: $(OUTPUT_ELF)
	@$(SIZE)  $(OUTPUT_ELF)
	@echo ' '

# create list file from elf file
$(OUTPUT_LIST): $(OUTPUT_ELF)
	@$(OBJDUMP) -h -S $< > $@
	@echo 'Finished building: $@'
	@echo ' '

# create bin file from elf file
$(OUTPUT_BIN): $(OUTPUT_ELF)
	@$(OBJCOPY) -O binary $< $@
	@echo 'Finished building: $@'
	@echo ' '

clean:
	-$(RM) $(BUILD_DIR)
	-@echo ' '

.PHONY: all print-compiler-version size-output clean


