/*****************************************************************************************************************
*                                                                                                                *
 *                                          姿态传感器SC7A20处理函数                                             *
 *                    如果想一次读写多个寄存器，需要将寄存器地址最高位置1，也就是寄存器地址|0x80                    *
*                                                                                                                *
******************************************************************************************************************/
#include "hw_attitude_sensor.h"
#include "i2c.h"
#include "math.h"
#include "print.h"
#include "systick.h"

//****************************************************参数定义**************************************************//
#define SC7A20_REG_WHO_AM_I     0x0F
#define SC7A20_REG_CTRL_1		0x20
#define SC7A20_REG_CTRL_2		0x21
#define SC7A20_REG_CTRL_3		0x22
#define SC7A20_REG_CTRL_4		0x23
#define SC7A20_REG_X_L          0x28
#define SC7A20_REG_X_H          0x29
#define SC7A20_REG_Y_L          0x2A
#define SC7A20_REG_Y_H          0x2B
#define SC7A20_REG_Z_L          0x2C
#define SC7A20_REG_Z_H          0x2D
#define SC7A20_REG_STATUS		 0x27
#define SC7A20_REG_INT1 		 0x30

//****************************************************参数初始化**************************************************//
SW_I2C_t   tAtti_I2c;
SC7A20_T   tSc7a20;

//****************************************************函数声明****************************************************//
void v_sc7a20_io_init(void);
bool b_sc7a20_init(SC7A20_T *pHandle, u8 SlaveAddr, 
	bool (*IIC_ReadReg)(u8 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum), 
	bool (*IIC_WriteReg)(u16 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum));
static bool b_sc7a20_i2c_read_reg(u8 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum);
static bool b_sc7a20_i2c_write_reg(u16 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum);

/*****************************************************************************************************************
-----函数功能    传感器初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
void vAtti_SensorInit(void)
{
	v_sc7a20_io_init();
	b_sc7a20_init(&tSc7a20, tAtti_I2c.Addr, b_sc7a20_i2c_read_reg, b_sc7a20_i2c_write_reg);
}

/*****************************************************************************************************************
-----函数功能    传感器相关接口初始化
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
******************************************************************************************************************/
static void v_sc7a20_io_init(void)
{
	rcu_periph_clock_enable(attiSENSOR_INT_RCU);
	gpio_init(attiSENSOR_INT_GPIO, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ,attiSENSOR_INT_PIN);
	
	//初始化IIC接口
	rcu_periph_clock_enable(attiSENSOR_SCL_RCU);
	rcu_periph_clock_enable(attiSENSOR_SDA_RCU);
	tAtti_I2c.GPIO_SCL = attiSENSOR_SCL_GPIO;
    tAtti_I2c.PIN_SCL  = attiSENSOR_SCL_PIN;
    tAtti_I2c.GPIO_SDA = attiSENSOR_SDA_GPIO;
    tAtti_I2c.PIN_SDA  = attiSENSOR_SDA_PIN;
    tAtti_I2c.Addr = 0x19;
    tAtti_I2c.AddrType = AddrType_7bit;
    tAtti_I2c.Delay = 70;
    JY_I2C_INIT(&tAtti_I2c);
}



//读取寄存器接口
static bool b_sc7a20_i2c_read_reg(u8 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum)
{
	if (I2C_ReadBytes(&tAtti_I2c, RegAddr, pDataBuff, ByteNum) != 0) return false;
	else return true;
}
	
 
//写寄存器接口
static bool b_sc7a20_i2c_write_reg(u16 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum)
{
	if (I2C_WriteBytes(&tAtti_I2c, RegAddr, pDataBuff, ByteNum) != 0) return false;
	else return true;
}


/*************************************************************************************************************************
*函数        	:	bool b_sc7a20_init(SC7A20_T *pHandle, u8 SlaveAddr, 
						bool (*IIC_ReadReg)(u8 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum), 
						bool (*IIC_WriteReg)(u16 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum))
*功能        	:	SC7A20初始化
*参数        	:	pHandle:句柄；SlaveAddr：芯片IIC写地址；IIC_ReadReg：IIC读取接口；IIC_WriteReg:IIC写入接口；
*返回        	:	true:初始化成功；false:初始化失败
*依赖			: 	底层宏定义
*作者       		:	cp1300@139.com
*时间     		:	2022-07-01
*最后修改时间		:	2022-07-01
*说明        	:	
*************************************************************************************************************************/
static bool b_sc7a20_init(SC7A20_T *pHandle, u8 SlaveAddr, 
	bool (*IIC_ReadReg)(u8 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum), 
	bool (*IIC_WriteReg)(u16 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum))
{
	u16 i;
	u8 tempreg;
	u8 buff[16];
	
	if(pHandle == NULL) 
	{
		if(uPrint.tFlag.Sensor)
		  sMyPrint("无效的句柄\r\n");
		
		return false;
	}
	pHandle->SlaveAddr = SlaveAddr;										//记录地址
	pHandle->IIC_ReadReg = IIC_ReadReg;									//IIC接口
	pHandle->IIC_WriteReg = IIC_WriteReg;								//IIC接口
	
	tempreg = 0;

	for (i = 0; i < 10; i++)
	{
		buff[0] = buff[1] = 0;
		if (pHandle->IIC_ReadReg(pHandle->SlaveAddr, SC7A20_REG_WHO_AM_I, buff, 1) == false)
		{
			if(uPrint.tFlag.Sensor)
			  sMyPrint("Sensor通讯失败\r\n");
			
		}
		else if(buff[0] == 0x11)
		{
			break;
		}
		else
		{
			if(uPrint.tFlag.Sensor)
			  sMyPrint("无效的id0x%02X\r\n", buff[0]);
			
		}
		delay_1ms(100);
	}
 
    //惯性中断源 AOI1、AOI2 供用户使用
	//CLICK单击或双击
//	tempreg = 0x44;  //0100_0100
//	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_1, &tempreg, 1);//10Hz+低耗模式+z使能
//	tempreg = 0x89;  //0000 1001
//	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_2, &tempreg, 1);//中断 1 AOI 功能高通滤波使能。
//	tempreg = 0x40; //0100 0000
//	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_3, &tempreg, 1); //AOI1 中断在 on INT1。
//	tempreg = 0x88;
//	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_4, &tempreg, 1); //读取完成再更新，小端模式，、2g+正常模式，高精度模式
//	tempreg = 0x60;  //0110 0000
//	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_INT1, &tempreg, 1);
	
	tempreg = 0x44;  //0100_0100
	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_1, &tempreg, 1);//10Hz+低耗模式+z使能
	tempreg = 0x01;  //0000 0001
	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_2, &tempreg, 1);//中断 1 AOI 功能高通滤波使能。
	tempreg = 0x40; //0100 0000
	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_3, &tempreg, 1); //AOI1 中断在 on INT1。
	tempreg = 0x88;
	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_CTRL_4, &tempreg, 1); //读取完成再更新，小端模式，、2g+正常模式，高精度模式
	tempreg = 0x60;  //0110 0000
	pHandle->IIC_WriteReg(pHandle->SlaveAddr, SC7A20_REG_INT1, &tempreg, 1);
	
	if (i < 10)
	{
		if(uPrint.tFlag.Sensor)
		  sMyPrint("初始化成功\r\n");
		
		return true;
	}
	else
	{
		if(uPrint.tFlag.Sensor)
		  sMyPrint("初始化失败\r\n");
		
		return false;
	}
	    
}
 
s16 SC7A20_12bitComplement(uint8_t msb, uint8_t lsb)
{
	s16 temp;
 
	temp = msb << 8 | lsb;
	temp = temp >> 4;   //只有高12位有效
	temp = temp & 0x0fff;
	if (temp & 0x0800) //负数 补码==>原码
	{
		temp = temp & 0x07ff; //屏弊最高位      
		temp = ~temp;
		temp = temp + 1;
		temp = temp & 0x07ff;
		temp = -temp;       //还原最高位
	}
	return temp;
}
 
 
/*************************************************************************************************************************
*函数        	:	bool SC7A20_ReadAcceleration(SC7A20_T* pHandle, s16* pXa, s16* pYa, s16* pZa)
*功能        	:	SC7A20读取加速度值
*参数        	:	pHandle:句柄；pXa，pYa,pZa 三轴加速度值
*返回        	:	true:成功；false:失败
*依赖			: 	底层宏定义
*作者       		:	cp1300@139.com
*时间     		:	2022-07-01
*最后修改时间		:	2022-07-01
*说明        	:
*************************************************************************************************************************/
bool SC7A20_ReadAcceleration(SC7A20_T* pHandle, s16* pXa, s16* pYa, s16* pZa)
{
	u8 buff[6];
	u8 i;
 
	memset(buff, 0, 6);
	if (pHandle->IIC_ReadReg(pHandle->SlaveAddr, SC7A20_REG_X_L|0x80, buff, 6) == false)
	{
		return false;
	}
	else
	{
		
		pHandle->IIC_ReadReg(pHandle->SlaveAddr, 0x27, &i, 1);
		
		if(uPrint.tFlag.Sensor)
		{
			for (i = 0; i < 6; i++)
			{
				  sMyPrint("%02X ",buff[i]);
				
			}
			sMyPrint("\r\n");
		}
		
 
		//X轴
		*pXa = buff[1];
		*pXa <<= 8;
		*pXa |= buff[0];
		*pXa >>= 4;	//取12bit精度
 
		//Y轴
		*pYa = buff[3];
		*pYa <<= 8;
		*pYa |= buff[2];
		*pYa >>= 4;	//取12bit精度
		
		//Z轴
		*pZa = buff[5];
		*pZa <<= 8;
		*pZa |= buff[4];
		*pZa >>= 4;	//取12bit精度
 
		return true;
	}
 
}
 
#define PI 3.1415926535898
/*************************************************************************************************************************
*函数        	:	bool SC7A20_GetZAxisAngle(SC7A20_T* pHandle, s16 AcceBuff[3], float* pAngleZ)
*功能        	:	SC7A20 获取Z轴倾角
*参数        	:	pHandle:句柄；AcceBuff:3个轴的加速度；pAngleZ：Z方向倾角
*返回        	:	true:成功；false:失败
*依赖			: 	底层宏定义
*作者       		:	cp1300@139.com
*时间     		:	2022-07-02
*最后修改时间		:	2022-07-02
*说明        	:
*************************************************************************************************************************/
bool SC7A20_GetZAxisAngle(SC7A20_T* pHandle, s16 AcceBuff[3], float* pAngleZ)
{
	double fx, fy, fz;
	double A;
	s16 Xa, Ya, Za;
 
 
 
 
 
	if (SC7A20_ReadAcceleration(pHandle, &Xa, &Ya, &Za) == false) return false;		//ADXL362 读取加速度数据
	//  sMyPrint("Xa:%d \tYa:%d \tZa:%d \r\n",Xa,Ya,Za);
	AcceBuff[0] = Xa;	//x轴加速度
	AcceBuff[1] = Ya;	//y轴加速度
	AcceBuff[2] = Za;	//z轴加速度
 
	fx = Xa;
	fx *= 2.0 / 4096;
	fy = Ya;
	fy *= 2.0 / 4096;
	fz = Za;
	fz *= 2.0 / 4096;
 
	//  sMyPrint("fx：%.04f\tfy：%.04f\tfz：%.04f\t\r\n",fx,fy,fz);
 
	//Z方向
	A = fx * fx + fy * fy;
	A = sqrt(A);
	A = (double)A / fz;
	A = atan(A);
	A = A * 180 / PI;
 
	*pAngleZ = A;
	
	if(uPrint.tFlag.Sensor)
	  sMyPrint("=======角度：%f\r\n",*pAngleZ);
	
 
	return true;
}



/***********************************************************************************************************************
-----函数功能    LCD显示任务
-----说明(备注)  none
-----传入参数    none
-----输出参数    none
-----返回值      none
************************************************************************************************************************/
bool bAtti_EnterLowPower(void)
{
	rcu_periph_clock_enable(attiSENSOR_SCL_RCU);
	gpio_init(attiSENSOR_SCL_GPIO, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ,attiSENSOR_SCL_PIN);
	
	rcu_periph_clock_enable(attiSENSOR_SDA_RCU);
	gpio_init(attiSENSOR_SDA_GPIO, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ,attiSENSOR_SDA_PIN);
	
	
//	/* enable clock */
//    rcu_periph_clock_enable(RCU_PMU);
//	rcu_periph_clock_enable(attiSENSOR_INT_RCU);//A9
//	rcu_periph_clock_enable(RCU_AF);
//	
//	gpio_init(attiSENSOR_INT_GPIO,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_2MHZ,attiSENSOR_INT_PIN);
//	
//	/* enable and set key EXTI interrupt to the lowest priority */
//	nvic_irq_enable(EXTI5_9_IRQn, 2U, 0U);

//	/* connect key EXTI line to key GPIO pin */
//	gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_9);  //PA9

//	/* configure key EXTI line */
//	exti_init(EXTI_9, EXTI_INTERRUPT, EXTI_TRIG_FALLING); //上升下降沿触发
//	exti_interrupt_flag_clear(EXTI_9);
//	exti_interrupt_enable(EXTI_9);//
	
	return true;
}













