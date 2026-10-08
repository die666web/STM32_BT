#include <stdint.h>

#define RCC_BASE   0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define UART1_BASE 0x40013800UL

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
} RCC_Typedef;

typedef struct
{
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_Typedef;

typedef struct
{
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} UART1_Typedef;

#define RCC   ((RCC_Typedef *)RCC_BASE)
#define GPIOA ((GPIO_Typedef *)GPIOA_BASE)
#define UART1 ((UART1_Typedef *)UART1_BASE)

void delay(uint16_t time)
{
    volatile uint32_t i;
    volatile uint32_t j;

    for (i = 0; i < time; i++)
        for (j = 0; j < 1000U; j++);
}

void Config_UART(void)
{
    RCC->APB2ENR |= (1U << 2) | (1U << 14);

    /* PA9 TX: AF push-pull; PA10 RX: input floating */
    GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->CRH |=  (0xBU << 4) | (0x4U << 8);

    UART1->CR2 = 0U;
    UART1->CR3 = 0U;

    /* Clock mặc định 8 MHz, baud 9600 */
    UART1->BRR = 833U;

    /* RE, TE, UE */
    UART1->CR1 = (1U << 2) |
                 (1U << 3) |
                 (1U << 13);
}

void Send_UART(uint8_t data)
{
    while ((UART1->SR & (1U << 7)) == 0U);
    UART1->DR = data;
}

void SendString_UART(const char *text)
{
    while (*text)
        Send_UART((uint8_t)*text++);
}

uint8_t Receive_UART(void)
{
    /* Chờ RXNE = 1, nghĩa là đã nhận được một byte. */
    while ((UART1->SR & (1U << 5)) == 0U);

    return (uint8_t)UART1->DR;
}

int main(void)
{
    char buffer[64];
    uint8_t index = 0;

    Config_UART();

    buffer[0] = '\0';

    while (1)
    {
        uint8_t data_received = Receive_UART();

        if (data_received == '!')
        {
            buffer[index] = '\0';

            SendString_UART("DMDTMT02 - Nhom 14: ");
            SendString_UART(buffer);
            SendString_UART("\r\n");

            for (uint8_t i = 0; i < 64; i++)
            {
                buffer[i] = '\0';
            }

            index = 0;
        }
        else 
        {
            buffer[index] = (char)data_received;
            index++;

            /* Luôn kết thúc chuỗi. */
            buffer[index] = '\0';
        }
    }
}