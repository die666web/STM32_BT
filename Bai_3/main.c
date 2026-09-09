#include <stdint.h>

#define RCC_APB2ENR   (*(volatile uint32_t *)0x40021018)
#define GPIOC_CRH     (*(volatile uint32_t *)0x40011004)
#define GPIOC_BSRR    (*(volatile uint32_t *)0x40011010)
#define GPIOC_BRR     (*(volatile uint32_t *)0x40011014)

static void delay(volatile uint32_t count)
{
    while (count--) {
        __asm volatile ("nop");
    }
}

int main(void)
{
    /* Enable GPIOC clock */
    RCC_APB2ENR |= (1 << 4);

    /* PC13: output push-pull, 2 MHz */
    GPIOC_CRH &= ~(0xF << 20);
    GPIOC_CRH |=  (0x2 << 20);

    while (1)
    {
        /* PC13 LOW -> LED ON */
        GPIOC_BRR = (1 << 13);
        delay(500000);

        /* PC13 HIGH -> LED OFF */
        GPIOC_BSRR = (1 << 13);
        delay(500000);
    }
}
