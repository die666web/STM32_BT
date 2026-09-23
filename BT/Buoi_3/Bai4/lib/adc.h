#ifndef ADC_H
#define ADC_H

#include <stdint.h>


#define ADC1_BASE 0x40012400UL

typedef struct
{
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
} ADC_TypeDef;

#define ADC1 ((ADC_TypeDef *)ADC1_BASE)


typedef enum
{
    ADC_CHANNEL_0_PA0 = 0,
    ADC_CHANNEL_1_PA1 = 1,
    ADC_CHANNEL_2_PA2 = 2,
    ADC_CHANNEL_3_PA3 = 3,
    ADC_CHANNEL_4_PA4 = 4,
    ADC_CHANNEL_5_PA5 = 5,
    ADC_CHANNEL_6_PA6 = 6,
    ADC_CHANNEL_7_PA7 = 7,
    ADC_CHANNEL_8_PB0 = 8,
    ADC_CHANNEL_9_PB1 = 9
} ADC_Channel;

typedef enum
{
    ADC_CLOCK_DIV2 = 0,
    ADC_CLOCK_DIV4 = 1,
    ADC_CLOCK_DIV6 = 2,
    ADC_CLOCK_DIV8 = 3
} ADC_ClockDivider;

void ADC1_InitTimer3Trigger(
    ADC_Channel channel,
    ADC_ClockDivider divider
);

volatile uint32_t *ADC1_GetDataRegisterAddress(void);

#endif
