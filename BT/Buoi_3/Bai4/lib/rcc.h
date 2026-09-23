#ifndef RCC_H
#define RCC_H

#include <stdint.h>

void RCC_EnableGPIOAClock(void);
void RCC_EnableGPIOBClock(void);
void RCC_EnableGPIOCClock(void);
void RCC_EnableUSART1Clock(void);
void RCC_EnableADC1Clock(void);
void RCC_EnableTIM2Clock(void);
void RCC_EnableTIM3Clock(void);
void RCC_EnableDMA1Clock(void);
void RCC_SetADCPrescaler(uint8_t divider_bits);

#endif
