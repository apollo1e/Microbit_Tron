# ################################################################################
# # micro T-Kernel 3.00.05  makefile
# ################################################################################

# GCC := arm-none-eabi-gcc
# AS := arm-none-eabi-gcc3123
# LINK := arm-none-eabi-gcc

# CFLAGS := -mcpu=cortex-m4 -mthumb -ffreestanding \
#     -std=gnu11 \
#     -O0 -g3 \
#     -MMD -MP \
#     -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# ASFLAGS := -mcpu=cortex-m4 -mthumb -ffreestanding \
#     -x assembler-with-cpp \
#     -O0 -g3 \
#     -MMD -MP \
#     -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# # LFLAGS := -mcpu=cortex-m4 -mthumb -ffreestanding \
# #     -nostartfiles \
# #     -O0 -g3 \
# #     -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# LFLAGS := -mcpu=cortex-m4 -mthumb -ffreestanding -nostartfiles -O0 -g3 \
#   -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
#   -lc -lnosys





# LNKFILE := "../etc/linker/microbit/tkernel_map.ld"

# include mtkernel_3/lib/libtm/sysdepend/microbit/subdir.mk
# include mtkernel_3/lib/libtm/sysdepend/no_device/subdir.mk
# include mtkernel_3/lib/libtk/sysdepend/cpu/nrf5/subdir.mk
# include mtkernel_3/lib/libtk/sysdepend/cpu/core/armv7m/subdir.mk
# include mtkernel_3/kernel/sysdepend/microbit/subdir.mk
# include mtkernel_3/kernel/sysdepend/cpu/nrf5/subdir.mk
# include mtkernel_3/kernel/sysdepend/cpu/core/armv7m/subdir.mk

# # include mtkernel_3/app_sample2/subdir.mk


################################################################################
# micro T-Kernel 3.x  – micro:bit v2 (nRF52833) toolchain config
################################################################################

# Tools
GCC  := arm-none-eabi-gcc
AS   := arm-none-eabi-gcc
# Add flags directly to LINK so the final link uses hard-float too
LINK := arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
        -nostartfiles -specs=nosys.specs -Wl,--gc-sections

# Common flags
CFLAGS := -mcpu=cortex-m4 -mthumb -ffreestanding \
    -std=gnu11 \
    -O0 -g3 \
    -MMD -MP \
    -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
    -D_MICROBIT_

ASFLAGS := -mcpu=cortex-m4 -mthumb -ffreestanding \
    -x assembler-with-cpp

# Include paths already used by your project
CFLAGS += \
    -I"../include" \
    -I"../config" \
    -I"../kernel/knlinc" \
    -I"mtkernel_3/lib/radio"

ASFLAGS += \
    -I"../include" \
    -I"../config" \
    -I"../kernel/knlinc"

# ---------- Nordic / CMSIS (NEW) ----------
NRFX_DIR  := ../third_party/nrfx
CMSIS_DIR := ../third_party/CMSIS_5/CMSIS/Core/Include

# Headers for nrf.h and CMSIS core_cm4.h
CFLAGS  += -I$(NRFX_DIR) -I$(NRFX_DIR)/mdk -I$(NRFX_DIR)/hal -I$(CMSIS_DIR) \
           -DNRF52833_XXAA -DNRF52
ASFLAGS += -I$(NRFX_DIR) -I$(NRFX_DIR)/mdk -I$(NRFX_DIR)/hal -I$(CMSIS_DIR) \
           -DNRF52833_XXAA -DNRF52
# -----------------------------------------

CFLAGS  += -ffunction-sections -fdata-sections
ASFLAGS += -ffunction-sections -fdata-sections


# Linker script
LNKFILE := "../etc/linker/microbit/tkernel_map.ld"

# (Your existing object/subdir includes)
include mtkernel_3/lib/libtm/sysdepend/microbit/subdir.mk
include mtkernel_3/lib/libtm/sysdepend/no_device/subdir.mk
include mtkernel_3/lib/libtk/sysdepend/cpu/nrf5/subdir.mk
include mtkernel_3/lib/libtk/sysdepend/cpu/core/armv7m/subdir.mk
include mtkernel_3/kernel/sysdepend/microbit/subdir.mk
include mtkernel_3/kernel/sysdepend/cpu/nrf5/subdir.mk
include mtkernel_3/kernel/sysdepend/cpu/core/armv7m/subdir.mk
# include mtkernel_3/app_sample2/subdir.mk
