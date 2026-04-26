#ifndef __BSP_USART_H
#define __BSP_USART_H


/* 包含其他头文件 */
#include "py32f0xx_hal.h"
#include "stdio.h"


/* 宏定义 */
#define		DEBUG_UART_Rx_GPIO_CLK_ENABLE			__HAL_RCC_GPIOB_CLK_ENABLE
#define		DEBUG_UART_Rx_GPIO_PORT						GPIOB
#define		DEBUG_UART_Rx_GPIO_PIN						GPIO_PIN_7

#define		DEBUG_UART_Tx_GPIO_CLK_ENABLE			__HAL_RCC_GPIOB_CLK_ENABLE
#define		DEBUG_UART_Tx_GPIO_PORT						GPIOB
#define		DEBUG_UART_Tx_GPIO_PIN						GPIO_PIN_6


/* 全局变量声明 */
extern	UART_HandleTypeDef	Uart1_Handle;



/* 函数声明 */
void DEBUG_USART_Config(uint32_t BaudRate);
void Usart_SendString(uint8_t *str);


#endif /* __BSP_USART_H */

