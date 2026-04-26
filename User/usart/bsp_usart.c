/**
  ******************************************************************************
  * @file    bsp_usart_debug.c
  * @author  embedfire
  * @version V1.0
  * @date    2023-xx-xx
  * @brief   重定向c库printf函数到usart端口
  ******************************************************************************
  * @attention
  *
  * 实验平台：野火 GDF427-霸天虎 GD32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */
#include "usart/bsp_usart.h"



UART_HandleTypeDef	Uart1_Handle;



/**
 * @brief       串口X初始化函数
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @note        注意: 必须设置正确的时钟源, 否则串口波特率就会设置异常.
 * @retval      无
 */
void DEBUG_USART_Config(uint32_t BaudRate)
{
	
	Uart1_Handle.Instance = USART1;
	Uart1_Handle.Init.BaudRate = BaudRate;
	Uart1_Handle.Init.WordLength = UART_WORDLENGTH_8B;
	Uart1_Handle.Init.StopBits = UART_STOPBITS_1;
	Uart1_Handle.Init.Parity = UART_PARITY_NONE;
	Uart1_Handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	Uart1_Handle.Init.Mode = UART_MODE_TX_RX;
	/*使能串口 */
	HAL_UART_Init(&Uart1_Handle);
	/*使能串口接收断 */
	__HAL_UART_ENABLE_IT(&Uart1_Handle,UART_IT_RXNE); 
}

/**
 * @brief       UART底层初始化函数
 * @param       huart: UART句柄类型指针
 * @note        此函数会被HAL_UART_Init()调用
 *              完成时钟使能，引脚配置，中断配置z
 * @retval      无
 */	 
void HAL_UART_MspInit(UART_HandleTypeDef *UartHandle)
{
	
	if(UartHandle->Instance == USART1)  
	{
		GPIO_InitTypeDef	Uart_GPIO_InitConfig;
	
		/*USART1时钟使能*/
		DEBUG_UART_Rx_GPIO_CLK_ENABLE();
        DEBUG_UART_Tx_GPIO_CLK_ENABLE();
        __HAL_RCC_USART1_CLK_ENABLE();
            
            
        Uart_GPIO_InitConfig.Pin       = DEBUG_UART_Tx_GPIO_PIN;
        Uart_GPIO_InitConfig.Mode      = GPIO_MODE_AF_PP;
        Uart_GPIO_InitConfig.Pull      = GPIO_PULLUP;
        Uart_GPIO_InitConfig.Speed     = GPIO_SPEED_FREQ_HIGH;
        Uart_GPIO_InitConfig.Alternate = GPIO_AF0_USART1;
        HAL_GPIO_Init(DEBUG_UART_Tx_GPIO_PORT, &Uart_GPIO_InitConfig);
    
        Uart_GPIO_InitConfig.Pin 			 = DEBUG_UART_Rx_GPIO_PIN;
        Uart_GPIO_InitConfig.Mode      = GPIO_MODE_AF_PP;
        Uart_GPIO_InitConfig.Pull      = GPIO_PULLUP;
        Uart_GPIO_InitConfig.Speed     = GPIO_SPEED_FREQ_HIGH;
        Uart_GPIO_InitConfig.Alternate = GPIO_AF0_USART1;
        HAL_GPIO_Init(DEBUG_UART_Rx_GPIO_PORT, &Uart_GPIO_InitConfig);
		
		
		HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);              /* 抢占优先级0，子优先级1 */
		HAL_NVIC_EnableIRQ(USART1_IRQn);                      /* 使能USART1中断通道 */	
	}
}


void Usart_SendString(uint8_t *str)
{
	unsigned int k=0;
  do 
  {
    HAL_UART_Transmit(&Uart1_Handle,(uint8_t *)(str + k) ,1,1000);
    k++;
  } while(*(str + k)!='\0');
  
}


///* 加入以下代码, 支持printf函数 */
#if (defined (__CC_ARM)) || (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
/**
  * @brief  writes a character to the usart
  * @param  ch
  *         *f
  * @retval the character
  */
int fputc(int ch, FILE *f)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&Uart1_Handle, (uint8_t *)&ch, 1, 1000);

  return (ch);
}

/**
  * @brief  get a character from the usart
  * @param  *f
  * @retval a character
  */
int fgetc(FILE *f)
{
  int ch;
  HAL_UART_Receive(&Uart1_Handle, (uint8_t *)&ch, 1, 1000);
  return (ch);
}

#elif defined(__ICCARM__)
/**
  * @brief  writes a character to the usart
  * @param  ch
  *         *f
  * @retval the character
  */
int putchar(int ch)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&DebugUartHandle, (uint8_t *)&ch, 1, 1000);

  return (ch);
}
#elif  defined(__GNUC__)
/**
  * @brief  writes a character to the usart
  * @param  ch
  * @retval the character
  */
int __io_putchar(int ch)
{
  /* Send a byte to USART */
  HAL_UART_Transmit(&DebugUartHandle, (uint8_t *)&ch, 1, 1000);

  return ch;
}

int _write(int file, char *ptr, int len)
{
  int DataIdx;
  for (DataIdx=0;DataIdx<len;DataIdx++)
  {
    __io_putchar(*ptr++);
  }
  return len;
}
#endif



