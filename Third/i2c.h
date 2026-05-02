#ifndef JY_I2C_H
#define JY_I2C_H

#include "main.h"

typedef enum 
{
	AddrType_7bit,
    AddrType_8bit,
}I2C_AddrType;




typedef struct  
{
    vu8              Addr;
    I2C_AddrType     AddrType;
    vu32             GPIO_SCL;
    vu32             PIN_SCL;
    vu32             GPIO_SDA;
    vu32             PIN_SDA;
    vu16             Delay;
}SW_I2C_t;


void JY_I2C_INIT(SW_I2C_t *I2C_Param);

////连续读数据，写方式发送地址后，就读len个字节在buf中，适合TEA5767 等 ，没有寄存器，直接读入N个字节的器件
u8 I2C_WriteData(const SW_I2C_t *I2C_Param, u8 *buf, u16 len);    
u8 I2C_ReadData(const SW_I2C_t *I2C_Param, u8 *buf, u16 len); 

u8 I2C_ReadBytes(const SW_I2C_t *I2C_Param, u8 REG_Addr, u8 *buf, u8 len);
u8 I2C_WriteBytes(const SW_I2C_t *I2C_Param, u8 REG_Addr, u8 *buf, u8 len);
u8 I2C_SingleWrite(const SW_I2C_t *I2C_Param, u8 REG_Addr, u8 data);
u8 I2C_SingleRead(const SW_I2C_t *I2C_Param, u8 REG_Addr);//？


#endif

