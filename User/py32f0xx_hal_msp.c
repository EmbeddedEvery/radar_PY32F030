/**
  ******************************************************************************
  * @file    py32f0xx_hal_msp.c
  * @author  MCU Application Team
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
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
#include "py32f0xx_hal_tim.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
  * @brief  TIM14 handle used as HAL timebase source (instead of SysTick).
  *         SysTick is exclusively owned by FreeRTOS.
  *         Declared extern so py32f0xx_it.c can reference it.
  */
TIM_HandleTypeDef htim14;

/* Private function prototypes -----------------------------------------------*/
/* External functions --------------------------------------------------------*/

/**
  * @brief 初始化全局MSP
  */
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();
}

/**
  * @brief  Override the default HAL_InitTick() to use TIM14 instead of SysTick.
  *         This is required for FreeRTOS: FreeRTOS owns SysTick entirely; HAL
  *         uses TIM14 for its 1ms tick (HAL_Delay, HAL_GetTick, etc.).
  * @param  TickPriority: Tick interrupt priority (unused here, fixed to 0).
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef clkconfig;
  uint32_t           uwTimclock, uwAPB1Prescaler;
  uint32_t           pFLatency;
  uint32_t           uwPrescalerValue;

  /* Enable TIM14 clock */
  __HAL_RCC_TIM14_CLK_ENABLE();

  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

  uwAPB1Prescaler = clkconfig.APB1CLKDivider;

  /* Compute TIM14 clock */
  if (uwAPB1Prescaler == RCC_HCLK_DIV1)
  {
    uwTimclock = HAL_RCC_GetPCLK1Freq();
  }
  else
  {
    uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
  }

  /* Compute the prescaler value to have TIM14 counter clock equal to 1MHz */
  uwPrescalerValue = (uint32_t)((uwTimclock / 1000000U) - 1U);

  /* Initialize TIM14 */
  htim14.Instance = TIM14;

  /* Initialize TIMx peripheral as follows:
     + Period     = [(TIM14CLK/1000) - 1] => 1ms tick
     + Prescaler  = [(uwTimclock/1000000) - 1] => 1MHz timer clock
     + ClockDiv   = 0
     + Counter    = Up
  */
  htim14.Init.Period            = (1000000U / 1000U) - 1U;  /* 1ms */
  htim14.Init.Prescaler         = uwPrescalerValue;
  htim14.Init.ClockDivision     = 0U;
  htim14.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim14.Init.RepetitionCounter = 0U;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  if (HAL_TIM_Base_Init(&htim14) == HAL_OK)
  {
    /* Start the TIM time Base generation in interrupt mode */
    return HAL_TIM_Base_Start_IT(&htim14);
  }

  /* Return function status */
  return HAL_ERROR;
}

/**
  * @brief  Suspend Tick increment.
  *         Disable TIM14 interrupt to pause the 1ms HAL timebase (used before STOP mode).
  */
void HAL_SuspendTick(void)
{
  /* Disable TIM14 update interrupt */
  __HAL_TIM_DISABLE_IT(&htim14, TIM_IT_UPDATE);
}

/**
  * @brief  Resume Tick increment.
  *         Re-enable TIM14 interrupt to resume the 1ms HAL timebase (used after STOP mode wakeup).
  */
void HAL_ResumeTick(void)
{
  /* Enable TIM14 update interrupt */
  __HAL_TIM_ENABLE_IT(&htim14, TIM_IT_UPDATE);
}

/**
  * @brief  TIM Base MSP Initialization (called by HAL_TIM_Base_Init).
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim_base)
{
  if (htim_base->Instance == TIM14)
  {
    /* Enable TIM14 clock (already done above, but safe to call twice) */
    __HAL_RCC_TIM14_CLK_ENABLE();

    /* Enable TIM14 IRQ, set priority to be lower than all task-usable interrupts */
    HAL_NVIC_SetPriority(TIM14_IRQn, 3U, 0U);
    HAL_NVIC_EnableIRQ(TIM14_IRQn);
  }
}

/**
  * @brief 初始化ADC相关MSP
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

/************************ (C) COPYRIGHT Puya *****END OF FILE******************/
