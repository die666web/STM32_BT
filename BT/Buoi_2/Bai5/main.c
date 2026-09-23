#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lib/uart.h"
#include "lib/timer.h"

static uint8_t led_percent = 100U;
static uint8_t led_is_on = 0U;

/* ==================== LED ==================== */

static void LED_SetPercent(uint8_t percent)
{
    if (percent > 100U)
    {
        percent = 100U;
    }

    led_percent = percent;

    /* LED đang tắt thì chỉ lưu mức PWM. */
    if (led_is_on == 0U)
    {
        return;
    }

    ControlPWM_Timer2(
        2,
        ((uint32_t)led_percent * PWM_PERIOD) / 100U
    );
}

static void LED_On(void)
{
    led_is_on = 1U;

    ControlPWM_Timer2(
        2,
        ((uint32_t)led_percent * PWM_PERIOD) / 100U
    );
}

static void LED_Off(void)
{
    led_is_on = 0U;
    ControlPWM_Timer2(2, 0U);
}

/* ==================== PWM COMMAND ==================== */

static uint8_t ParsePWM(
    const char *command,
    uint8_t *percent
)
{
    char *end;
    unsigned long value;

    if (strncmp(command, "PWM:", 4U) != 0) return 0;
    value = strtoul(&command[4], &end, 10);

    /* Không đọc được chữ số nào. */
    if (end == &command[4]) return 0;

    if (*end != '%' ||
        end[1] != '\0' ||
        value > 100UL) return 0;
        
    *percent = (uint8_t)value;

    return 1U;
}

/* ==================== STATUS ==================== */

static void SendStatus(void)
{
    char status[32];

    snprintf(
        status,
        sizeof(status),
        "LED %s PWM: %u%%",
        led_is_on ? "ON" : "OFF",
        (unsigned int)led_percent
    );

    SendStringInterrupt_UART(status);
}

/* ==================== MAIN ==================== */

int main(void)
{
    char command[16] = "";
    uint8_t percent;

    Config_uart();
    Config_Timer(2, 0);

    SendStringInterrupt_UART(
        "Nhap on, off, PWM:50% hoac status"
    );

    while (1)
    {
        if (UART_ReadLine(command, sizeof(command)))
        {
            if (strcmp(command, "on") == 0)
            {
                LED_On();
                SendStringInterrupt_UART("LED ON");
            }
            else if (strcmp(command, "off") == 0)
            {
                LED_Off();
                SendStringInterrupt_UART("LED OFF");
            }
            else if (strcmp(command, "status") == 0)
            {
                SendStatus();
            }
            else if (ParsePWM(command, &percent))
            {
                LED_SetPercent(percent);
                SendStringInterrupt_UART("PWM OK");
            }
            else
            {
                SendStringInterrupt_UART("Lenh sai");
            }

            command[0] = '\0';
        }
    }
}