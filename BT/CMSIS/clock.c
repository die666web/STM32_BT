#include "clock.h"

#define RCC_BASE    0x40021000UL
#define FLASH_BASE  0x40022000UL

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

typedef struct
{
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

#define RCC    ((RCC_TypeDef *)RCC_BASE)
#define FLASH  ((FLASH_TypeDef *)FLASH_BASE)


void SystemClock_Init(void)
{
    /*
     * HSE = 8 MHz
     * PLL = HSE x 9
     *
     * SYSCLK = 72 MHz
     * HCLK   = 72 MHz
     * PCLK1  = 36 MHz
     * PCLK2  = 72 MHz
     */

    // 1. Enable HSE
    RCC->CR |= (1U << 16);

    // Wait HSE ready
    while (!(RCC->CR & (1U << 17)))
    {
    }


    // 2. Flash:
    // Prefetch enable
    // 2 wait states cho 72 MHz
    FLASH->ACR = (1U << 4) | (2U << 0);


    // 3. Clear các bit clock config cần dùng
    RCC->CFGR &= ~(
          (0xFU << 4)     // HPRE
        | (0x7U << 8)     // PPRE1
        | (0x7U << 11)    // PPRE2
        | (1U << 16)      // PLLSRC
        | (1U << 17)      // PLLXTPRE
        | (0xFU << 18)    // PLLMUL
    );


    /*
     * AHB /1
     * HPRE = 0000
     *
     * APB1 /2
     * PPRE1 = 100
     *
     * APB2 /1
     * PPRE2 = 000
     */

    RCC->CFGR |= (0x4U << 8);


    // 4. PLL source = HSE
    RCC->CFGR |= (1U << 16);


    // 5. PLL multiplier = x9
    // PLLMUL = 0111
    RCC->CFGR |= (0x7U << 18);


    // 6. Enable PLL
    RCC->CR |= (1U << 24);

    // Wait PLL ready
    while (!(RCC->CR & (1U << 25)))
    {
    }


    // 7. Chọn PLL làm SYSCLK
    // SW = 10
    RCC->CFGR &= ~(0x3U << 0);
    RCC->CFGR |=  (0x2U << 0);


    // 8. Đợi PLL thực sự trở thành SYSCLK
    // SWS = 10
    while (((RCC->CFGR >> 2) & 0x3U) != 0x2U)
    {
    }
}