#include "main.h"
#include "mems/vibration_sensor.h"
#include "spi/bsp_spi.h"
#include "usart/bsp_usart.h"
#include "led/bsp_gpio_led.h"
#include "mems/sc7a20h.h"

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ld2451_parser.h"
#include <string.h>

#define DEBUG_LOG_ENABLE 0

static void APP_EnterStopMode(void);
void vPWMTask(void *pvParameters);
void vMainTask(void *pvParameters);
void vRadarTask(void *pvParameters);

extern void Usart_SendString(uint8_t *str);
extern UART_HandleTypeDef Uart1_Handle;

/* Global variables for Radar task coordination */
volatile bool g_radar_alert = false;
QueueHandle_t g_uart_rx_queue = NULL;

/**
  * @brief  应用程序入口函数.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  APP_SystemClockConfig();

  DEBUG_USART_Config(115200);
  Usart_SendString((uint8_t *)"[DBG] UART configured\r\n");
  
  Bsp_Led_Init();
  Usart_SendString((uint8_t *)"[DBG] LED Init done\r\n");
  
  Bsp_Led_PWM_Init();
  Usart_SendString((uint8_t *)"[DBG] LED PWM Init done\r\n");

  BSP_SPI_Init();
  Usart_SendString((uint8_t *)"[DBG] SPI Init done\r\n");

  /* Create FreeRTOS Tasks */
  Usart_SendString((uint8_t *)"[DBG] Creating Tasks...\r\n");
  
  /* Create UART RX queue */
  g_uart_rx_queue = xQueueCreate(128, sizeof(uint8_t));
  if (g_uart_rx_queue == NULL)
  {
    Usart_SendString((uint8_t *)"[ERR] Queue creation failed\r\n");
    APP_ErrorHandler();
  }

  if (xTaskCreate(vPWMTask, "PWMTask", 64, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
  {
    Usart_SendString((uint8_t *)"[ERR] PWMTask creation failed\r\n");
    APP_ErrorHandler();
  }
  if (xTaskCreate(vMainTask, "MainTask", 160, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
  {
    Usart_SendString((uint8_t *)"[ERR] MainTask creation failed\r\n");
    APP_ErrorHandler();
  }
  if (xTaskCreate(vRadarTask, "RadarTask", 192, NULL, tskIDLE_PRIORITY + 2, NULL) != pdPASS)
  {
    Usart_SendString((uint8_t *)"[ERR] RadarTask creation failed\r\n");
    APP_ErrorHandler();
  }

  /* Start FreeRTOS Scheduler */
  Usart_SendString((uint8_t *)"[DBG] Starting Scheduler...\r\n");
  vTaskStartScheduler();

  /* Should never reach here */
  while (1)
  {
  }
}

/* NOTE: HAL_IncTick() is called from TIM14_IRQHandler (see py32f0xx_it.c).
 * SysTick is exclusively owned by FreeRTOS and is NOT used for HAL timebase.
 * Therefore, vApplicationTickHook is NOT defined here and configUSE_TICK_HOOK = 0. */

/**
  * @brief  Task to control LED3 & LED4 PWM brightness (Alternate Breathing).
  *         If g_radar_alert is true (target approaching), flash LED3 and LED4 rapidly.
  */
void vPWMTask(void *pvParameters)
{
  (void)pvParameters;
  uint16_t brightness = 0;
  int8_t direction = 1;
  bool flash_state = false;

  while (1)
  {
    if (g_radar_alert)
    {
      /* Rapid flashing when target is approaching from behind */
      flash_state = !flash_state;
      Bsp_Led_PWM_SetBrightness(3, flash_state ? 1000 : 0);
      Bsp_Led_PWM_SetBrightness(4, flash_state ? 1000 : 0);
      vTaskDelay(pdMS_TO_TICKS(100)); /* Fast flash (100ms interval) */
    }
    else
    {
      /* Breathe LED3 and LED4 in opposite directions */
      Bsp_Led_PWM_SetBrightness(3, brightness);
      Bsp_Led_PWM_SetBrightness(4, 1000 - brightness);
      
      brightness += 10 * direction;
      if (brightness >= 1000)
      {
        brightness = 1000;
        direction = -1;
        vTaskDelay(pdMS_TO_TICKS(100)); /* Hold at maximum brightness */
      }
      else if (brightness <= 0)
      {
        brightness = 0;
        direction = 1;
        vTaskDelay(pdMS_TO_TICKS(100)); /* Hold at off */
      }
      else
      {
        vTaskDelay(pdMS_TO_TICKS(15)); /* 15ms step delay for smooth transition */
      }
    }
  }
}

/**
  * @brief  USART1 Interrupt Handler.
  *         Reads received bytes and pushes them to FreeRTOS queue.
  */
void USART1_IRQHandler(void)
{
  if (__HAL_UART_GET_FLAG(&Uart1_Handle, UART_FLAG_RXNE) != RESET)
  {
    uint8_t byte = (uint8_t)(Uart1_Handle.Instance->DR & 0x00FF);
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (g_uart_rx_queue != NULL)
    {
      xQueueSendFromISR(g_uart_rx_queue, &byte, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

/**
  * @brief  FreeRTOS task to parse LD2451 radar data frames.
  *         It parses raw bytes from the UART RX queue.
  *         If no data is received on the serial port, it automatically runs in 
  *         simulation mode to cyclically demonstrate target detection and LED flashing.
  */
void vRadarTask(void *pvParameters)
{
  (void)pvParameters;
  uint8_t byte;
  
  /* Allocate buffer for serial parsing */
  static uint8_t rx_buf[64];
  static uint16_t rx_idx = 0;
  
  TickType_t last_hw_packet_tick = xTaskGetTickCount();
  bool sim_mode = true;
  uint32_t sim_counter = 0;
  
  APP_LOG("Radar Task Started.");

  while (1)
  {
    /* Read byte from the UART queue with 50ms timeout */
    if (xQueueReceive(g_uart_rx_queue, &byte, pdMS_TO_TICKS(50)) == pdTRUE)
    {
      /* Got real hardware data! Disable simulation mode */
      sim_mode = false;
      last_hw_packet_tick = xTaskGetTickCount();
      
      if (rx_idx >= sizeof(rx_buf))
      {
        rx_idx = 0;
      }
      rx_buf[rx_idx++] = byte;
      
      if (rx_idx >= 10)
      {
        /* Check if footer matches the end of buffer */
        if (rx_buf[rx_idx-4] == 0xF8 && rx_buf[rx_idx-3] == 0xF7 && 
            rx_buf[rx_idx-2] == 0xF6 && rx_buf[rx_idx-1] == 0xF5)
        {
          /* Search for header in the buffer */
          for (int i = 0; i <= rx_idx - 10; i++)
          {
            if (rx_buf[i] == 0xF4 && rx_buf[i+1] == 0xF3 && 
                rx_buf[i+2] == 0xF2 && rx_buf[i+3] == 0xF1)
            {
              uint16_t payload_len = rx_buf[i+4] | (rx_buf[i+5] << 8);
              uint16_t frame_len = 4 + 2 + payload_len + 4;
              
              if (rx_idx - i >= frame_len)
              {
                ld2451_data_t result;
                ld2451_parse_status_t status = parse_ld2451_data(&rx_buf[i], frame_len, &result);
                if (status == LD2451_PARSE_OK)
                {
                  APP_LOG("Parsed HW Frame:");
                  print_ld2451_data(&result);
                  
                  /* Check if a target is approaching from behind */
                  bool approaching = false;
                  if (result.alert && result.target_count > 0)
                  {
                    for (int t = 0; t < result.target_count && t < LD2451_MAX_TARGETS; t++)
                    {
                      if (result.targets[t].is_approaching)
                      {
                        approaching = true;
                        break;
                      }
                    }
                  }
                  g_radar_alert = approaching;
                }
                else
                {
                  APP_LOG("HW Parse error: %d", status);
                }
                
                /* Shift remaining bytes */
                uint16_t consumed = i + frame_len;
                if (rx_idx > consumed)
                {
                  memmove(rx_buf, &rx_buf[consumed], rx_idx - consumed);
                  rx_idx -= consumed;
                }
                else
                {
                  rx_idx = 0;
                }
                break;
              }
            }
          }
        }
      }
    }
    else
    {
      /* If no hardware packet for 5 seconds, switch to/maintain Simulation Mode */
      if (!sim_mode && (xTaskGetTickCount() - last_hw_packet_tick > pdMS_TO_TICKS(5000)))
      {
        APP_LOG("No UART data received. Switching to Simulation Mode.");
        sim_mode = true;
      }
      
      if (sim_mode)
      {
        sim_counter++;
        /* Toggle simulation state every 5 seconds (100 * 50ms) */
        if (sim_counter % 100 == 0)
        {
          ld2451_data_t test_result;
          uint8_t state = (sim_counter / 100) % 3;
          
          if (state == 0)
          {
            APP_LOG("[SIM] Generating Left Target frame...");
            ld2451_create_test_frame_left(&test_result);
            print_ld2451_data(&test_result);
            g_radar_alert = true;
          }
          else if (state == 1)
          {
            APP_LOG("[SIM] Generating Right Target frame...");
            ld2451_create_test_frame_right(&test_result);
            print_ld2451_data(&test_result);
            g_radar_alert = true;
          }
          else
          {
            APP_LOG("[SIM] Generating No Target frame...");
            test_result.alert = false;
            test_result.target_count = 0;
            print_ld2451_data(&test_result);
            g_radar_alert = false;
          }
        }
      }
    }
  }
}

/**
  * @brief  Main Application Task (Vibration Monitoring & Low Power)
  */
void vMainTask(void *pvParameters)
{
  (void)pvParameters;

#if (DEBUG_LOG_ENABLE)
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
      SC7A20H_PrintAccel();
      LED2_TOGGLE();  /* Toggle LED to show main loop is running */
      vTaskDelay(pdMS_TO_TICKS(1000));
  }

#else
  if (VibrationSensor_Init() != HAL_OK)
  {
    APP_LOG("Vibration sensor bring-up failed");
    /* Fast flash LED2 (100ms interval) to indicate startup failure */
    while (1)
    {
      LED2_TOGGLE();
      vTaskDelay(pdMS_TO_TICKS(100));
    }
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
      
      vTaskDelay(pdMS_TO_TICKS(1000));
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
  * @brief  系统时钟配置函数 内部HSI 主频24M
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
  
  /* Disable interrupts to prevent task switching before clock is restored */
  __disable_irq();
  
  HAL_SuspendTick();
  HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);
  
  /* DIAGNOSTIC: Flash LED3 very fast right after waking up */
  for(volatile int i=0; i<300000; i++) {
     if (i % 30000 == 0) LED3_TOGGLE();
  }
  
  HAL_ResumeTick();
  
  // Re-configure system clock after waking up (since STOP mode turns off HSI/PLL/HSE)
  APP_SystemClockConfig();
  
  /* Re-enable interrupts after system clock is fully configured */
  __enable_irq();
  
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
