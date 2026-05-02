
#include "main.h"
#include "mems/vibration_sensor.h"
#include "spi/bsp_spi.h"
#include "usart/bsp_usart.h"
#include "led/bsp_gpio_led.h"
#include "mems/sc7a20h.h"

#define DEBUG_LOG_ENABLE 1

static void APP_EnterStopMode(void);

/**
  * @brief  应用程序入口函数.0
  * @retval int
  */
int main(void)
{
    HAL_Init();
    APP_SystemClockConfig();

    DEBUG_USART_Config(115200);
    Bsp_Led_Init();

  APP_LOG("System boot");
  BSP_SPI_Init();
  APP_LOG("SPI ready");

  /* ===== CS# Pin Test ===== */
  /* Test if CS# can toggle freely without SPI interference */
  // APP_LOG("Testing CS# pin toggle...");
  // for (int i = 0; i < 10; i++)
  // {
  //   BSP_SPI_CS_Low();
  //   HAL_Delay(100);
  //   BSP_SPI_CS_High();
  //   HAL_Delay(100);
  //   APP_LOG("CS# toggle %d done", i);
  // }
  // APP_LOG("CS# test complete");
  /* ===== End of test ===== */

  # if (DEBUG_LOG_ENABLE)
  APP_LOG("Debug log enabled, skipping vibration sensor init");
  
  /* Initialize SC7A20H for accelerometer testing */
  SC7A20H_WakeupConfig wakeup_config = {
      .threshold = 0x10U,
      .duration = 0x00U,
  };
  
  if (SC7A20H_Init(&wakeup_config) != HAL_OK)
  {
      APP_LOG("SC7A20H initialization failed");
      APP_ErrorHandler();
  }
  
  APP_LOG("SC7A20H initialized successfully, starting XYZ test");
  
  /* Test loop: print XYZ every 1 second */
  while (1)
  {
      static uint32_t last_print_tick = 0U;
      uint32_t current_tick = HAL_GetTick();
      
      /* Print XYZ every 1 second */
      if ((current_tick - last_print_tick) >= 1000U)
      {
          last_print_tick = current_tick;
          SC7A20H_PrintAccel();
          LED2_TOGGLE();  /* Toggle LED to show main loop is running */
      }
      
      HAL_Delay(10);  /* Small delay to avoid CPU spinning */
  }

  #else
  if (VibrationSensor_Init() != HAL_OK)
  {
    APP_LOG("Vibration sensor bring-up failed");
    APP_ErrorHandler();
  }

  APP_LOG("Entering vibration wakeup flow");

    while (1)
    {
    if (VibrationSensor_HasWakeEvent() != 0U)
        {
      VibrationSensor_ClearWakeEvent();
      APP_LOG("Wake event received");
      LED2_TOGGLE();
      HAL_Delay(50);
        }

    APP_EnterStopMode();
    }
  #endif
}

/**
	* @brief  系统时钟配置函数 内部HSI倍频 主频48M
  * @param  无
  * @retval 无
  */
void APP_SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    /* 振荡器配置 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
#if defined(RCC_LSE_SUPPORT)
  RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_LSE;
#endif
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* 开启HSI */
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* HSI 1分频 */
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;  /* 配置HSI时钟24MHz */
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                         /* 开启HSE */
    RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;
    RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                         /* 关闭LSI */
#if defined(RCC_LSE_SUPPORT)
    RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                         /* 关闭LSE */
#endif
#if defined(RCC_PLL_SUPPORT)
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                     /* 开启PLL */
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
#endif
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

static void APP_EnterStopMode(void)
{
  APP_LOG("Enter STOP mode");
  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  HAL_ResumeTick();
  APP_SystemClockConfig();
  APP_LOG("Leave STOP mode");
}

/**
  * @brief  错误执行函数
  * @param  无
  * @retval 无
  */
void APP_ErrorHandler(void)
{
  APP_LOG("Fatal error");
  /* 无限循环 */
  while (1)
  {
    LED3_TOGGLE();
    HAL_Delay(150);
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
