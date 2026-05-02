/*************************************************************************************************************
 * 文件名:			SC7A20.h
 * 功能:			SC7A20 三轴加速度传感器支持
 * 作者:			cp1300@139.com
 * 创建时间:		    2022-07-01
 * 最后修改时间:	    2022-07-01
 * 详细:
*************************************************************************************************************/
#ifndef HW_ATTITUDE_SENSOR_H_
#define HW_ATTITUDE_SENSOR_H_

#include "main.h"

#define      attiSENSOR_INT_RCU      RCU_GPIOA
#define      attiSENSOR_INT_GPIO     GPIOA
#define      attiSENSOR_INT_PIN      GPIO_PIN_9

#define      attiSENSOR_SCL_RCU      RCU_GPIOA
#define      attiSENSOR_SCL_GPIO     GPIOA
#define      attiSENSOR_SCL_PIN      GPIO_PIN_11

#define      attiSENSOR_SDA_RCU      RCU_GPIOA
#define      attiSENSOR_SDA_GPIO     GPIOA
#define      attiSENSOR_SDA_PIN      GPIO_PIN_10


//SC7A20 句柄
typedef struct
{
	bool (*IIC_ReadReg)(u8 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum);		//IIC读取寄存器接口
	bool (*IIC_WriteReg)(u16 SlaveAddr, u8 RegAddr, u8 *pDataBuff, u16 ByteNum);	//IIC写入寄存器接口
	u8 SlaveAddr;
	u8 ModeConfigData;						//记录模式配置值
}SC7A20_T;
extern SC7A20_T   tSc7a20;
 
 
//SC7A20初始化
void vAtti_SensorInit(void); 
bool SC7A20_ReadAcceleration(SC7A20_T* pHandle, s16* pXa, s16* pYa, s16* pZa);//SC7A20读取加速度值
bool SC7A20_GetZAxisAngle(SC7A20_T* pHandle, s16 AcceBuff[3], float* pAngleZ);//SC7A20 获取Z轴倾角
bool bAtti_EnterLowPower(void);

#endif  //HW_ATTITUDE_SENSOR_H_












