#include <stdint.h>
#include "gpio.h"
#include "rcc.h"

#define GPIOA_BASE 0x40010800UL
typedef struct
{
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)

void GPIOA_ConfigPin(uint8_t pin, uint8_t config)
{
    uint32_t shift;

    if (pin > 15U)
    {
        return;
    }

    RCC_EnableGPIOAClock();
    config &= 0xFU;

    if (pin < 8U)
    {
        shift = (uint32_t)pin * 4U;
        GPIOA->CRL &= ~(0xFU << shift);
        GPIOA->CRL |=  ((uint32_t)config << shift);
    }
    else
    {
        shift = ((uint32_t)pin - 8U) * 4U;
        GPIOA->CRH &= ~(0xFU << shift);
        GPIOA->CRH |=  ((uint32_t)config << shift);
    }
}

void GPIOA_WritePin(uint8_t pin, uint8_t state)
{
    if (pin > 15U)
    {
        return;
    }

    if (state)
    {
        GPIOA->BSRR = (1U << pin);
    }
    else
    {
        GPIOA->BRR = (1U << pin);
    }
}

uint8_t GPIOA_ReadPin(uint8_t pin)
{
    if (pin > 15U)
    {
        return 0;
    }

    return (uint8_t)((GPIOA->IDR >> pin) & 1U);
}
