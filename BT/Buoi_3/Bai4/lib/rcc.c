#include <stdint.h>
#include "rcc.h"

#define RCC_BASE 0x40021000UL

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

#define RCC ((RCC_TypeDef *)RCC_BASE)

void RCC_EnableGPIOAClock(void)  { RCC->APB2ENR |= (1U << 2); }
void RCC_EnableGPIOBClock(void)  { RCC->APB2ENR |= (1U << 3); }
void RCC_EnableGPIOCClock(void)  { RCC->APB2ENR |= (1U << 4); }
void RCC_EnableADC1Clock(void)   { RCC->APB2ENR |= (1U << 9); }
void RCC_EnableUSART1Clock(void) { RCC->APB2ENR |= (1U << 14); }
void RCC_EnableTIM2Clock(void)   { RCC->APB1ENR |= (1U << 0); }
void RCC_EnableTIM3Clock(void)   { RCC->APB1ENR |= (1U << 1); }
void RCC_EnableDMA1Clock(void)   { RCC->AHBENR  |= (1U << 0); }

void RCC_SetADCPrescaler(uint8_t divider_bits)
{
    RCC->CFGR &= ~(3U << 14);
    RCC->CFGR |= ((uint32_t)(divider_bits & 3U) << 14);
}
