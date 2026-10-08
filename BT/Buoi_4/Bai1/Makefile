CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

TARGET = firmware
LDSCRIPT = STM32F103C8Tx_FLASH.ld

FREERTOS_DIR = FreeRTOS-Kernel
PORT_DIR = $(FREERTOS_DIR)/portable/GCC/ARM_CM3

SRCS = main.c \
       startup.c \
       $(FREERTOS_DIR)/tasks.c \
       $(FREERTOS_DIR)/list.c \
       $(FREERTOS_DIR)/queue.c \
       $(PORT_DIR)/port.c \
       $(FREERTOS_DIR)/portable/MemMang/heap_4.c

HEADERS = FreeRTOSConfig.h \
          $(wildcard $(FREERTOS_DIR)/include/*.h) \
          $(wildcard $(PORT_DIR)/*.h)

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
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
         -I../../CMSIS \
         -I$(FREERTOS_DIR)/include \
         -I$(PORT_DIR) \
         -DSTM32F10X_MD

LDFLAGS = -T $(LDSCRIPT) \
          -nostartfiles \
          --specs=nano.specs \
          --specs=nosys.specs \
          -Wl,--gc-sections \
          -Wl,-Map=$(TARGET).map

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