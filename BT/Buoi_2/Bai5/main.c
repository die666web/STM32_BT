#include <stdint.h>

#define RCC_BASE 0x40021000UL
#define GPIOA_BASE 0x40010800UL
#define UART1_BASE 0x40013800UL
#define TIM2_BASE 0x40000000UL

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_Typedef;

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_Typedef;

typedef struct {
    volatile uint32_t SR;
    volatile uint32_t DR;
    volatile uint32_t BRR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t GTPR;
} UART1_Typedef;

typedef struct
{
    volatile uint32_t CR1;      // 0x00
    volatile uint32_t CR2;      // 0x04
    volatile uint32_t SMCR;     // 0x08
    volatile uint32_t DIER;     // 0x0C
    volatile uint32_t SR;       // 0x10
    volatile uint32_t EGR;      // 0x14
    volatile uint32_t CCMR1;    // 0x18
    volatile uint32_t CCMR2;    // 0x1C
    volatile uint32_t CCER;     // 0x20
    volatile uint32_t CNT;      // 0x24
    volatile uint32_t PSC;      // 0x28
    volatile uint32_t ARR;      // 0x2C

    volatile uint32_t RESERVED; // 0x30

    volatile uint32_t CCR1;     // 0x34
    volatile uint32_t CCR2;     // 0x38
    volatile uint32_t CCR3;     // 0x3C
    volatile uint32_t CCR4;     // 0x40

    volatile uint32_t RESERVED2;// 0x44
    volatile uint32_t DCR;      // 0x48
    volatile uint32_t DMAR;     // 0x4C
} Tim2_Typedef;


#define RCC ((RCC_Typedef *)RCC_BASE)
#define GPIOA ((GPIO_Typedef *)GPIOA_BASE)
#define UART1 ((UART1_Typedef *)UART1_BASE)
#define TIM2 ((Tim2_Typedef *)TIM2_BASE)

#define NVIC_ISER1 (*(volatile uint32_t *)0xE000E104UL)

#define UART_MAX_BUFFER 128
#define SYSTEM_CLOCK    8000000 // Tần số System Clock (8MHz)
#define PWM_FREQ        1000UL     // Tần số PWM mong muốn (1kHz)
#define PWM_PERIOD      1000U      // Tương ứng độ phân giải duty cycle (0 -> 1000)

volatile uint8_t uart1_rx_buffer[UART_MAX_BUFFER];
volatile uint8_t rx_head = 0;
volatile uint8_t rx_tail = 0; 

volatile uint8_t uart1_tx_buffer[UART_MAX_BUFFER];
volatile uint8_t tx_head = 0;
volatile uint8_t tx_tail = 0;

uint8_t led_percent = 100;
uint8_t led_is_on = 0;


void delay(uint32_t time)
{
    for(uint32_t i = 0; i < time; i++)
    {
        for(uint32_t j = 0; j < 1000U; j++)
        {}
    }
}

/*brief 
- Cấp xung RCC: APB2, bit 2 là GPIOA, bit 14 là USART1

- Pa9 - Chế độ alternate function push up - 50 mhzz: 1011
- Pa10 - chế  độ inputfloating - 0100 

- Cài đặt baudrate: thanh ghi BRR -> f_pclk / baud = 8MHz/ 9600 = 833
- Bật UART: thanh ghi CR1 -> bật cờ: 13, bộ  nhận: 2, bộ phát: 3

- Cấu hình ngắt NVIC: bật usart1 -> 37.
*/
void Config_uart(void)
{ 
    RCC->APB2ENR |= 1 << 2 | 1 << 14;
    
    GPIOA->CRH &= ~(0xF << 4 | 0xF << 8);
    GPIOA->CRH |= 0xB << 4 | 0x4 << 8;

    UART1->BRR |= 833;
    UART1->CR1 |= (1U << 2)   // RE
            | (1U << 3)   // TE
            | (1U << 5)   // RXNEIE  <-- thêm cái này
            | (1U << 13); // UE

    NVIC_ISER1 |= 1 << 5;
}

/**
 * @brief Khởi tạo Timer 2 và cấu hình kênh PWM theo Channel được chọn
 * @param channel: Kênh PWM (1, 2, 3, hoặc 4)
 * @param duty_cycle: Giá trị Duty Cycle khởi tạo (0 -> PWM_PERIOD)
 */
void Config_Timer(uint8_t channel, uint16_t duty_cycle)
{
    // 1. Bật Clock cho GPIOA (bit 2 APB2ENR) và TIM2 (bit 0 APB1ENR)
    RCC->APB2ENR |= (1 << 2);
    RCC->APB1ENR |= (1 << 0);

    // 2. Cấu hình chân GPIO tương ứng ở chế độ AF Push-Pull 50MHz (0xB)
    switch (channel)
    {
        case 1: // PA0
            GPIOA->CRL &= ~(0xFU << 0);
            GPIOA->CRL |=  (0xBU << 0);
            break;
        case 2: // PA1
            GPIOA->CRL &= ~(0xFU << 4);
            GPIOA->CRL |=  (0xBU << 4);
            break;
        case 3: // PA2
            GPIOA->CRL &= ~(0xFU << 8);
            GPIOA->CRL |=  (0xBU << 8);
            break;
        case 4: // PA3
            GPIOA->CRL &= ~(0xFU << 12);
            GPIOA->CRL |=  (0xBU << 12);
            break;
        default:
            return; // Channel không hợp lệ
    }

    // 3. Cấu hình Tần số PWM (1kHz)
    // f_timer = f_clk / ((PSC + 1) * (ARR + 1))
    TIM2->PSC = (SYSTEM_CLOCK / (PWM_FREQ * PWM_PERIOD)) - 1; // PSC = 71
    TIM2->ARR = PWM_PERIOD - 1;                                // ARR = 999

    // 4. Cấu hình PWM Mode 1 (110) và Bật Preload (OCxPE)
    switch (channel)
    {
        case 1:
            TIM2->CCMR1 &= ~(0xFFU << 0);
            TIM2->CCMR1 |=  (6 << 4) | (1 << 3); // OC1M = 110 (PWM Mode 1), OC1PE = 1
            TIM2->CCER  |=  (1 << 0);            // CC1E = 1 (Enable Channel 1 Output)
            TIM2->CCR1   =  duty_cycle;          // Đặt Duty Cycle ban đầu
            break;

        case 2:
            TIM2->CCMR1 &= ~(0xFFU << 8);
            TIM2->CCMR1 |=  (6 << 12) | (1 << 11); // OC2M = 110, OC2PE = 1
            TIM2->CCER  |=  (1 << 4);              // CC2E = 1 (Enable Channel 2 Output)
            TIM2->CCR2   =  duty_cycle;
            break;

        case 3:
            TIM2->CCMR2 &= ~(0xFFU << 0);
            TIM2->CCMR2 |=  (6 << 4) | (1 << 3); // OC3M = 110, OC3PE = 1
            TIM2->CCER  |=  (1 << 8);            // CC3E = 1 (Enable Channel 3 Output)
            TIM2->CCR3   =  duty_cycle;
            break;

        case 4:
            TIM2->CCMR2 &= ~(0xFFU << 8);
            TIM2->CCMR2 |=  (6 << 12) | (1 << 11); // OC4M = 110, OC4PE = 1
            TIM2->CCER  |=  (1 << 12);             // CC4E = 1 (Enable Channel 4 Output)
            TIM2->CCR4   =  duty_cycle;
            break;
    }

    // 5. Bật Auto-reload Preload (ARPE) và Tạo sự kiện Update Event
    TIM2->CR1 |= (1 << 7); // Bit ARPE
    TIM2->EGR |= (1 << 0); // Bit UG (Update Generation)

    // 6. Cho phép Timer 2 bắt đầu đếm (CEN = 1)
    TIM2->CR1 |= (1 << 0);
}

/**
 * @brief Thay đổi Duty Cycle PWM của Timer 2 khi chương trình đang chạy
 * @param channel: Kênh PWM (1, 2, 3, hoặc 4)
 * @param duty_cycle: Giá trị Duty Cycle mới (0 -> PWM_PERIOD)
 */
void ControlPWM_Timer2(uint8_t channel, uint16_t duty_cycle)
{
    // Giới hạn giá trị max để tránh tràn xung
    if (duty_cycle > PWM_PERIOD)
    {
        duty_cycle = PWM_PERIOD;
    }

    // Gán trực tiếp giá trị vào thanh ghi Capture/Compare (CCR) tương ứng
    switch (channel)
    {
        case 1:
            TIM2->CCR1 = duty_cycle;
            break;
        case 2:
            TIM2->CCR2 = duty_cycle;
            break;
        case 3:
            TIM2->CCR3 = duty_cycle;
            break;
        case 4:
            TIM2->CCR4 = duty_cycle;
            break;
        default:
            break;
    }
}

/**
 * @brief 
 * Nhiem vu tinh vi tri tiep theo trong gui du lieu 
 * @param index: chi so hien tai
 * @return uint16_t : in ra so tiep theo 
 */
uint16_t UART_NexInex(uint8_t index)
{
    index++;
    if(index == UART_MAX_BUFFER) index = 0;
    return index;
}

/**
 * @brief 
 * Quan tâm tới 2 thanh ghi CR1 và SR
 * CR1: thanh ghi CR1 thì mk chỉ quan tâm tới 2,3,5,6,7,13 tương ứng RE, TE, IDLETE, RXNEIE, TCIE, TXEIE, Enable Uart
 * SR: thanh ghi này mk chỉ quan tâm 4,5,6,7. tương ứng IDLE, RXNE, TC, TXE. 
 * Nhiệm vụ của hàm là bật cờ truyền ngắt, set cờ bận lên 1 và truyền data_byte vào tx_data
 * @param data : dữ liệu được gửi đi - tương ứng 1 byte
 */
uint8_t SendInterrupt_UART(uint8_t data)
{
    uint16_t next = UART_NexInex(tx_head);

    if(next == tx_tail) return 0;

    uart1_tx_buffer[tx_head] = data;
    tx_head = next;

    UART1->CR1 |= 1 << 7;
    return 1; 
}

/**
 * @brief 
 * Nhiệm vụ của hàm là gửi dữ liệu, set cờ bận về  0 và set cờ ngắt về  0 
 */
void USART1_IRQHandler(void){
    // Check trang thai phat va co ngat 
    if((UART1->SR & (1 << 7)) && (UART1->CR1 & (1 << 7)))
    {
        if(tx_tail != tx_head)
        {
            UART1->DR = uart1_tx_buffer[tx_tail];
            tx_tail = UART_NexInex(tx_tail);
        }
        if(tx_tail == tx_head)
        {
            UART1->CR1 &= ~(1 << 7);
        }
    }

    if ((UART1->SR & (1U << 5)) && (UART1->CR1 & (1U << 5)))
    {
        uint8_t receive = (uint8_t)UART1->DR;
        uint16_t next = UART_NexInex(rx_head);

        if(next != rx_tail)
        {
            uart1_rx_buffer[rx_head] = receive;
            rx_head = next;
        }
    }
}

/**
 * @brief 
 * Nhiệm vụ là gửi cả 1 chuỗi dài 
 * @param str: chuỗi định 
 */
void SendStringInterrupt_UART(const char *str)
{
    while(*str != '\0')
    {
        while(!SendInterrupt_UART((uint8_t)*str)); // Giá trị của con trỏ tại vij trí đấy 
        str++; // Câu lệnh này thực chất là ko thay đổi giá trị mà là thay đổi địa chỉ của str -> 0,1,2,3,
    }
    while(!SendInterrupt_UART('\n'));
}

uint8_t ReceiveInterrupt_UART(uint8_t *out_data)
{
    if(rx_tail == rx_head) return 0;

    *out_data = uart1_rx_buffer[rx_tail];
    rx_tail = UART_NexInex(rx_tail);

    return 1;
}

void GPIO_Init(void)
{
    RCC->APB2ENR |=  1 << 2; 
    GPIOA->CRL &= ~(0xF << 4); 
    GPIOA->CRL |= 0x3 << 4;

}

void LED_SetPercent(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }

    led_percent = percent;

    /* Neu den dang tat, chi luu muc PWM, khong bat den. */
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

uint8_t StringEqual(const char *s1, const char *s2)
{
    while (*s1 != '\0' && *s2 != '\0')
    {
        if (*s1 != *s2)
        {
            return 0;
        }

        s1++;
        s2++;
    }

    return (*s1 == '\0' && *s2 == '\0');
}

uint8_t UART_ReadCommand(char *command, uint8_t max_size)
{
    static uint8_t index = 0;
    uint8_t data;

    if (!ReceiveInterrupt_UART(&data))
    {
        return 0;
    }

    /* Bo qua CR/LF sau khi terminal gui lenh. */
    if (data == '\r' || data == '\n')
    {
        index = 0;
        command[0] = '\0';
        return 0;
    }

    if (index >= max_size - 1)
    {
        index = 0;
        command[0] = '\0';
        return 0;
    }

    command[index++] = (char)data;
    command[index] = '\0';

    /* Lenh chu duoc xu ly ngay khi nhan du ky tu. */
    if (StringEqual(command, "on") ||
        StringEqual(command, "off") ||
        StringEqual(command, "status"))
    {
        index = 0;
        return 1;
    }

    /* Lenh PWM ket thuc tai dau %. */
    if (index >= 6 &&
        command[0] == 'P' &&
        command[1] == 'W' &&
        command[2] == 'M' &&
        command[3] == ':' &&
        data == '%')
    {
        index = 0;
        return 1;
    }

    return 0;
}

uint8_t ParsePWM(const char *command, uint8_t *percent)
{
    uint8_t index = 4;
    uint16_t value = 0;

    if (command[0] != 'P' ||
        command[1] != 'W' ||
        command[2] != 'M' ||
        command[3] != ':')
    {
        return 0;
    }

    if (command[index] < '0' || command[index] > '9')
    {
        return 0;
    }

    while (command[index] >= '0' && command[index] <= '9')
    {
        value = value * 10U +
                (uint16_t)(command[index] - '0');

        if (value > 100U)
        {
            return 0;
        }

        index++;
    }

    if (command[index] != '%' || command[index + 1] != '\0')
    {
        return 0;
    }

    *percent = (uint8_t)value;
    return 1;
}

void SendStatus(void)
{
    char status[24];
    const char *text;
    uint8_t index = 0;

    if (led_is_on)
    {
        text = "LED ON PWM: ";
    }
    else
    {
        text = "LED OFF PWM: ";
    }

    /* Dua phan chu vao cung mot chuoi status. */
    while (*text != '\0')
    {
        status[index++] = *text++;
    }

    /* Dua gia tri PWM vao chinh chuoi status. */
    if (led_percent == 100)
    {
        status[index++] = '1';
        status[index++] = '0';
        status[index++] = '0';
    }
    else if (led_percent >= 10)
    {
        status[index++] =
            (char)('0' + led_percent / 10U);
        status[index++] =
            (char)('0' + led_percent % 10U);
    }
    else
    {
        status[index++] =
            (char)('0' + led_percent);
    }

    status[index++] = '%';
    status[index] = '\0';

    /* Gui mot chuoi hoan chinh duy nhat. */
    SendStringInterrupt_UART(status);
}

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
        if (UART_ReadCommand(command, sizeof(command)))
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
        }
    }
}
