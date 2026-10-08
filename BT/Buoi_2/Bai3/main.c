#include <stdint.h>

#define RCC_BASE     0x40021000UL
#define GPIOA_BASE   0x40010800UL
#define USART1_BASE  0x40013800UL
#define ADC1_BASE    0x40012400UL

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
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct
{
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} USART_TypeDef;

typedef struct
{
    volatile uint32_t SR;       // 0x00
    volatile uint32_t CR1;      // 0x04
    volatile uint32_t CR2;      // 0x08
    volatile uint32_t SMPR1;    // 0x0C
    volatile uint32_t SMPR2;    // 0x10
    volatile uint32_t JOFR1;    // 0x14
    volatile uint32_t JOFR2;    // 0x18
    volatile uint32_t JOFR3;    // 0x1C
    volatile uint32_t JOFR4;    // 0x20
    volatile uint32_t HTR;      // 0x24
    volatile uint32_t LTR;      // 0x28
    volatile uint32_t SQR1;     // 0x2C
    volatile uint32_t SQR2;     // 0x30
    volatile uint32_t SQR3;     // 0x34
    volatile uint32_t JSQR;     // 0x38
    volatile uint32_t JDR1;     // 0x3C
    volatile uint32_t JDR2;     // 0x40
    volatile uint32_t JDR3;     // 0x44
    volatile uint32_t JDR4;     // 0x48
    volatile uint32_t DR;       // 0x4C
} ADC_TypeDef;

#define RCC     ((RCC_TypeDef *)RCC_BASE)
#define GPIOA   ((GPIO_TypeDef *)GPIOA_BASE)
#define USART1  ((USART_TypeDef *)USART1_BASE)
#define ADC1    ((ADC_TypeDef *)ADC1_BASE)

/*================ UART =================*/

void UART_Init(void)
{
    /* Cấp clock GPIOA và USART1. */
    RCC->APB2ENR |= (1U << 2) | (1U << 14);

    /*
     * PA9: TX - Alternate Function Push-Pull 50 MHz = 1011
     * PA10: RX - Input Floating = 0100
     */
    GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->CRH |=  (0xBU << 4) | (0x4U << 8);

    /* PCLK2 = 8 MHz, baudrate = 9600. */
    USART1->BRR = 833U;

    /*
     * UE: bật USART
     * TE: bật truyền
     * RE: bật nhận
     */
    USART1->CR1 = (1U << 13) |
                  (1U << 3)  |
                  (1U << 2);
}

void UART_SendChar(char data)
{
    /* Chờ TXE = 1: thanh ghi truyền đang trống. */
    while (!(USART1->SR & (1U << 7)))
    {
    }

    USART1->DR = (uint32_t)data;
}

void UART_SendString(const char *text)
{
    while (*text != '\0')
    {
        UART_SendChar(*text);
        text++;
    }
}

void UART_SendNumber(uint16_t number)
{
    char text[5];
    uint8_t index = 0;

    if (number == 0U)
    {
        UART_SendChar('0');
        return;
    }

    while (number > 0U)
    {
        text[index++] = (char)('0' + number % 10U);
        number /= 10U;
    }

    while (index > 0U)
    {
        UART_SendChar(text[--index]);
    }
}

/*================ ADC =================*/

void ADC_Init(void)
{
    /* Cấp clock GPIOA và ADC1. */
    RCC->APB2ENR |= (1U << 2) | (1U << 9);

    /*
     * ADCPRE = 00
     * ADC clock = PCLK2 / 2 = 8 MHz / 2 = 4 MHz. Toois da laf 14MHz cung cap ADC
     */
    RCC->CFGR &= ~(3U << 14);

    /* PA5 ở chế độ Analog Input: MODE=00, CNF=00. */
    GPIOA->CRL &= ~(0xFU << 20);

    /* Channel 5 lấy mẫu trong 239.5 chu kỳ. */
    ADC1->SMPR2 &= ~(7U << 15);
    ADC1->SMPR2 |=  (7U << 15);

    /* Chỉ thực hiện một lần chuyển đổi. */
    ADC1->SQR1 = 0U;

    /* Lần chuyển đổi đầu tiên đọc channel 5, tức PA5. */
    ADC1->SQR3 = 5U;

    /*
     * EXTSEL = 111: chọn SWSTART. -> Co nghia laf kich hoat duoc bagn phan mem 
     * EXTTRIG = 1: cho phép kích hoạt chuyển đổi.
     */
    ADC1->CR2 = (7U << 17) | (1U << 20);

    /* ADON = 1: bật ADC. */
    ADC1->CR2 |= (1U << 0);

    /* Chờ ADC ổn định. */
    for (volatile uint32_t i = 0; i < 1000U; i++) ;

    /* Reset calibration. */
    ADC1->CR2 |= (1U << 3);
    while (ADC1->CR2 & (1U << 3));

    /* Bắt đầu calibration. */
    ADC1->CR2 |= (1U << 2);
    while (ADC1->CR2 & (1U << 2));
}

uint16_t ADC_Read(void)
{
    /* SWSTART = 1: bắt đầu chuyển đổi. */
    ADC1->CR2 |= (1U << 22);

    /* Chờ EOC = 1: chuyển đổi hoàn thành. */
    while (!(ADC1->SR & (1U << 1)));

    /* ADC STM32F103 có độ phân giải 12 bit: 0–4095. */
    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

/*================ DELAY =================*/

void Delay(void)
{
    for (volatile uint32_t i = 0; i < 50000U; i++)
    {
    }
}

/*================ MAIN =================*/

int main(void)
{
    uint16_t adc_value;

    UART_Init();
    ADC_Init();

    UART_SendString("ADC READY\r\n");

    while (1)
    {
        adc_value = ADC_Read();

        UART_SendString("ADC = ");
        UART_SendNumber(adc_value);
        UART_SendString("\r\n");

        Delay();
    }
}