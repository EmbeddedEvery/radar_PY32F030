#ifndef __BSP_ADC_SINGLECONV_POLLING_H
#define __BSP_ADC_SINGLECONV_POLLING_H

/* 包含其他头文件 */
#include "py32f0xx_hal.h"



/* 全局变量声明 */
extern ADC_HandleTypeDef             AdcHandle;
extern uint32_t adc_value[3];


/* 函数声明 */
void Bsp_ADC_SingleConv_Polling_Init(void);




#endif /* __BSP_ADC_SINGLECONV_POLLING_H */

