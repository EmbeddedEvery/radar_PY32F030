/**
  ******************************************************************************
  * @file     main.c
  * @author   embedfire
  * @version  V1.0
  * @date     2024
  * @brief    ADC_SingleConversion_TriggerTimer_IT 
  ******************************************************************************
  * @attention
  *
  * 实验平台：野火 ebf_py32f030 PY32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */
#include "main.h"
#include "usart/bsp_usart.h"
#include "adc/bsp_adc_singleconv_polling.h"
#include "led/bsp_gpio_led.h"



static void APP_SystemClockConfig(void);

/**
  * @brief  应用程序入口函数.
  * @retval int
  */
int main(void)
{
    /* 初始化所有外设，Flash接口，SysTick */
    HAL_Init();
    /* 系统时钟配置 48M */
    APP_SystemClockConfig();

    /* Debug串口初始化 */
    DEBUG_USART_Config(115200);
    Bsp_Led_Init();
    
    /* ADC初始化 */
    Bsp_ADC_SingleConv_Polling_Init();

    while (1)
    {
         /* ADC开启 */  
        HAL_ADC_Start(&AdcHandle);  
        printf("\r\n");        
				LED2_TOGGLE();
        for (uint8_t i = 0; i < 3; i++)
        {
            /* 等待ADC转换 */
            HAL_ADC_PollForConversion(&AdcHandle, 10000); 
            /* 获取AD值 */
            adc_value[i] = HAL_ADC_GetValue(&AdcHandle);                       
            printf("adc[%d]:%d\r\n", i, adc_value[i]);
        }
        printf("\r\n"); 
        HAL_Delay(1000);
    }
}

/**
	* @brief  系统时钟配置函数 内部HSI倍频 主频48M
  * @param  无
  * @retval 无
  */
static void APP_SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* 振荡器配置 */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE; /* 选择振荡器HSE,HSI,LSI,LSE */
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* 开启HSI */
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* HSI 1分频 */
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;  /* 配置HSI时钟24MHz */
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;                         /* 关闭HSE */
    RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                         /* 关闭LSI */
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                         /* 关闭LSE */
    /*RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;*/
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                     /* 开启PLL */
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    /* 配置振荡器 */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        APP_ErrorHandler();
    }
    
    /* 时钟源配置 */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; /* 选择配置时钟 HCLK,SYSCLK,PCLK1 */
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; /* 选择PLLCLK作为系统时钟 */
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;     /* AHB时钟 1分频 */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;      /* APB时钟 1分频 */
    /* 配置时钟源 */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        APP_ErrorHandler();
    }
}

/**
  * @brief  错误执行函数
  * @param  无
  * @retval 无
  */
void APP_ErrorHandler(void)
{
  /* 无限循环 */
  while (1)
  {
		
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  输出产生断言错误的源文件名及行号
  * @param  file：源文件名指针
  * @param  line：发生断言错误的行号
  * @retval 无
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* 用户可以根据需要添加自己的打印信息,
     例如: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* 无限循环 */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
