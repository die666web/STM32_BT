#include <stdint.h>
#include "lib/gpio.h"
#include "lib/rcc.h"
#include "lib/timer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

typedef struct 
{
    uint8_t pin;
    uint16_t pwm;
    uint8_t enable; 
} Led_typdeDef;

static Led_typdeDef led_config[3] = {
    {1, 500, 1},  
    {2, 500, 0},   
    {3, 500, 0}    
};

uint16_t state_enable_led = 0;

static QueueHandle_t xQueue[3];

/**
 * @brief 
 * Config 3 button and 3 led
 * 3 button: GPIOB, 0, 1, 2
 * 3 led: GPIOA, 0, 1, 2
 */
void GPIO_Init(){
    // Button
    RCC_EnableGPIOAClock();
    RCC_EnableGPIOBClock();
    GPIO_ConfigPin(GPIOB, 0, GPIO_MODE_INPUT_PULLUP, GPIO_SPEED_50MHZ);
    GPIO_ConfigPin(GPIOB, 1, GPIO_MODE_INPUT_PULLUP, GPIO_SPEED_50MHZ);
    GPIO_ConfigPin(GPIOB, 10, GPIO_MODE_INPUT_PULLUP, GPIO_SPEED_50MHZ);
}

void Transfer_Led(void)
{
    led_config[state_enable_led].enable = 0;
    state_enable_led += 1;
    if(state_enable_led > 2) state_enable_led = 0;
    led_config[state_enable_led].enable = 1;

    for(int i = 0; i < 3; i++)
    {
        xQueueSend(xQueue[i], &led_config[i], portMAX_DELAY);
    }
}

void Increase_Led(void)
{
    for(int i = 0; i < 3; i++)
    {
        uint16_t pwm = led_config[i].pwm;
        if(led_config[i].enable == 1)
        {
            pwm += 100;
            if(pwm > 1000) pwm = 1000;

            led_config[i].pwm = pwm;
            xQueueSend(xQueue[i], &led_config[i], portMAX_DELAY);
        }
    }
}

void Decrease_led(void)
{
    for(int i = 0; i < 3; i++)
    {
        int32_t pwm = led_config[i].pwm;
        if(led_config[i].enable == 1)
        {
            pwm -= 100;
            if(pwm < 0) pwm = 0;

            led_config[i].pwm = pwm;
            xQueueSend(xQueue[i], &led_config[i], portMAX_DELAY);
        }
    }
}

/**
 * @brief 
 * 
 * @param parameters 
 * 3 Button
 * 1: Dung chuyen led
 * 2: tang do sang 10%
 * 3: giam do sang 10%
 */
void Button_Task(void *parameters)
{
    (void)parameters;

    uint8_t new_1 = 0;
    uint8_t new_2 = 0;
    uint8_t new_3 = 0;

    uint8_t old_1 = GPIO_ReadPin(GPIOB, 0);
    uint8_t old_2 = GPIO_ReadPin(GPIOB, 1);
    uint8_t old_3 = GPIO_ReadPin(GPIOB, 10);

    while(1)
    {
        new_1 = GPIO_ReadPin(GPIOB, 0);
        new_2 = GPIO_ReadPin(GPIOB, 1);
        new_3 = GPIO_ReadPin(GPIOB, 10);

        if(new_1 == 0 && old_1 == 1)
        {
            Transfer_Led();
        }

        if(new_2 == 0 && old_2 == 1)
        {
            Increase_Led();
        }

        if(new_3 == 0 && old_3 == 1)
        {
            Decrease_led();
        }

        old_1 = new_1;
        old_2 = new_2;
        old_3 = new_3;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief 
 * 
 * 3 leh hoajt ddong o 3 tasjk dong lap 
 * 
 */
void Led_Task(void *parameters)
{
    const Led_typdeDef *config = (const Led_typdeDef *)parameters;
    Led_typdeDef led_queue;
    while(1)
    {
        if (xQueueReceive(xQueue[config->pin - 1], &led_queue, portMAX_DELAY) == pdPASS)
        {
            uint16_t duty = led_queue.enable ? led_queue.pwm : 0U;
            ControlPWM_Timer2(config->pin, duty);
        }
    }
}

void configTimer()
{
    Config_Timer(1, 0);
    Config_Timer(2, 0);
    Config_Timer(3, 0);
}

int main(void)
{
    GPIO_Init();
    configTimer();

    for (int i = 0; i < 3; i++)
    {
        xQueue[i] = xQueueCreate(1, sizeof(Led_typdeDef));
        if (xQueue[i] == NULL)
            while (1) {}

        xQueueOverwrite(xQueue[i], &led_config[i]);
    }

    if (xTaskCreate(Button_Task, "Button", 128, NULL, 1, NULL) != pdPASS)
        while (1) {}

    for (int i = 0; i < 3; i++)
    {
        if (xTaskCreate(Led_Task, "LED", 128,
                        &led_config[i], 1, NULL) != pdPASS)
            while (1) {}
    }


    vTaskStartScheduler();
    while(1){

        // ControlPWM_Timer2(1, 500);
    }
}