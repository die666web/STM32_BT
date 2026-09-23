#include <stdint.h>
#include "timer.h"
#include "gpio.h"
#include "rcc.h"

#define TIM2_BASE 0x40000000UL
#define TIM3_BASE 0x40000400UL

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMCR;
    volatile uint32_t DIER;
    volatile uint32_t SR;
    volatile uint32_t EGR;
    volatile uint32_t CCMR1;
    volatile uint32_t CCMR2;
    volatile uint32_t CCER;
    volatile uint32_t CNT;
    volatile uint32_t PSC;
    volatile uint32_t ARR;
    volatile uint32_t RESERVED;
    volatile uint32_t CCR1;
    volatile uint32_t CCR2;
    volatile uint32_t CCR3;
    volatile uint32_t CCR4;
} TIM_TypeDef;

#define TIM2 ((TIM_TypeDef *)TIM2_BASE)
#define TIM3 ((TIM_TypeDef *)TIM3_BASE)

void Config_Timer(uint8_t channel, uint16_t duty_cycle)
{
    uint8_t pin;

    if (channel < 1U || channel > 4U)
    {
        return;
    }

    if (duty_cycle > PWM_PERIOD)
    {
        duty_cycle = PWM_PERIOD;
    }

    RCC_EnableGPIOAClock();
    RCC_EnableTIM2Clock();

    pin = channel - 1U;
    GPIO_ConfigPin(GPIOA, pin, GPIO_MODE_AF_PP, GPIO_SPEED_50MHZ);

    TIM2->PSC = 7U;
    TIM2->ARR = PWM_PERIOD - 1U;

    switch (channel)
    {
        case 1:
            TIM2->CCMR1 &= ~(0xFFU << 0);
            TIM2->CCMR1 |= (6U << 4) | (1U << 3);
            TIM2->CCER |= (1U << 0);
            TIM2->CCR1 = duty_cycle;
            break;
        case 2:
            TIM2->CCMR1 &= ~(0xFFU << 8);
            TIM2->CCMR1 |= (6U << 12) | (1U << 11);
            TIM2->CCER |= (1U << 4);
            TIM2->CCR2 = duty_cycle;
            break;
        case 3:
            TIM2->CCMR2 &= ~(0xFFU << 0);
            TIM2->CCMR2 |= (6U << 4) | (1U << 3);
            TIM2->CCER |= (1U << 8);
            TIM2->CCR3 = duty_cycle;
            break;
        case 4:
            TIM2->CCMR2 &= ~(0xFFU << 8);
            TIM2->CCMR2 |= (6U << 12) | (1U << 11);
            TIM2->CCER |= (1U << 12);
            TIM2->CCR4 = duty_cycle;
            break;
    }

    TIM2->CR1 |= (1U << 7);
    TIM2->EGR = (1U << 0);
    TIM2->CR1 |= (1U << 0);
}

void ControlPWM_Timer2(uint8_t channel, uint16_t duty_cycle)
{
    if (duty_cycle > PWM_PERIOD)
    {
        duty_cycle = PWM_PERIOD;
    }

    switch (channel)
    {
        case 1: TIM2->CCR1 = duty_cycle; break;
        case 2: TIM2->CCR2 = duty_cycle; break;
        case 3: TIM2->CCR3 = duty_cycle; break;
        case 4: TIM2->CCR4 = duty_cycle; break;
        default: break;
    }
}

void TIM3_ConfigUpdateTrigger(uint16_t prescaler, uint16_t auto_reload)
{
    RCC_EnableTIM3Clock();

    TIM3->CR1 = 0U;
    TIM3->CR2 = 0U;
    TIM3->PSC = prescaler;
    TIM3->ARR = auto_reload;
    TIM3->CNT = 0U;

    TIM3->EGR = (1U << 0);
    TIM3->SR = 0U;

    /* MMS=010: Update event tao TIM3_TRGO. */
    TIM3->CR2 = (2U << 4);
}

void TIM3_Start(void)
{
    TIM3->CR1 |= (1U << 0);
}

void TIM3_Stop(void)
{
    TIM3->CR1 &= ~(1U << 0);
}
