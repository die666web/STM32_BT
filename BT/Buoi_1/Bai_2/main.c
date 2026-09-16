#include <stdint.h>

#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x40010800UL
#define GPIOB_BASE  0x40010C00UL

typedef struct {
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

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

#define RCC   ((RCC_TypeDef *)RCC_BASE)
#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *)GPIOB_BASE)

void delay(uint32_t ms)
{
    volatile uint32_t i, j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 1000; j++)
        {
        }
    }
}

void GPIOA_Init_Register(void)
{
    RCC->APB2ENR |= (1U << 2);

    GPIOA->CRL = 0x11111111;
    GPIOA->ODR &= ~0xFF;
}

int main(void)
{
    int8_t i;

    GPIOA_Init_Register();

    while (1)
    {
        for (i = 0; i < 8; i++)
        {
            GPIOA->ODR &= ~0xFF;
            GPIOA->ODR |= (1U << i);

            delay(100);
        }

        for (i = 6; i >= 1; i--)
        {
            GPIOA->ODR &= ~0xFF;
            GPIOA->ODR |= (1U << i);

            delay(100);
        }
    }
}