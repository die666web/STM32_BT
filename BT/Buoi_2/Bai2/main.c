#include <stdint.h>
#include "../../CMSIS/clock.h"

#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define Systick_BASE 0xE000E010

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
} RCC_Typedef;

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_Typedef;

typedef struct {
    volatile uint32_t CTRL;
    volatile uint32_t LOAD;
    volatile uint32_t VAL;
    volatile uint32_t CALIB;
}Systick_Typdef;

#define RCC ((RCC_Typedef *)RCC_BASE)
#define GPIOA ((GPIO_Typedef *)GPIOA_BASE)
#define Systick ((Systick_Typdef *)Systick_BASE)

volatile uint32_t systick_counter = 0;

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

void GPIO_Init(void)
{
    /* Bật clock GPIOA */
    RCC->APB2ENR |= (1U << 2);

    /* Xóa 4 bit cấu hình của PA0, PA1, PA2 */
    GPIOA->CRL &= ~(
          (0xFU << 0)
        | (0xFU << 4)
        | (0xFU << 8)
    );

    /* PA0, PA1, PA2: output push-pull 50 MHz = 0x3 */
    GPIOA->CRL |=
          (0x3U << 0) // 0011 
        | (0x3U << 4)
        | (0x3U << 8);

    /* Ban đầu tắt cả ba LED */
    GPIOA->BRR = (1U << 0) |
                 (1U << 1) |
                 (1U << 2);
}

// delay 1ms using SysTick timer
void Systick_init(void)
{
    Systick->LOAD = 8000 - 1; // Set reload register for 1ms delay
    Systick->VAL = 0; // Clear the current value
    Systick->CTRL = 1 << 0 | 1 << 1 | 1 << 2; // Enable SysTick with processor clock and no interrupt
}

void SysTick_Handler(void)
{
    systick_counter++;
}

int main(void)
{
    GPIO_Init();
    Systick_init();
    uint32_t led_1_counter = 0;
    uint32_t led_2_counter = 0;
    uint32_t led_3_counter = 0;
    while(1){
        if(systick_counter - led_1_counter >= 50)
        {
            GPIOA->ODR ^= 1 << 0; // Toggle PA0
            led_1_counter = systick_counter;
        }
        if(systick_counter -led_2_counter >= 500)
        {
            GPIOA->ODR ^= 1 << 1;
            led_2_counter = systick_counter;
        }
        if(systick_counter - led_3_counter >= 5000)
        {
            GPIOA->ODR ^= 1 << 2;
            led_3_counter = systick_counter;
        }

    }
}