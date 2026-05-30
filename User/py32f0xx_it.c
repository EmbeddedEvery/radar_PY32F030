/**
  ******************************************************************************
  * @file    py32f0xx_it.c
  * @author  MCU Application Team
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mems/vibration_sensor.h"
#include "py32f0xx_it.h"
#include "py32f0xx_hal_tim.h"

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim14;  /* HAL timebase timer defined in py32f0xx_hal_msp.c */



/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/* System exception handlers (SVC, PendSV, SysTick) are implemented by FreeRTOS via macros in FreeRTOSConfig.h */

/******************************************************************************/
/* PY32F0xx Peripheral Interrupt Handlers                                     */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file.                                          */
/******************************************************************************/

/**
  * @brief  TIM14 interrupt handler - used as HAL 1ms timebase source.
  *         Calls HAL_IncTick() to increment the HAL tick counter (uwTick).
  *         This replaces the SysTick_Handler role for HAL, since SysTick is
  *         exclusively owned by FreeRTOS.
  */
void TIM14_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim14);
}

/**
  * @brief  Called by HAL_TIM_IRQHandler when the timer update event fires.
  *         We use this to increment the HAL millisecond tick counter.
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM14)
  {
    HAL_IncTick();
  }
}

void EXTI0_1_IRQHandler(void)
{
  VibrationSensor_IrqHandler();
}




/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
