#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "py32f0xx_hal.h"



#define		Key1_GPIO_CLK_ENABLE			__HAL_RCC_GPIOA_CLK_ENABLE
#define		Key1_GPIO_PORT						GPIOA
#define		Key1_GPIO_PIN							GPIO_PIN_5

#define		Key2_GPIO_CLK_ENABLE			__HAL_RCC_GPIOA_CLK_ENABLE
#define		Key2_GPIO_PORT						GPIOA
#define		Key2_GPIO_PIN							GPIO_PIN_6


#define		UP 		1
#define		Dowm 	0


#define		Key1_Dowm 	0x1
#define		Key2_Dowm 	0x2



void 			Bsp_Key_Init(void);
uint8_t 	Key_Scan(uint8_t mode);



#endif /* __BSP_KEY_H */

