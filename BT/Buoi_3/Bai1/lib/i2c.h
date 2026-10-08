#ifndef I2C_H
#define I2C_H

#include<stdint.h>

#define I2C_BASE 0x400001020

/**
 * @brief Explain for feature of register
 * CR1: Dùng để cấu hình hoạt động chính cảu I2C. Cacs bit quan trọng 0 bịt ngoại vi, 8 start, 9 stop, 10 ack, 11 pos và 15  
 * CR2: Chứa tần số PCLK1 tính theo MHz, Nó cho bk clock của APB1 hiện tịa là bao niêu để tính timming nội bộ, ko quết đinhj tốc độ truyền
 * OAR1, OAR2 là địa chỉ 1 và 2 khi stm32 làm slave 
 * DR: thanh ghi dât 32 bit, 4 byte
 * SR1: Thanh ghi cờ, tự xem để nhớ ko nhớ hết được quá nhiều 0 - Start, 6 cho RXNE, 7 cho TXE, 2 BTF là truyền đã 
 * SR2: bit 0 bật lên 1 thì stm32 làm master, 1 thì có nghĩa là data bus đang bận - busy, tra : stm32 đang truyền hoặc nhận
 * CCR: cái này mới quy định tốc đọ SCL. CCR = PCLK1/ (2*tốc độ I2C)
 * TRISE: Hiểu nôm na là thời gian tnawg tói đa của tín hiệu khi kéo từ 0 -> 1.
 */

typedef struct
{
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t OAR1;
    volatile uint32_t OAR2;
    volatile uint32_t DR;
    volatile uint32_t SR1;
    volatile uint32_t SR2;
    volatile uint32_t CCR;
    volatile uint32_t TRISE;
} I2C_TypeDef;

#define I2C ((I2C_TypeDef *)I2C_BASE)

/**
 * @brief Kieu du lieu I2C_Status
 * Kieu du lieu nay cos 8 gia tri
 * ok xacs nhanj i2c truyen nhan thanh cong 
 * Cac error de doc loi 
 */
typedef enum{
    I2C_Ok = 0,

    I2C_ERROR_BUS = 1, // Bus gui bij loi do stop hoac start dduwojc nhan nhung ko dung
    I2C_ERROR_ARL0 = 2, // Maast quyen lam chu bus. Bi chiem quyen lam masster
    I2C_ERROR_AF = 3, // Loi ko nhan duoc ACK khi nhan ban tin gui len
    I2C_ERROR_OVR = 4 ,// loi bi tran

    I2C_ERROR_BUSY = 5, // bus dang ban hay ko 
    I2C_ERROR_INVALID = 6, //loi tham so truyen vao ko phu hop 
    I2C_ERROR_TIMEOUT = 7, // qua thoi gian nhan hoac gui
} I2C_Status;

typedef enum{
    flag_start,
    flag_addr,
    flag_btf,
    flag_rxne,
    flag_txe
} Flag_typedef;

// Thuw vieenj public 
I2C_Status I2C_Init(uint32_t pclk1, uint32_t i2c_clk);
I2C_Status I2C_Write(uint32_t slave_address, const uint8_t *data, uint16_t length);
I2C_Status I2C_Read(uint32_t slave_address, uint8_t *data, uint16_t length);


#endif