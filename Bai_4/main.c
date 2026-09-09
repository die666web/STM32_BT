#include <stdint.h>

#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x40010800UL

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

#define RCC     ((RCC_TypeDef *)RCC_BASE)
#define GPIOA   ((GPIO_TypeDef *)GPIOA_BASE)

#define BUTTON_PIN  0
#define LED_PIN     1

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

int main(void)
{
    uint8_t old_state = 1;
    uint8_t new_state = 1;

    RCC->APB2ENR |= (1U << 2);

    GPIOA->CRL &= ~(0xFU << 0);
    GPIOA->CRL |=  (0x8U << 0);
    GPIOA->ODR |=  (1U << BUTTON_PIN);

    GPIOA->CRL &= ~(0xFU << 4);
    GPIOA->CRL |=  (0x2U << 4);

    GPIOA->BRR = (1U << LED_PIN);

    while (1)
    {
        new_state = (GPIOA->IDR & (1U << BUTTON_PIN)) ? 1 : 0;

        if (new_state != old_state)
        {
            delay(20);

            new_state = (GPIOA->IDR & (1U << BUTTON_PIN)) ? 1 : 0;

            if (new_state != old_state)
            {
                old_state = new_state;

                if (new_state == 1)
                {
                    GPIOA->ODR ^= (1U << LED_PIN);
                }
            }
        }
    }
}