#include <stdint.h>
#include "lib/adc.h"
#include "lib/dma.h"
#include "lib/timer.h"
#include "lib/uart.h"

#define ADC_BUFFER_SIZE 100U
#define ADC_HALF_SIZE    50U

static volatile uint16_t adc_buffer[ADC_BUFFER_SIZE];

static void SendADCBlock(uint8_t start, uint8_t end)
{
    uint8_t index;

    for (index = start; index < end; index++)
    {
        SendNumberInterrupt_UART(adc_buffer[index]);
        SendTextInterrupt_UART("\n\r");
    }
}

int main(void)
{
    Config_uart();

    /* 8 MHz / (79+1) / (999+1) = 100 Hz. */
    TIM3_ConfigUpdateTrigger(79U, 999U);

    ADC1_InitTimer3Trigger(
        ADC_CHANNEL_5_PA5,
        ADC_CLOCK_DIV2
    );

    DMA1_Channel1_Init(
        ADC1_GetDataRegisterAddress(),
        adc_buffer,
        ADC_BUFFER_SIZE
    );

    /* Bat Timer cuoi cung de bat dau lay mau. */
    TIM3_Start();

    while (1)
    {
        if (DMA1_Channel1_TakeHalfTransfer())
        {
            SendADCBlock(0U, ADC_HALF_SIZE);
        }

        if (DMA1_Channel1_TakeTransferComplete())
        {
            SendADCBlock(ADC_HALF_SIZE, ADC_BUFFER_SIZE);
        }
    }
}
