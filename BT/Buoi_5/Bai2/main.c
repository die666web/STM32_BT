#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/rcc.h"
#include "lib/gpio.h"
#include "lib/uart.h"

#define RCC_APB2ENR   (*(volatile uint32_t *)0x40021018UL)
#define AFIO_EXTICR1  (*(volatile uint32_t *)0x40010008UL)

#define EXTI_IMR      (*(volatile uint32_t *)0x40010400UL)
#define EXTI_RTSR     (*(volatile uint32_t *)0x40010408UL)
#define EXTI_FTSR     (*(volatile uint32_t *)0x4001040CUL)
#define EXTI_PR       (*(volatile uint32_t *)0x40010414UL)

#define NVIC_ISER0    (*(volatile uint32_t *)0xE000E100UL)
#define NVIC_ISER1    (*(volatile uint32_t *)0xE000E104UL)
#define NVIC_IPR_IRQ6 (*(volatile uint8_t  *)0xE000E406UL)

static QueueHandle_t xCountQueue;
static volatile uint16_t count = 0;

static void ConfigButton_EXTI0(void)
{
    RCC_EnableGPIOBClock();
    RCC_EnableAPB2Clock(RCC_APB2_AFIO);

    GPIO_ConfigPin(
        GPIOB, 0,
        GPIO_MODE_INPUT_PULLUP,
        GPIO_SPEED_INPUT
    );
    GPIO_WritePin(GPIOB, 0, 1);  /* ODR=1 để chọn pull-up cho PB0. */

    AFIO_EXTICR1 &= ~(0xFU << 0);
    AFIO_EXTICR1 |=  (1U << 0);

    EXTI_RTSR &= ~(1U << 0);
    EXTI_FTSR |=  (1U << 0);

    EXTI_IMR &= ~(1U << 0);
    EXTI_PR = (1U << 0);

    NVIC_IPR_IRQ6 = 0xF0U;
    NVIC_ISER0 = (1U << 6);
}

void EXTI0_IRQHandler(void)
{
    BaseType_t task_woken = pdFALSE;

    if (EXTI_PR & (1U << 0))
    {
        EXTI_PR = (1U << 0);  /* Xóa pending EXTI0. */

        count++;
        uint16_t value = count;

        xQueueSendFromISR(xCountQueue, &value, &task_woken);
        portYIELD_FROM_ISR(task_woken);
    }
}

static void Uart_Task(void *parameters)
{
    (void)parameters;

    uint16_t received_count;
    char message[32];

    EXTI_PR = (1U << 0);
    EXTI_IMR |= (1U << 0);

    while (1)
    {
        if (xQueueReceive(
                xCountQueue,
                &received_count,
                portMAX_DELAY) == pdPASS)
        {
            snprintf(
                message,
                sizeof(message),
                "Count: %u",
                (unsigned int)received_count
            );

            SendStringInterrupt_UART(message);
        }
    }
}

int main(void)
{
    xCountQueue = xQueueCreate(5, sizeof(uint16_t));
    if (xCountQueue == NULL)
    {
        while (1) {}
    }

    Config_uart();

    NVIC_ISER1 = (1U << 5);

    ConfigButton_EXTI0();

    if (xTaskCreate(
            Uart_Task,
            "UART",
            256,
            NULL,
            2,
            NULL) != pdPASS)
    {
        while (1) {}
    }

    vTaskStartScheduler();

    while (1) {}
}