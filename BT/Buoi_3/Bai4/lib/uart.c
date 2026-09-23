#include <stdint.h>
#include "uart.h"
#include "gpio.h"
#include "rcc.h"

#define USART1_BASE      0x40013800UL
#define NVIC_ISER1       (*(volatile uint32_t *)0xE000E104UL)
#define UART_BUFFER_SIZE 128U

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

#define USART1 ((USART_TypeDef *)USART1_BASE)

static volatile uint8_t rx_buffer[UART_BUFFER_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;
static volatile uint8_t tx_buffer[UART_BUFFER_SIZE];
static volatile uint8_t tx_head = 0;
static volatile uint8_t tx_tail = 0;

static uint8_t UART_NextIndex(uint8_t index)
{
    index++;
    if (index >= UART_BUFFER_SIZE)
    {
        index = 0;
    }
    return index;
}

static uint8_t UART_SendByteInterrupt(uint8_t data)
{
    uint8_t next = UART_NextIndex(tx_head);

    if (next == tx_tail)
    {
        return 0;
    }

    tx_buffer[tx_head] = data;
    tx_head = next;
    USART1->CR1 |= (1U << 7);
    return 1;
}

void Config_uart(void)
{
    RCC_EnableGPIOAClock();
    RCC_EnableUSART1Clock();

    GPIO_ConfigPin(GPIOA, 9, GPIO_MODE_AF_PP, GPIO_SPEED_50MHZ);
    GPIO_ConfigPin(GPIOA, 10, GPIO_MODE_INPUT_FLOATING, GPIO_SPEED_INPUT);

    USART1->BRR = 833U;
    USART1->CR1 = (1U << 2) | (1U << 3) |
                  (1U << 5) | (1U << 13);

    NVIC_ISER1 = (1U << 5);
}

void SendTextInterrupt_UART(const char *text)
{
    while (*text != '\0')
    {
        while (!UART_SendByteInterrupt((uint8_t)*text))
        {
        }
        text++;
    }
}

void SendStringInterrupt_UART(const char *text)
{
    SendTextInterrupt_UART(text);
    SendTextInterrupt_UART("\n");
}

void SendNumberInterrupt_UART(uint16_t number)
{
    char text[6];
    uint8_t index = 5;

    text[index] = '\0';
    do
    {
        text[--index] = (char)('0' + number % 10U);
        number /= 10U;
    }
    while (number != 0U);

    SendTextInterrupt_UART(&text[index]);
}

uint8_t ReceiveInterrupt_UART(uint8_t *out_data)
{
    if (rx_tail == rx_head)
    {
        return 0;
    }

    *out_data = rx_buffer[rx_tail];
    rx_tail = UART_NextIndex(rx_tail);
    return 1;
}

void USART1_IRQHandler(void)
{
    if ((USART1->SR & (1U << 7)) &&
        (USART1->CR1 & (1U << 7)))
    {
        if (tx_tail != tx_head)
        {
            USART1->DR = tx_buffer[tx_tail];
            tx_tail = UART_NextIndex(tx_tail);
        }

        if (tx_tail == tx_head)
        {
            USART1->CR1 &= ~(1U << 7);
        }
    }

    if ((USART1->SR & (1U << 5)) &&
        (USART1->CR1 & (1U << 5)))
    {
        uint8_t received = (uint8_t)USART1->DR;
        uint8_t next = UART_NextIndex(rx_head);

        if (next != rx_tail)
        {
            rx_buffer[rx_head] = received;
            rx_head = next;
        }
    }
}
