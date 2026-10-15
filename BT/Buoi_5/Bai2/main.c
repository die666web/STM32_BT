#include "FreeRTOS.h"
#include "lib/rcc.h"
#include "lib/uart.h"
#include "lib/gpio.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void delay(uint16_t time)
{
    for(int i = 0 ; i< time; i++)
    {
        for(int j = 0; j < 1000; j++)
        {}
    }
}



int main(void)
{
    Config_uart();
    while(1)
    {
        SendStringInterrupt_UART("Hello");
        delay(100);
    }
}
