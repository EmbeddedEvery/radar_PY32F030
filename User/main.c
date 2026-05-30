
#include "main.h"
#include "mems/vibration_sensor.h"
#include "spi/bsp_spi.h"
#include "usart/bsp_usart.h"
#include "led/bsp_gpio_led.h"
#include "mems/sc7a20h.h"
// #include "bsp_low_power.h"

#define DEBUG_LOG_ENABLE 0

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

    // Debugging For LowPower
    //BSP_LowPower_Init();
    //BSP_LowPower_ConfigExternalWakeup(GPIOA, GPIO_PIN_0, GPIO_MODE_IT_RISING);
    // WakeupSource_t reason = BSP_LowPower_GetWakeupReason();
    // if (reason == WAKEUP_EXTI_PA0) {
    //     // GPIO PA0 triggered wakeup
    // }
    //BSP_LowPower_EnterStandby(WAKEUP_EXTI_PA0);


  APP_LOG("System boot");
  BSP_SPI_Init();
  APP_LOG("SPI ready");


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

  APP_LOG("Vibration sensor initialized successfully.");

  while (1)
  {
    APP_LOG("MCU Started / Woken Up!");
    
    // Countdown 20 seconds before entering STOP mode
    for (int i = 20; i > 0; i--)
    {
      APP_LOG("Entering STOP mode in %d seconds...", i);
      LED2_TOGGLE();  /* Blink LED to show countdown is running */
      
      /* DIAGNOSTIC: Check if EXTI triggers during countdown! */
      if (VibrationSensor_HasWakeEvent() != 0U) {
          APP_LOG(">>> EXTI EVENT DETECTED DURING COUNTDOWN <<<");
          VibrationSensor_ClearWakeEvent();
      }
      
      HAL_Delay(1000);
    }
    
    // Ensure LED2 is turned off during sleep to minimize power consumption
    LED2(LED_OFF);
    
    // Clear any previous wake event before going to sleep
    VibrationSensor_ClearWakeEvent();
    
    // Enter STOP LPR mode (low power regulator) and wait for interrupt
    APP_EnterStopMode();
    
    // Resume here after wakeup
    if (VibrationSensor_HasWakeEvent() != 0U)
    {
      APP_LOG("Wakeup event detected: SC7A20H Vibration triggered!");
      VibrationSensor_ClearWakeEvent();
    }
    else
    {
      APP_LOG("Wakeup event detected: Unknown source");
    }
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

  /* Use HSI as the main clock source to avoid HSE startup/lock issues after STOP mode */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                          /* Enable HSI */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz; /* Set HSI to 24MHz */
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                          /* No division */
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
#if defined(RCC_PLL_SUPPORT)
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;                     /* Keep PLL off */
#endif

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* Select HSI as system clock */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  /* HSI 24MHz works at FLASH_LATENCY_0 */
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

static void APP_EnterStopMode(void)
{
  APP_LOG("Enter STOP mode");
  
  // Wait for USART1 transmission to complete before disabling clocks
  while (__HAL_UART_GET_FLAG(&Uart1_Handle, UART_FLAG_TC) == RESET);
  
  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
  
  /* DIAGNOSTIC: Flash LED3 very fast right after waking up */
  for(volatile int i=0; i<300000; i++) {
     if (i % 30000 == 0) LED3_TOGGLE();
  }
  
  HAL_ResumeTick();
  
  // Re-configure system clock after waking up (since STOP mode turns off HSI/PLL/HSE)
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
