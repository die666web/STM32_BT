#include <stdint.h>

#include "lib/uart.h"
#include "lib/timer.h"

uint8_t led_percent = 100;
uint8_t led_is_on = 0;

static uint8_t line_index = 0;

/* ==================== LED ==================== */

void LED_SetPercent(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }

    led_percent = percent;

    // Nếu LED đang tắt thì chỉ lưu PWM
    if (led_is_on == 0)
    {
        return;
    }

    ControlPWM_Timer2(
        2,
        ((uint32_t)led_percent * PWM_PERIOD) / 100U
    );
}

void LED_On(void)
{
    led_is_on = 1;

    ControlPWM_Timer2(
        2,
        ((uint32_t)led_percent * PWM_PERIOD) / 100U
    );
}

void LED_Off(void)
{
    led_is_on = 0;
    ControlPWM_Timer2(2, 0);
}

/* ==================== STRING ==================== */


void ClearCommand(char *command)
{
    line_index = 0;
    command[0] = '\0';
}


/* ==================== PWM COMMAND ==================== */

uint8_t ParsePWM(
    const char *command,
    uint8_t *percent
)
{
    uint8_t index = 4;
    uint16_t value = 0;

    if (command[0] != 'P' || command[1] != 'W' || command[2] != 'M' || command[3] != ':') return 0;

    // Sau PWM: phải có ít nhất một số
    if (command[index] < '0' || command[index] > '9') return 0;

    while (command[index] >= '0' && command[index] <= '9')
    {
        value = value * 10U + (uint16_t)(command[index] - '0');

        if (value > 100U) return 0;
        index++;
    }

    // Phải kết thúc bằng %
    if (command[index] != '%' || command[index + 1] != '\0')  return 0;

    *percent = (uint8_t)value;

    return 1;
}

/* ==================== STATUS ==================== */

void SendStatus(void)
{
    if (led_is_on)
    {
        SendStringInterrupt_UART("LED ON PWM: ");
    }
    else
    {
        SendStringInterrupt_UART("LED OFF PWM: ");
    }

    SendNumberInterrupt_UART(led_percent);
}

/* ==================== MAIN ==================== */


int main(void)
{
    char command[16];
    uint8_t percent;

    command[0] = '\0';

    Config_uart();
    Config_Timer(2, 0);

    SendStringInterrupt_UART("Nhap on, off, PWM:50% hoac status");

    while (1)
    {
        if (UART_ReadLine(command, sizeof(command)))
        {
            if (StringEqual(command, "on"))
            {
                LED_On();
                SendStringInterrupt_UART("LED ON");
            }
            else if (StringEqual(command, "off"))
            {
                LED_Off();
                SendStringInterrupt_UART("LED OFF");
            }
            else if (ParsePWM(command, &percent))
            {
                LED_SetPercent(percent);
                SendStringInterrupt_UART("PWM OK");
            }
            else if (StringEqual(command, "status"))
            {
                SendStatus();
            }
            else
            {
                SendStringInterrupt_UART("Lenh sai");
            }

            /* Lenh da xu ly xong, xoa chuoi de nhan lenh moi. */
            ClearCommand(command);
        }
    }
}
