#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "lib/gpio.h"
#include "lib/rcc.h"

static uint8_t led_pins[3] = {0, 1, 2};

uint16_t half_timeDelay(uint8_t pin)
{
    uint16_t sum = 0;

    if(pin == 0)
    {
        sum = 100/2;
    }
    else if(pin == 1)
    {
        sum = 1000/2;
    }
    else if(pin == 2)
    {
        sum = 10000/2;
    }

    return sum;
}

void Led_Task(void * parameters)
{
    uint8_t led_pin = *(uint8_t *)parameters;
    while(1)
    {
            GPIO_WritePin(GPIOA, led_pin, 0);
            vTaskDelay(pdMS_TO_TICKS(half_timeDelay(led_pin)));

            GPIO_WritePin(GPIOA, led_pin, 1);
            vTaskDelay(pdMS_TO_TICKS(half_timeDelay(led_pin)));
    
    }
}

void GPIO_Init()
{
    RCC_EnableGPIOAClock();
    GPIO_ConfigPin(GPIOA, 0, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_50MHZ);
    GPIO_ConfigPin(GPIOA, 1, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_50MHZ);
    GPIO_ConfigPin(GPIOA, 2, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_50MHZ);
}

int main(void)
{
    GPIO_Init();

    for(int i = 0 ;i < 3; i++)
    {
        if(xTaskCreate(
            Led_Task,
            "lED",
            128,
            (void *)&led_pins[i],
            1,
            NULL
        ) != pdPASS)
        {
            while(1){}
        }
    }
    
    
    vTaskStartScheduler();

    while(1){};
}