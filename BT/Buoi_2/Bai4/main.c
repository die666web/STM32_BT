#include <stdint.h>

#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define TIM2_BASE 0x40000000UL

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


typedef struct
{
    volatile uint32_t CR1;      // 0x00
    volatile uint32_t CR2;      // 0x04
    volatile uint32_t SMCR;     // 0x08
    volatile uint32_t DIER;     // 0x0C
    volatile uint32_t SR;       // 0x10
    volatile uint32_t EGR;      // 0x14
    volatile uint32_t CCMR1;    // 0x18
    volatile uint32_t CCMR2;    // 0x1C
    volatile uint32_t CCER;     // 0x20
    volatile uint32_t CNT;      // 0x24
    volatile uint32_t PSC;      // 0x28
    volatile uint32_t ARR;      // 0x2C

    volatile uint32_t RESERVED; // 0x30

    volatile uint32_t CCR1;     // 0x34
    volatile uint32_t CCR2;     // 0x38
    volatile uint32_t CCR3;     // 0x3C
    volatile uint32_t CCR4;     // 0x40

    volatile uint32_t RESERVED2;// 0x44
    volatile uint32_t DCR;      // 0x48
    volatile uint32_t DMAR;     // 0x4C
} Tim2_Typedef;

#define RCC ((RCC_Typedef *)RCC_BASE)
#define GPIOA ((GPIO_Typedef *)GPIOA_BASE)
#define TIM2 ((Tim2_Typedef *)TIM2_BASE)

void Config_Timer(void)
{
    // Enable GPIOA + TIM2 clock
    RCC->APB2ENR |= (1U << 2);
    RCC->APB1ENR |= (1U << 0);

    // PA0..PA3 = Alternate Function Push-Pull 50MHz
    GPIOA->CRL &= ~(
          (0xFU << 0)
        | (0xFU << 4)
        | (0xFU << 8)
        | (0xFU << 12)
    );

    GPIOA->CRL |=
          (0xBU << 0)
        | (0xBU << 4)
        | (0xBU << 8)
        | (0xBU << 12);


    // Timer clock = 8MHz
    // 8MHz / (7 + 1) = 1MHz
    TIM2->PSC = 7;

    // 1MHz / 1000 = 1kHz PWM
    TIM2->ARR = 999;


    // ===== CH1 =====
    // OC1M bits 6:4 = 110 => PWM mode 1
    TIM2->CCMR1 &= ~(0x7U << 4);
    TIM2->CCMR1 |=  (0x6U << 4);


    // ===== CH2 =====
    // OC2M bits 14:12 = 110
    TIM2->CCMR1 &= ~(0x7U << 12);
    TIM2->CCMR1 |=  (0x6U << 12);


    // ===== CH3 =====
    // OC3M bits 6:4 = 110
    TIM2->CCMR2 &= ~(0x7U << 4);
    TIM2->CCMR2 |=  (0x6U << 4);


    // ===== CH4 =====
    // OC4M bits 14:12 = 110
    TIM2->CCMR2 &= ~(0x7U << 12);
    TIM2->CCMR2 |=  (0x6U << 12);


    // Enable CH1, CH2, CH3, CH4 output
    TIM2->CCER |=
          (1U << 0)    // CC1E
        | (1U << 4)    // CC2E
        | (1U << 8)    // CC3E
        | (1U << 12);  // CC4E


    // Duty riêng từng channel
    TIM2->CCR1 = 100;   // PA0 ~10%
    TIM2->CCR2 = 300;   // PA1 ~30%
    TIM2->CCR3 = 500;   // PA2 ~50%
    TIM2->CCR4 = 700;   // PA3 ~70%


    // Force update để PSC/ARR được nạp ngay
    TIM2->EGR |= (1U << 0);

    // Clear UIF do update event vừa tạo
    TIM2->SR &= ~(1U << 0);

    // Start TIM2
    TIM2->CR1 |= (1U << 0);
}


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
    // Enable clock for GPIOA and TIM2
    Config_Timer();
    while(1){}
}