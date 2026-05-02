#include "i2c.h"


#define   I2C_LOW(GPIO, PIN)   		GPIO_BC(GPIO) = (uint32_t)PIN  
#define   I2C_HIGH(GPIO, PIN)  		GPIO_BOP(GPIO) = (uint32_t)PIN    	
//#define   I2C_LOW(GPIO, PIN)   		gpio_init(GPIO, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, PIN)
//#define   I2C_HIGH(GPIO, PIN)  		gpio_init(GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, PIN)


#define  ACK     0
#define  NACK    1


__STATIC_INLINE void Set_SDAOUT(const SW_I2C_t *I2C_Param);  //配置 SDA引脚 为输出模式
__STATIC_INLINE void Set_SDAIN(const SW_I2C_t *I2C_Param);	//配置 SDA引脚 为输入模式

__STATIC_INLINE void I2C_Start(const SW_I2C_t *I2C_Param);		//产生IIC起始信号
__STATIC_INLINE void I2C_Stop(const SW_I2C_t *I2C_Param);		//产生IIC停止信号
__STATIC_INLINE u8 I2C_Wait_Ack(const SW_I2C_t *I2C_Param);
__STATIC_INLINE void I2C_Ack(const SW_I2C_t *I2C_Param);
__STATIC_INLINE void I2C_NAck(const SW_I2C_t *I2C_Param);

void JY_I2C_INIT(SW_I2C_t *I2C_Param)
{
    if(I2C_Param->AddrType == AddrType_7bit)
        I2C_Param->Addr = (I2C_Param->Addr << 1) & 0xFE;

    gpio_init(I2C_Param->GPIO_SCL, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2C_Param->PIN_SCL);
	gpio_init(I2C_Param->GPIO_SDA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2C_Param->PIN_SDA);
	
//    gpio_init(I2C_Param->GPIO_SCL, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, I2C_Param->PIN_SCL);
//	gpio_init(I2C_Param->GPIO_SDA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, I2C_Param->PIN_SDA);
	
    I2C_HIGH(I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);
    I2C_HIGH(I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);
}



__STATIC_INLINE void Set_SDAOUT(const SW_I2C_t *I2C_Param)  //配置 SDA引脚 为输出模式
{  
    gpio_init(I2C_Param->GPIO_SDA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, I2C_Param->PIN_SDA);
//	gpio_init(I2C_Param->GPIO_SDA, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, I2C_Param->PIN_SDA);
}

__STATIC_INLINE void Set_SDAIN(const SW_I2C_t *I2C_Param)	//配置 SDA引脚 为输入模式
{
    gpio_init(I2C_Param->GPIO_SDA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, I2C_Param->PIN_SDA);
}

/****数据接收用的  SDA*************************/
__STATIC_INLINE bool I2C_SDA_Read(const SW_I2C_t *I2C_Param)	//		((NRF_GPIO->IN >> I2C_SDA_PIN) & 1ul)    //nrf_gpio_pin_read(I2C_SDA_PIN)     
{
    return ((GPIO_ISTAT(I2C_Param->GPIO_SDA) & (I2C_Param->PIN_SDA)) != 0);
}



static void Delay(u16 X)      //延时，可以降低 IIC的速度
{ 
//	u16 X = 50;        //40
	while(X--);
}

static void DelayH(u16 X)      //半延时
{ 
	X >>= 1; 
	while(X--);
}

__STATIC_INLINE void I2C_Start(const SW_I2C_t *I2C_Param)		//产生IIC起始信号
{
	Set_SDAOUT  (I2C_Param);	// SDA线输出，完成后会变高
	I2C_HIGH    (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);   
	Delay       (I2C_Param->Delay);		
	I2C_HIGH    (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);
	Delay       (I2C_Param->Delay);
 	I2C_LOW     (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA); 		//START:when CLK is high,DATA change form high to low 
	Delay       (I2C_Param->Delay);
	I2C_LOW     (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);		//钳住I2C总线，准备发送或接收数据 
}	


__STATIC_INLINE void I2C_Stop(const SW_I2C_t *I2C_Param)	    //产生IIC停止信号********重点BUG
{
	Set_SDAOUT  (I2C_Param);	// SDA线输出，完成后会变高
	DelayH      (I2C_Param->Delay);
	I2C_LOW     (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);
    DelayH      (I2C_Param->Delay);
	I2C_LOW     (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA); 		//可以同时变低---------上一句无延时，此处会有一个冲击（慢速时）
	DelayH      (I2C_Param->Delay);		//延时
	I2C_HIGH    (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);		//先把时钟 = 1
	Delay       (I2C_Param->Delay);
	I2C_HIGH    (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);		//时钟=1期间，SDA来个上升沿---发送I2C总线结束信号	
    DelayH      (I2C_Param->Delay);    
}


//主机 产生ACK应答  --- 此时SDA处于 接收状态
__STATIC_INLINE void I2C_Ack(const SW_I2C_t *I2C_Param)
{
	I2C_LOW     (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);    
	Set_SDAOUT  (I2C_Param);	                                // SDA线输出，完成后会变高 
	I2C_LOW     (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA); 		// SDA = 0----上一句无延时 且 最后一个位收到0，主机应答这里会有一个冲击（慢速时候）
	Delay       (I2C_Param->Delay); 
	I2C_HIGH    (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);		//然后 SCL来个上升沿
	Delay       (I2C_Param->Delay); 
	I2C_LOW     (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);	    //此句后无延时 且 收下一字节首位为0，会有一个冲击
}

//主机不产生ACK应答	-- 产生 NACK 信号	    --- 此时SDA处于 接收状态
__STATIC_INLINE void I2C_NAck(const SW_I2C_t *I2C_Param)
{
	I2C_LOW     (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);
    I2C_HIGH    (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);		//    20180828 和后一句调换位置
	Set_SDAOUT  (I2C_Param);	                                // SDA线输出，完成后会变高	
	Delay       (I2C_Param->Delay); 
	I2C_HIGH    (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);		//然后 SCL来个上升沿
	Delay       (I2C_Param->Delay); 
	I2C_LOW     (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);
}


//等待 从机 应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
__STATIC_INLINE u8 I2C_Wait_Ack(const SW_I2C_t *I2C_Param)
{
	vu32 ucErrTime = 0;
	I2C_HIGH    (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);		//发送 1 Byte 结束后已经把SDA置 1 这里可以
	Set_SDAIN   (I2C_Param);                                    // SDA为输入
	
	Delay       (I2C_Param->Delay);      	  
	I2C_HIGH    (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);
	Delay       (I2C_Param->Delay); 
	
	while(I2C_SDA_Read(I2C_Param))	//
	{
		ucErrTime++;
		if(ucErrTime > 250000)
		{
			I2C_Stop(I2C_Param);
			return 1;
		}
	}
	I2C_LOW(I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);			//时钟输出0 	   
	return 0; 
} 					 	

//IIC发送一个字节		  
__STATIC_INLINE void I2C_SendByte(const SW_I2C_t *I2C_Param, u8 txd)   //发送一个字节后，应该等到从机应答，
{                        
    u8 t;   
	
	{
		Set_SDAOUT  (I2C_Param);			// SDA线输出，完成后会变高  
		DelayH      (I2C_Param->Delay);
		I2C_LOW     (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);				//拉低时钟开始数据传输
		DelayH      (I2C_Param->Delay);
		for(t = 0; t < 8; t++)
		{                      
			if((txd & 0x80) == 0x80)	//从高位开始，把数据放在 SDA上
				I2C_HIGH(I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);
			else
				I2C_LOW (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);
			txd <<= 1; 	  
			DelayH  (I2C_Param->Delay);
			I2C_HIGH(I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);				//SCL变高上升沿，提醒从机，SDA数据有效
			Delay   (I2C_Param->Delay); 			//延时，给从机时间读取数据
			I2C_LOW (I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);				//SCL变低，再次可以改变SDA数据      
			DelayH  (I2C_Param->Delay);
		}
		I2C_HIGH    (I2C_Param->GPIO_SDA, I2C_Param->PIN_SDA);  //硬件有上拉电阻则可以不要
	}

	//发送一个 8 bit 后，SCL = 0   SDA = x
} 


//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
__STATIC_INLINE u8 I2C_ReadByte(const SW_I2C_t *I2C_Param, u8 ack)
{
	uint8_t i, receive=0;

	{
		Set_SDAIN   (I2C_Param);     		// SDA为输入，配置完后会变低
		Delay       (I2C_Param->Delay);
		for(i = 0; i < 8; i++)
		{
			I2C_LOW(I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL); 			// SCL = 0 让从机改变 SDA上的数据
			Delay(I2C_Param->Delay);			
			I2C_HIGH(I2C_Param->GPIO_SCL, I2C_Param->PIN_SCL);			// SCL = 1 提示从机，主机要读数据，不能改变数据
			DelayH(I2C_Param->Delay);
			receive <<= 1;			// 右移 1 位
			if(I2C_SDA_Read(I2C_Param))	//不能写 == 1，应该按库函数的规则写
				receive++;  		// 就把最低位 变成 1
			DelayH(I2C_Param->Delay);
		}			
		//******读取 8 bit 完成时，SCL = 1   SDA = x	
		if (ack == ACK)
			I2C_Ack(I2C_Param); //发送ACK 
		else
			I2C_NAck(I2C_Param);//发送nACK
	}

    return receive;
}


//**************************************************************************对外接口***********************************

//连续写数据，写方式发送地址后，就写len个字节在buf中，适合TEA5767 等 ，没有寄存器，直接写入5个字节的器件
u8 I2C_WriteData(const SW_I2C_t *I2C_Param, u8 *buf, u16 len)    
{
	vu8 Tx_Num = 0; 
	I2C_Start(I2C_Param);
	I2C_SendByte(I2C_Param, (I2C_Param->Addr | 0 ) );//发送器件地址+写命令
	if(I2C_Wait_Ack(I2C_Param))	//等待应答
	{
		return 1;		 //应答超时，放弃写数据
	}
	for(Tx_Num = 0; Tx_Num < len; Tx_Num++)
	{
		I2C_SendByte(I2C_Param, buf[Tx_Num] );	//发送数据
		if( I2C_Wait_Ack(I2C_Param) )				//等待ACK  等到应答超时则放弃后面的数据
		{
			return 1;		 
		}		
	}    
    I2C_Stop(I2C_Param);	 						//发送停止，提示从机，数据发送完毕
	return 0;
}

//连续读数据，写方式发送地址后，就读len个字节在buf中，适合TEA5767 等 ，没有寄存器，直接读入5个字节的器件
u8 I2C_ReadData(const SW_I2C_t *I2C_Param, u8 *buf, u16 len)  
{
	I2C_Start(I2C_Param);	
	I2C_SendByte(I2C_Param, (I2C_Param->Addr | 1));//发送器件地址 + 读命令
	I2C_Wait_Ack(I2C_Param);	//等待应答
	while(len)
	{
		if(len == 1)
			*buf = I2C_ReadByte(I2C_Param, NACK);//读数据,发送nACK 
		else 
			*buf = I2C_ReadByte(I2C_Param, ACK);		//读数据,发送ACK  
		len --;
		buf ++; 
	} 
	I2C_Stop(I2C_Param);	 						//发送停止，提示从机，数据发送完毕
	return 0;
}










//写数据：从机地址，从机内一个寄存器，写入数据指针，数据长度-----适合有寄存器的器件，EEPROM，传感器，等
u8 I2C_WriteBytes(const SW_I2C_t *I2C_Param, u8 REG_Addr, u8 *buf, u8 len)
{
	vu8 Tx_Num = 0; 
	{
		I2C_Start(I2C_Param);
		I2C_SendByte(I2C_Param, (I2C_Param->Addr | 0));//发送器件地址+写命令
		if(I2C_Wait_Ack(I2C_Param))	//等待应答
		{	 
			return 1;		//应答超时，放弃写数据
		}
		I2C_SendByte(I2C_Param, REG_Addr);	//写寄存器地址
		I2C_Wait_Ack(I2C_Param);		//等待应答
		for(Tx_Num = 0; Tx_Num < len; Tx_Num++)
		{
			I2C_SendByte(I2C_Param, buf[ Tx_Num ] );	//发送数据
			if( I2C_Wait_Ack(I2C_Param) )				//等待ACK  等到应答超时则放弃后面的数据
			{
				return 1;		 
			}		
		}    
		I2C_Stop(I2C_Param);	 						//发送停止，提示从机，数据发送完毕
	}
	return 0;
}

u8 I2C_ReadBytes(const SW_I2C_t *I2C_Param, u8 REG_Addr, u8 *buf, u8 len)  
{
	{
		I2C_Start(I2C_Param);
		I2C_SendByte(I2C_Param, (I2C_Param->Addr | 0 ) );//发送器件地址+写命令
		if(I2C_Wait_Ack(I2C_Param))	//等待应答
		{
			//SW_I2C_Stop();		 //应答超时，放弃写数据
			return 1;		
		}
		I2C_SendByte(I2C_Param, REG_Addr);	//写寄存器地址
		I2C_Wait_Ack(I2C_Param);		//等待应答
		
		I2C_Start(I2C_Param);	
		I2C_SendByte(I2C_Param, (I2C_Param->Addr ) | 1 );//发送器件地址 + 读命令
		I2C_Wait_Ack(I2C_Param);	//等待应答
		while(len)
		{
			if(len == 1)
				*buf = I2C_ReadByte(I2C_Param, NACK);//读数据,发送nACK 
			else 
				*buf = I2C_ReadByte(I2C_Param, ACK);		//读数据,发送ACK  
			len --;
			buf ++; 
		} 
		I2C_Stop(I2C_Param);	 						//发送停止，提示从机，数据发送完毕
	}
	return 0;
}









u8 I2C_SingleWrite(const SW_I2C_t *I2C_Param, u8 REG_Addr, u8 data)
{
	vu8 Tx_Num = 0; 
	I2C_Start(I2C_Param);
	I2C_SendByte(I2C_Param, (I2C_Param->Addr  | 0));//发送器件地址+写命令
	if(I2C_Wait_Ack(I2C_Param))	//等待应答
	{	 
		return 1;		//应答超时，放弃写数据
	}
	I2C_SendByte(I2C_Param, REG_Addr);	//写寄存器地址
    I2C_Wait_Ack(I2C_Param);		//等待应答
	
	I2C_SendByte(I2C_Param, data );	//发送数据
	if( I2C_Wait_Ack(I2C_Param) )				//等待ACK  等到应答超时则放弃后面的数据
	{
		return 1;		 
	}
	    
    I2C_Stop(I2C_Param);	 						//发送停止，提示从机，数据发送完毕
	return 0;
}

u8 I2C_SingleRead(const SW_I2C_t *I2C_Param, u8 REG_Addr)	//此函数返回1是失败还是数值为1？
{
	uint8_t REG_data; 
	I2C_Start(I2C_Param);
	I2C_SendByte(I2C_Param, (I2C_Param->Addr | 0 ) );//发送器件地址+写命令
	if(I2C_Wait_Ack(I2C_Param))	//等待应答
	{	
		return 1;		 //应答超时，放弃写数据
	}
	I2C_SendByte(I2C_Param, REG_Addr);	//写寄存器地址
	if(I2C_Wait_Ack(I2C_Param))	//等待应答
	{	 
		return 1;		//应答超时，放弃写数据
	}
	
	I2C_Start(I2C_Param);	
	I2C_SendByte(I2C_Param, (I2C_Param->Addr | 1 ) );//发送器件地址 + 读命令
	I2C_Wait_Ack(I2C_Param);	//等待应答
	REG_data = I2C_ReadByte(I2C_Param, NACK);//读数据,发送nACK 
	I2C_Stop(I2C_Param);	 						//发送停止，提示从机，数据发送完毕
	return REG_data;
}









