#include <stdint.h>
#include "adc.h"
#include "gpio.h"
#include "rcc.h"


void ADC1_InitTimer3Trigger(
    ADC_Channel channel,
    ADC_ClockDivider divider
)
{
    volatile uint32_t delay;
    uint32_t sample_shift;

    if ((uint8_t)channel > 9U) return;
    
    RCC_EnableADC1Clock();
    RCC_SetADCPrescaler((uint8_t)divider);

    if ((uint8_t)channel <= 7U)
    {
        RCC_EnableGPIOAClock();
        GPIO_ConfigPin(GPIOA, (uint8_t)channel, GPIO_MODE_ANALOG, GPIO_SPEED_INPUT);
    }
    else
    {
        RCC_EnableGPIOBClock();
        GPIO_ConfigPin(GPIOB, (uint8_t)channel - 8U, GPIO_MODE_ANALOG, GPIO_SPEED_INPUT);
    }

    sample_shift = (uint32_t)channel * 3U;
    ADC1->SMPR2 &= ~(7U << sample_shift);
    ADC1->SMPR2 |=  (7U << sample_shift);

    ADC1->CR1 = 0U;
    ADC1->SQR1 = 0U;
    ADC1->SQR3 = (uint32_t)channel;

    /* DMA=1, EXTSEL=100 (TIM3_TRGO), EXTTRIG=1. */
    ADC1->CR2 = (1U << 8) | (4U << 17) | (1U << 20);
    ADC1->CR2 |= (1U << 0); /* ADON */

    for (delay = 0; delay < 1000U; delay++);

    ADC1->CR2 |= (1U << 3); /* RSTCAL */
    while (ADC1->CR2 & (1U << 3));

    ADC1->CR2 |= (1U << 2); /* CAL */
    while (ADC1->CR2 & (1U << 2));
}

volatile uint32_t *ADC1_GetDataRegisterAddress(void)
{
    return &ADC1->DR;
}
