#ifndef DMA_H
#define DMA_H

#include <stdint.h>

void DMA1_Channel1_Init(
    volatile uint32_t *peripheral_address,
    volatile uint16_t *memory_address,
    uint16_t number_of_data
);

uint8_t DMA1_Channel1_TakeHalfTransfer(void);
uint8_t DMA1_Channel1_TakeTransferComplete(void);
void DMA1_Channel1_IRQHandler(void);

#endif
