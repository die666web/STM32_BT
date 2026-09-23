#include <stdint.h>

#include "lib/max7219.h"

static void Delay(uint32_t time)
{
    volatile uint32_t i;
    volatile uint32_t j;

    for (i = 0U; i < time; i++)
    {
        for (j = 0U; j < 1000U; j++)
        {
        }
    }
}

int main(void)
{
    uint32_t count = 0U;

    MAX7219_Init();

    while (1)
    {
        MAX7219_DisplayNumber(count);

        count++;

        if (count > 99999999U)
        {
            count = 0U;
        }

        Delay(500U);
    }
}