#include <stdint.h>
#include <stdio.h>

#include "lib/uart.h"
#include "lib/rcc.h"
#include "lib/gpio.h"
#include "lib/dma.h"

#define PIN_BUTTON 4

static char message[32];

void delay(uint8_t time)
{
    for (int i = 0; i < time; i++)
    {
        for (int j = 0; j < 1000; j++)
        {
        }
    }
}

int main(void)
{
    uint8_t new_state;
    uint8_t old_state;
    uint8_t count = 0U;
    int length;

    dma_config uart_dma =
    {
        .direction          = DMA_Direc_MemoryToPeriph,
        .periph_size        = DMA_SIZE_8Bit,
        .memory_size        = DMA_SIZE_8Bit,
        .mode               = DMA_MODE_NORMAL,
        .priority           = DMA_Priority_HIGH,

        .minc               = 1U,
        .pinc               = 0U,

        .half_interrupt     = 0U,
        .complete_interrupt = 0U,
        .error_interrupt    = 0U,
        .memory_to_memory   = 0U
    };

    Config_uart();

    GPIO_ConfigPin(GPIOA, PIN_BUTTON, GPIO_MODE_INPUT_PULLUP, GPIO_SPEED_INPUT);

    DMA_Init(
        DMA1_Channel4,
        &USART1->DR,
        &uart_dma
    );

    DMA_ClearAllFlag(4);

    old_state = GPIO_ReadPin(GPIOA, PIN_BUTTON);

    while (1) 
    {
        new_state = GPIO_ReadPin(GPIOA, PIN_BUTTON);

        if (old_state == 1U && new_state == 0U)
        {
            delay(20);
            if (DMA_GetRemainingData(DMA1_Channel4) == 0U)
            {
                count++;
                length = snprintf(message, sizeof(message), "DMDTMT02 - Nhom 14:Button: %u\r\n", (unsigned int)count);
                if (length > 0 &&length < (int)sizeof(message))
                {
                    DMA_ClearAllFlag(4);
                    DMA_Start( DMA1_Channel4, message, (uint16_t)length);
                }
            }
        
        }

        old_state = new_state;
    }
}