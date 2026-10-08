CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

TARGET = firmware

SRCS = main.c \
       startup.c \
       lib/rcc.c \
       lib/gpio.c \
       lib/uart.c \
       lib/dma.c

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
         -Wall \
         -O0 \
         -std=c11 \
         -ffreestanding \
         -fno-builtin \
         -ffunction-sections \
         -fdata-sections \
         -I../../CMSIS \
         -DSTM32F10X_MD

LDFLAGS = -T STM32F103C8Tx_FLASH.ld \
          -nostartfiles \
          --specs=nano.specs \
          --specs=nosys.specs \
          -Wl,--gc-sections \
          -Wl,-Map=$(TARGET).map

all: $(TARGET).bin

$(TARGET).elf: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

clean:
	rm -f $(TARGET).elf $(TARGET).bin $(TARGET).map

.PHONY: all flash clean