#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "py32f0xx_hal.h"




#define		LED2_GPIO_CLK_ENABLE		__HAL_RCC_GPIOA_CLK_ENABLE
#define		LED2_GPIO_PORT					GPIOA
#define		LED2_GPIO_PIN						GPIO_PIN_2

#define		LED3_GPIO_CLK_ENABLE		__HAL_RCC_GPIOA_CLK_ENABLE
#define		LED3_GPIO_PORT					GPIOA
#define		LED3_GPIO_PIN						GPIO_PIN_3

#define		LED4_GPIO_CLK_ENABLE		__HAL_RCC_GPIOA_CLK_ENABLE
#define		LED4_GPIO_PORT					GPIOA
#define		LED4_GPIO_PIN						GPIO_PIN_4


#define		LED_ON 		0
#define		LED_OFF 	1


#define 	LED2(x)   					x ? \
														HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_GPIO_PIN, GPIO_PIN_SET): \
														HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_GPIO_PIN, GPIO_PIN_RESET); \

#define 	LED2_TOGGLE()     HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_GPIO_PIN)


#define 	LED3(x)   					x ? \
														HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_GPIO_PIN, GPIO_PIN_SET): \
														HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_GPIO_PIN, GPIO_PIN_RESET); \

#define 	LED3_TOGGLE()     HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_GPIO_PIN)


#define 	LED4(x)   					x ? \
														HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_GPIO_PIN, GPIO_PIN_SET): \
														HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_GPIO_PIN, GPIO_PIN_RESET); \

#define 	LED4_TOGGLE()     HAL_GPIO_TogglePin(LED4_GPIO_PORT, LED4_GPIO_PIN)


void Bsp_Led_Init(void);



#endif /* __BSP_LED_H */

