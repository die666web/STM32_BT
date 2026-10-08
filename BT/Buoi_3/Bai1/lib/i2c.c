#include "i2c.h"
#include "rcc.h"
#include "gpio.h"
#include "uart.h"

// Bit	Tên	Ý nghĩa
// 0	SB	START đã được tạo
// 1	ADDR	Đã gửi/nhận địa chỉ thành công
// 2	BTF	Byte transfer finished
// 6	RXNE	Đã nhận byte mới
// 7	TXE	Thanh ghi truyền đang trống
// 8	BERR	Lỗi bus
// 9	ARLO	Mất quyền điều khiển bus
// 10	AF	Không nhận được ACK
// 11	OVR	Overrun/underrun

typedef enum{
    FLAG_START =  1 << 0,
    FLAG_ADDR = 1 << 1,
    FLAG_BTF = 1 << 2,
    FLAG_RXNE = 1 << 6,
    FLAG_TXE = 1 << 7,
} Flag_typedef;

typedef enum
{
    Error_BERR = 1 << 8,
    Error_ARLO = 1 << 9,
    Error_AF = 1 << 10,
    Error_OVR = 1 << 11,
}Error_typedef;

static I2C_Status I2C_Start(void);
static I2C_Status I2C_SendAddress(uint8_t address, uint8_t read);
static I2C_Status I2C_CheckError(void);
static void I2C_Stop(void);

/**
 * @brief 
 * 
 * @param pck1 
 * @param i2c_clk 
 */
I2C_Status I2C_Init(uint32_t pclk1, uint32_t i2c_clk)
{
    // Bat xung clock APB1
    RCC_EnableAPB1Clock(RCC_APB1_I2C1);

    // Chan GPIOB 6 va 7 deu che do Open drain 
    GPIO_ConfigPin(GPIOB, 6, GPIO_MODE_AF_OD, GPIO_SPEED_50MHZ);
    GPIO_ConfigPin(GPIOB, 7, GPIO_MODE_AF_OD, GPIO_SPEED_50MHZ);

    //Reset CR1
    I2C->CR1 = 0U;

    // Cai dat xung nhip cua stm32
    I2C->CR2 = pclk1;  // Dien 8U neu laf 8Mhz

    // Cau hinh toc do bus SCL
    uint32_t speed_scl = ( pclk1 * 1000 / ( 2 * i2c_clk) );
    I2C->CCR = speed_scl; // Dien 100. Theo cong thuc  ccr = fpclk / (2 * i2c_speed)

    // Cau hinh toc do xung len
    I2C->TRISE = pclk1 + 1; // Neu dat toc do tu low -> high laf 1000ms -> coong thuc se laf = pclk MHz + 1

    // Bat I2C len
    I2C->CR1 |=  1 << 0;
}

I2C_Status I2C_Write(uint32_t slave_address, const uint8_t *data, uint16_t length)
{
    // Bat start
    I2C_Start();

    // Truyen dia chi 
    I2C_SendAddress(slave_address,0);

    // Truyen data
    while(length > 0)
    {
        length--;
        I2C->DR = *data;
        ++data; 
    }
    
    // Bat stop
    I2C_Stop();

    I2C_Status status  = I2C_CheckError();

    if(status == I2C_Ok)
    {
        return I2C_Ok;
    }
    else
    {
        return status; 
    }

}


I2C_Status I2C_Read(uint32_t slave_address, uint8_t *data, uint16_t length)
{
    // Bat start
    I2C_Start();

    // Truyen dia chi 
    I2C_SendAddress(slave_address, 1);

    // Doc du lieu 
    I2C->DR = *data;

    // Kiem tra ACK
    I2C_Status status  = I2C_CheckError();
    if(status == I2C_Ok)
    {
        return I2C_Ok;
    }
    else
    {
        return status; 
    }

    // Bat stop 
    I2C_Stop();
}

static I2C_Status I2C_WaitBusyFree(void)
{
    uint32_t t =  100000000U;
    while((I2C->SR2 & (1 << 1U)) != 0)
    {
        --t;
        if(t == 0) return I2C_ERROR_TIMEOUT;
    }
    return I2C_Ok;
}

static I2C_Status I2C_WaitFlag(Flag_typedef flag)
{
    uint32_t time_out = 10000000;

    while((I2C->SR1 & flag) == 0)
    {
        --time_out;
        I2C_Status status = I2C_CheckError();
        if(status != I2C_Ok)
        {
            return status;
        }
        if(time_out == 0)
        {
            return I2C_ERROR_TIMEOUT;
        }
    }
    return I2C_Ok;
}

// 8	BERR	Lỗi bus
// 9	ARLO	Mất quyền điều khiển bus
// 10	AF	Không nhận được ACK
// 11	OVR	Overrun/underrun

static uint8_t I2C_IsErrorSet(Error_typedef error)
{
    if(I2C->SR1 & (error))
    {
        return 1;
    }
    return 0;
}

static I2C_Status I2C_CheckError(void)
{
    uint32_t status = I2C->SR1;

    if(status & Error_BERR) return I2C_ERROR_BUS;

    if(status & Error_ARLO) return I2C_ERROR_ARL0;

    if(status & Error_AF) return I2C_ERROR_AF;

    if(status & Error_OVR) return I2C_ERROR_OVR;

    return I2C_Ok;
}

static I2C_Status I2C_Start(void)
{
    I2C_Status status = I2C_WaitBusyFree();

    if (status != I2C_Ok)
        return status;

    I2C->CR1 |= (1U << 8);
    return I2C_WaitFlag(FLAG_START);
}

static I2C_Status I2C_SendAddress(uint8_t address, uint8_t read)
{
    if(I2C->SR1 & FLAG_START)
    {
        I2C->DR  = ((uint32_t)address << 1) | read; 
    }

    return I2C_WaitFlag(FLAG_ADDR);
}

static void I2C_Stop(void)
{
    I2C->CR1 |= 1 << 9;
}
