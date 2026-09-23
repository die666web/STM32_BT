#include <stdint.h>
#include "dma.h"
#include "rcc.h"

#define DMA1_BASE     0x40020000UL
#define DMA1_CH1_BASE 0x40020008UL
#define NVIC_ISER0    (*(volatile uint32_t *)0xE000E100UL)

typedef struct
{
    volatile uint32_t ISR;
    volatile uint32_t IFCR;
} DMA_TypeDef;

typedef struct
{
    volatile uint32_t CCR;
    volatile uint32_t CNDTR;
    volatile uint32_t CPAR;
    volatile uint32_t CMAR;
} DMA_Channel_TypeDef;

#define DMA1     ((DMA_TypeDef *)DMA1_BASE)
#define DMA1_CH1 ((DMA_Channel_TypeDef *)DMA1_CH1_BASE)

static volatile uint8_t half_transfer_ready = 0;
static volatile uint8_t transfer_complete_ready = 0;

void DMA1_Channel1_Init(
    volatile uint32_t *peripheral_address,
    volatile uint16_t *memory_address,
    uint16_t number_of_data
)
{
    RCC_EnableDMA1Clock();

    DMA1_CH1->CCR &= ~(1U << 0);

    /* Xoa GIF1, TCIF1, HTIF1 va TEIF1. */
    DMA1->IFCR = 0x0FU;

    DMA1_CH1->CPAR = (uint32_t)(uintptr_t)peripheral_address;
    DMA1_CH1->CMAR = (uint32_t)(uintptr_t)memory_address;
    DMA1_CH1->CNDTR = number_of_data;

    DMA1_CH1->CCR =
          (1U << 1)   /* TCIE */
        | (1U << 2)   /* HTIE */
        | (1U << 5)   /* CIRC */
        | (1U << 7)   /* MINC */
        | (1U << 8)   /* PSIZE=01: 16-bit */
        | (1U << 10)  /* MSIZE=01: 16-bit */
        | (2U << 12); /* High priority */

    /* DMA1 Channel 1 IRQ = 11. */
    NVIC_ISER0 = (1U << 11);

    DMA1_CH1->CCR |= (1U << 0);
}

uint8_t DMA1_Channel1_TakeHalfTransfer(void)
{
    if (!half_transfer_ready)
    {
        return 0;
    }

    half_transfer_ready = 0;
    return 1;
}

uint8_t DMA1_Channel1_TakeTransferComplete(void)
{
    if (!transfer_complete_ready)
    {
        return 0;
    }

    transfer_complete_ready = 0;
    return 1;
}

void DMA1_Channel1_IRQHandler(void)
{
    uint32_t status = DMA1->ISR;

    if (status & (1U << 2)) /* HTIF1 */
    {
        DMA1->IFCR = (1U << 2);
        half_transfer_ready = 1;
    }

    if (status & (1U << 1)) /* TCIF1 */
    {
        DMA1->IFCR = (1U << 1);
        transfer_complete_ready = 1;
    }

    if (status & (1U << 3)) /* TEIF1 */
    {
        DMA1->IFCR = (1U << 3);
    }
}
