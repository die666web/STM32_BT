CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

TARGET = firmware
LDSCRIPT = STM32F103C8Tx_FLASH.ld

FREERTOS = FreeRTOS-Kernel
PORT = $(FREERTOS)/portable/GCC/ARM_CM3

SRCS = main.c \
       startup.c \
       $(wildcard lib/*.c) \
       $(FREERTOS)/tasks.c \
       $(FREERTOS)/list.c \
       $(FREERTOS)/queue.c \
       $(FREERTOS)/timers.c \
       $(FREERTOS)/event_groups.c \
       $(FREERTOS)/stream_buffer.c \
       $(PORT)/port.c \
       $(FREERTOS)/portable/MemMang/heap_4.c

HEADERS = $(wildcard *.h lib/*.h) \
          $(wildcard $(FREERTOS)/include/*.h) \
          $(wildcard $(PORT)/*.h)

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
         -mfloat-abi=soft \
         -Wall \
         -Wextra \
         -O0 \
         -g3 \
         -std=c11 \
         -ffreestanding \
         -fno-builtin \
         -ffunction-sections \
         -fdata-sections \
         -I. \
         -Ilib \
         -I../../CMSIS \
         -I$(FREERTOS)/include \
         -I$(PORT) \
         -DSTM32F10X_MD

LDFLAGS = -T$(LDSCRIPT) \
          -nostartfiles \
          --specs=nano.specs \
          --specs=nosys.specs \
          -Wl,--gc-sections \
          -Wl,-Map=$(TARGET).map

.DEFAULT_GOAL := all
.DELETE_ON_ERROR:

all: $(TARGET).bin

$(TARGET).elf: $(SRCS) $(HEADERS) $(LDSCRIPT) Makefile
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).map

.PHONY: all flash clean