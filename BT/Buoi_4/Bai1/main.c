#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#define REG32(address) (*(volatile uint32_t *)(address))

#define RCC_CR       REG32(0x40021000UL)
#define RCC_CFGR     REG32(0x40021004UL)
#define RCC_APB2ENR  REG32(0x40021018UL)

#define GPIOA_CRL    REG32(0x40010800UL)
#define GPIOA_BSRR   REG32(0x40010810UL)

/* Mỗi task nhận một cấu hình riêng. */
typedef struct {
    uint8_t pin;
    float frequency_hz;
} LED_Config;

static LED_Config led_config[3] = {
    {0,  0.1f},
    {1,  1.0f},
    {2, 10.0f}
};

/* Chọn HSI 8 MHz, các bus không chia clock. */
static void Clock_Init(void)
{
    RCC_CR |= (1U << 0);
    while ((RCC_CR & (1U << 1)) == 0U) {}

    RCC_CFGR &= ~(3U << 0);
    while ((RCC_CFGR & (3U << 2)) != 0U) {}

    RCC_CFGR &= ~((0xFU << 4) |
                  (7U << 8)   |
                  (7U << 11));
}

static void GPIO_Init(void)
{
    RCC_APB2ENR |= (1U << 2);

    /* PA0, PA1, PA2: output push-pull, 2 MHz. */
    GPIOA_CRL &= ~0xFFFU;
    GPIOA_CRL |=  0x222U;

    /* Ban đầu tắt cả ba LED. */
    GPIOA_BSRR = (7U << 16);
}

/* Một hàm dùng chung cho cả ba task. */
static void LED_Blink(void *argument)
{
    const LED_Config *config = (const LED_Config *)argument;
    uint8_t state = 0U;

    TickType_t half_period = (TickType_t)(
        (float)configTICK_RATE_HZ /
        (2.0f * config->frequency_hz) + 0.5f
    );

    if (half_period == 0U)
        half_period = 1U;

    TickType_t last_wake = xTaskGetTickCount();

    while (1)
    {
        state ^= 1U;

        if (state)
            GPIOA_BSRR = (1U << config->pin);
        else
            GPIOA_BSRR = (1U << (config->pin + 16U));

        vTaskDelayUntil(&last_wake, half_period);
    }
}

int main(void)
{
    static const char *names[3] = {
        "LED_0.1Hz", "LED_1Hz", "LED_10Hz"
    };

    Clock_Init();
    GPIO_Init();

    for (uint8_t i = 0; i < 3U; i++)
    {
        BaseType_t result = xTaskCreate(
            LED_Blink,       /* Hàm thực hiện task */
            names[i],        /* Tên task */
            256,             /* Stack: 256 word = 1024 byte */
            &led_config[i],  /* Pin và tần số riêng */
            1,               /* Ba task cùng mức ưu tiên */
            NULL
        );

        if (result != pdPASS)
        {
            /* Không đủ bộ nhớ để tạo task. */
            while (1) {}
        }
    }

    vTaskStartScheduler();

    /* Chỉ tới đây nếu scheduler không khởi động được. */
    while (1) {}
}
