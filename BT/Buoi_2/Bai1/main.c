#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Học luôn ở đây note ngay phần 

#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define GPIOB_BASE 0x40010C00UL
#define UART1_BASE 0x40013800UL


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
    volatile uint32_t SR;
    volatile uint8_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} UART1_Typedef;

#define RCC ((RCC_Typedef *)RCC_BASE)
#define GPIOA ((GPIO_Typedef *)GPIOA_BASE)
#define UART1 ((UART1_Typedef *)UART1_BASE)

void delay(uint16_t t)
{
    for(int i = 0; i < t; i++)
    {
        for(int j = 0; j < 1000; j++)
        {

        }
    }
}

void config_UART(void)
{
   // Cấp xung clock cho GPIOA và uart 1
    RCC->APB2ENR |= 1U << 2 | 1U << 14;

    GPIOA->CRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA->CRH |= (0xBU << 4) | (0x8U << 8);

    UART1->CR2 = 0U; // ko chọn stopbit 
    UART1->CR3 = 0U; // ko dùng chế độ đặc biệt
    // UART1->BRR = 7500U; // tần số là 72MHz -> Brr = 72M/9600 = 7500;
    UART1->BRR = 833U;
    UART1->CR1 = 1<<2| 1<<3 | 1<<13; 
}

void send_UART(uint8_t byte)
{
    while((UART1->SR & (1U << 6)) == 0){} 
    UART1->DR = byte;
}

void sendString_UART(const char *s)
{
    while(*s)
    {
        send_UART((uint8_t)*s++);
    }
}

uint8_t receive_UART(void)
{
    while((UART1->SR & (1U << 5)) == 0){} // Tuong tu: nhung doi voi nhan RXNE laf vi tri bit thu 5
    uint8_t data = (uint8_t)(UART1->DR);
    return data;
}

int main(void)
{
    config_UART();
    char buffer[64];
    uint8_t index = 0;
    while(1)
    {
        
        uint8_t data_received = receive_UART();

        if(data_received == '!') 
        {
            sendString_UART("DMDTMT02 - Nhom 14: ");
            sendString_UART(buffer);

            for (int i =0; i<64; i++)
            {
                buffer[i] = '\0';
            }
            
            index=0;
        }
        else
        {
            buffer[index] = data_received;
            index++;
        }
    }
}