#ifndef UART_H
#define UART_H

#include <stdint.h>

void Config_uart(void);
void SendTextInterrupt_UART(const char *text);
void SendStringInterrupt_UART(const char *text);
void SendNumberInterrupt_UART(uint16_t number);
uint8_t ReceiveInterrupt_UART(uint8_t *out_data);
void USART1_IRQHandler(void);

#endif
