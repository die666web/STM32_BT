#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* Gia tri MODE/CNF 4 bit cua STM32F1. */
#define GPIO_INPUT_FLOATING     0x4U
#define GPIO_OUTPUT_PP_50MHZ    0x3U
#define GPIO_AF_PP_50MHZ        0xBU

void GPIOA_ConfigPin(uint8_t pin, uint8_t config);
void GPIOA_WritePin(uint8_t pin, uint8_t state);
uint8_t GPIOA_ReadPin(uint8_t pin);

#endif