#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define REG32(address) (*(volatile uint32_t *)(address))

/* RCC */
#define RCC_APB1RSTR REG32(0x40021010UL)
#define RCC_APB2ENR  REG32(0x40021018UL)
#define RCC_APB1ENR  REG32(0x4002101CUL)

/* GPIO */
#define GPIOA_CRH REG32(0x40010804UL)
#define GPIOB_CRL REG32(0x40010C00UL)

/* USART1 */
#define USART1_SR  REG32(0x40013800UL)
#define USART1_DR  REG32(0x40013804UL)
#define USART1_BRR REG32(0x40013808UL)
#define USART1_CR1 REG32(0x4001380CUL)

/* I2C1 */
#define I2C1_CR1   REG32(0x40005400UL)
#define I2C1_CR2   REG32(0x40005404UL)
#define I2C1_OAR1  REG32(0x40005408UL)
#define I2C1_DR    REG32(0x40005410UL)
#define I2C1_SR1   REG32(0x40005414UL)
#define I2C1_SR2   REG32(0x40005418UL)
#define I2C1_CCR   REG32(0x4000541CUL)
#define I2C1_TRISE REG32(0x40005420UL)

#define DS1307_WRITE 0xD0U
#define DS1307_READ  0xD1U

#define I2C_TIMEOUT 100000U

/* ================= UART ================= */

void UART_Init(void)
{
    RCC_APB2ENR |= (1U << 2) | (1U << 14);

    GPIOA_CRH &= ~((0xFU << 4) | (0xFU << 8));
    GPIOA_CRH |=  (0xBU << 4) | (0x4U << 8);

    USART1_BRR = 833U;
    USART1_CR1 = (1U << 2) | (1U << 3) | (1U << 13);
}

void UART_SendChar(char data)
{
    while (!(USART1_SR & (1U << 7)));
    USART1_DR = (uint8_t)data;
}

void UART_SendString(const char *text)
{
    while (*text) UART_SendChar(*text++);
}

uint8_t UART_ReadLine(char *text, uint8_t size)
{
    static uint8_t index = 0;
    char data;

    if (!(USART1_SR & (1U << 5))) return 0;

    data = (char)USART1_DR;

    if (data == '\r' || data == '\n')
    {
        if (index == 0) return 0;

        text[index] = '\0';
        index = 0;
        return 1;
    }

    if (index < size - 1)
    {
        text[index++] = data;
    }
    else
    {
        index = 0;
        text[0] = '\0';
    }

    return 0;
}

/* ================= I2C ================= */

void I2C_Init(void)
{
    RCC_APB2ENR |= (1U << 3);
    RCC_APB1ENR |= (1U << 21);

    /* PB6 SCL, PB7 SDA: AF open-drain 50 MHz */
    GPIOB_CRL &= ~((0xFU << 24) | (0xFU << 28));
    GPIOB_CRL |=  (0xFU << 24) | (0xFU << 28);

    /* Reset I2C1 */
    RCC_APB1RSTR |=  (1U << 21);
    RCC_APB1RSTR &= ~(1U << 21);

    I2C1_CR1 = (1U << 15);
    I2C1_CR1 = 0;

    /* PCLK1 = 8 MHz, I2C = 100 kHz */
    I2C1_CR2   = 8U;
    I2C1_OAR1  = (1U << 14);
    I2C1_CCR   = 40U;
    I2C1_TRISE = 9U;

    /* PE + ACK */
    I2C1_CR1 = (1U << 0) | (1U << 10);
}

void I2C_ClearADDR(void)
{
    volatile uint32_t temp;

    temp = I2C1_SR1;
    temp = I2C1_SR2;

    (void)temp;
}

uint8_t I2C_WaitSR1(uint32_t flag)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (!(I2C1_SR1 & flag))
    {
        /* AF: thiết bị không trả ACK */
        if (I2C1_SR1 & (1U << 10)) return 0;
        if (--timeout == 0) return 0;
    }

    return 1;
}

uint8_t I2C_WaitBusFree(void)
{
    uint32_t timeout = I2C_TIMEOUT;

    while (I2C1_SR2 & (1U << 1))
        if (--timeout == 0) return 0;

    return 1;
}

void I2C_StopError(void)
{
    I2C1_CR1 |= (1U << 9);
    I2C1_SR1 &= ~(1U << 10);
}

/* ================= DS1307 ================= */

uint8_t DS1307_Write(uint8_t reg, uint8_t data)
{
    if (!I2C_WaitBusFree())
    {
        UART_SendString("ERROR: I2C BUS BUSY\r\n");
        return 0;
    }

    I2C1_CR1 |= (1U << 8);
    if (!I2C_WaitSR1(1U << 0)) goto error;

    I2C1_DR = DS1307_WRITE;
    if (!I2C_WaitSR1(1U << 1)) goto error;
    I2C_ClearADDR();

    I2C1_DR = reg;
    if (!I2C_WaitSR1(1U << 7)) goto error;

    I2C1_DR = data;
    if (!I2C_WaitSR1(1U << 2)) goto error;

    I2C1_CR1 |= (1U << 9);
    return 1;

error:
    I2C_StopError();
    UART_SendString("ERROR: DS1307 NOT ACK\r\n");
    return 0;
}

uint8_t DS1307_Read(uint8_t reg, uint8_t *data)
{
    if (!I2C_WaitBusFree())
    {
        UART_SendString("ERROR: I2C BUS BUSY\r\n");
        return 0;
    }

    I2C1_CR1 |= (1U << 8);
    if (!I2C_WaitSR1(1U << 0)) goto error;

    I2C1_DR = DS1307_WRITE;
    if (!I2C_WaitSR1(1U << 1)) goto error;
    I2C_ClearADDR();

    I2C1_DR = reg;
    if (!I2C_WaitSR1(1U << 2)) goto error;

    /* Repeated START */
    I2C1_CR1 |= (1U << 8);
    if (!I2C_WaitSR1(1U << 0)) goto error;

    I2C1_DR = DS1307_READ;
    if (!I2C_WaitSR1(1U << 1)) goto error;

    /* Nhận đúng một byte */
    I2C1_CR1 &= ~(1U << 10);
    I2C_ClearADDR();
    I2C1_CR1 |= (1U << 9);

    if (!I2C_WaitSR1(1U << 6)) goto error;

    *data = (uint8_t)I2C1_DR;
    I2C1_CR1 |= (1U << 10);

    return 1;

error:
    I2C_StopError();
    I2C1_CR1 |= (1U << 10);
    UART_SendString("ERROR: DS1307 NOT ACK\r\n");
    return 0;
}

/* ================= TIME ================= */

uint8_t ToBCD(uint8_t value)
{
    return (uint8_t)(((value / 10U) << 4) | (value % 10U));
}

uint8_t FromBCD(uint8_t value)
{
    return (uint8_t)(((value >> 4) * 10U) + (value & 0x0FU));
}

uint8_t DS1307_SetTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    if (!DS1307_Write(0x00, ToBCD(second) & 0x7F)) return 0;
    if (!DS1307_Write(0x01, ToBCD(minute))) return 0;
    if (!DS1307_Write(0x02, ToBCD(hour))) return 0;

    return 1;
}

void DS1307_PrintTime(void)
{
    uint8_t hour, minute, second;
    char text[24];

    if (!DS1307_Read(0x00, &second)) return;
    if (!DS1307_Read(0x01, &minute)) return;
    if (!DS1307_Read(0x02, &hour)) return;

    second = FromBCD(second & 0x7F);
    minute = FromBCD(minute & 0x7F);
    hour   = FromBCD(hour & 0x3F);

    snprintf(
        text,
        sizeof(text),
        "%02u:%02u:%02u\r\n",
        hour,
        minute,
        second
    );

    UART_SendString(text);
}

/* ================= COMMAND ================= */

void ProcessCommand(char *command)
{
    unsigned int hour, minute, second;

    if (strcmp(command, "GET") == 0)
    {
        DS1307_PrintTime();
    }
    else if (sscanf(
        command,
        "SET %u:%u:%u",
        &hour,
        &minute,
        &second
    ) == 3)
    {
        if (hour > 23 || minute > 59 || second > 59)
        {
            UART_SendString("TIME ERROR\r\n");
            return;
        }

        if (DS1307_SetTime(hour, minute, second))
            UART_SendString("SET OK\r\n");
    }
    else
    {
        UART_SendString("COMMAND ERROR\r\n");
    }
}

/* ================= MAIN ================= */

int main(void)
{
    char command[24];

    UART_Init();
    I2C_Init();

    UART_SendString(
        "\r\nDS1307 READY\r\n"
        "GET\r\n"
        "SET 12:30:00\r\n"
    );

    while (1)
    {
        if (UART_ReadLine(command, sizeof(command)))
        {
            UART_SendString("RX: ");
            UART_SendString(command);
            UART_SendString("\r\n");

            ProcessCommand(command);
        }
    }
}