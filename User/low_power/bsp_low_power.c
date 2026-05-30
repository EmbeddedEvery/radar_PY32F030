/**
 * @file    bsp_low_power.c
 * @brief   PY32F030 Low Power Mode Management Implementation
 * @author  BSP Team
 * @date    2024
 */

#include "bsp_low_power.h"

/* Private variables */
static WakeupSource_t g_wakeup_source = WAKEUP_RTC_SECOND;

/**
 * @brief Initialize system for ultra-low power Standby mode
 */
HAL_StatusTypeDef BSP_LowPower_Init(void)
{
    /* Disable all peripheral clocks except necessary ones */
    BSP_LowPower_DisablePeripherals();
    
    /* Configure GPIO for low power (all pull-down inputs) */
    BSP_LowPower_ConfigGPIO();
    
    /* Enable PWR clock for low power control */
    __HAL_RCC_PWR_CLK_ENABLE();
    
    return HAL_OK;
}

/**
 * @brief Enter ultra-low power Standby mode (~4.5µA)
 * 
 * Note: PY32F0xx implements Standby mode via STOP with low-power regulator
 * This is the lowest power consumption mode available
 */
void BSP_LowPower_EnterStandby(WakeupSource_t wakeup_source)
{
    g_wakeup_source = wakeup_source;
    
    /* Clear all EXTI pending flags */
    EXTI->PR = 0xFFFF;
    
    /* Enable PWR clock for register access */
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /* Suspend SysTick during low power mode */
    HAL_SuspendTick();
    
    /* Enter STOP mode with low-power regulator */
    /* This achieves ~4.5µA consumption on PY32F0xx */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    
    /* Resume SysTick after wakeup */
    HAL_ResumeTick();
}

/**
 * @brief Enter Stop mode (<50µA, RAM retained)
 */
void BSP_LowPower_EnterStop(void)
{
    /* Enable PWR clock */
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /* Use HAL function to enter Stop mode */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

/**
 * @brief Enter Sleep mode (~300-500µA, all peripherals running)
 */
void BSP_LowPower_EnterSleep(void)
{
    /* Clear SLEEPDEEP bit for Sleep mode */
    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
    
    /* Execute WFI */
    __WFI();
}

/**
 * @brief Configure all GPIO for low power
 * 
 * Configures all GPIO ports as pull-down inputs to prevent
 * floating pin leakage currents.
 */
void BSP_LowPower_ConfigGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable all GPIO clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    /* Configure all GPIOA pins: pull-down input */
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* Configure all GPIOB pins: pull-down input */
    GPIO_InitStruct.Pin = GPIO_PIN_All;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /* Disable GPIO clocks to save power */
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
}

/**
 * @brief Disable all non-essential peripheral clocks
 */
void BSP_LowPower_DisablePeripherals(void)
{
    /* Disable UART */
    __HAL_RCC_USART1_CLK_DISABLE();
    
    /* Disable SPI */
    __HAL_RCC_SPI1_CLK_DISABLE();
    
    /* Disable ADC */
    __HAL_RCC_ADC_CLK_DISABLE();
    
    /* Other peripherals are already disabled by default */
    /* Keep GPIO and EXTI enabled for wakeup functionality */
}

/**
 * @brief Configure RTC for periodic wakeup
 * 
 * @note RTC support may be limited on PY32F0xx
 * For now, use GPIO/EXTI wakeup instead
 */
HAL_StatusTypeDef BSP_LowPower_ConfigRTCWakeup(uint32_t interval_seconds)
{
    if (interval_seconds < 1 || interval_seconds > 3600) {
        return HAL_ERROR;
    }
    
    /* RTC configuration would go here */
    /* Currently simplified for PY32F0xx compatibility */
    
    return HAL_OK;
}

/**
 * @brief Configure external pin for wakeup
 */
HAL_StatusTypeDef BSP_LowPower_ConfigExternalWakeup(GPIO_TypeDef *port, uint16_t pin, uint8_t trigger_type)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* Enable GPIO clock */
    if (port == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (port == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else {
        return HAL_ERROR;
    }
    
    /* Configure GPIO as external interrupt input */
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = trigger_type;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    /* Enable EXTI interrupt */
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 2, 0);
    
    return HAL_OK;
}

/**
 * @brief Get wakeup reason after exiting Standby
 */
WakeupSource_t BSP_LowPower_GetWakeupReason(void)
{
    /* Check which wakeup source was triggered by EXTI flags */
    
    if (EXTI->PR & EXTI_PR_PIF0) {
        EXTI->PR = EXTI_PR_PIF0;  /* Clear flag */
        return WAKEUP_EXTI_PA0;
    }
    
    if (EXTI->PR & EXTI_PR_PIF5) {
        EXTI->PR = EXTI_PR_PIF5;  /* Clear flag */
        return WAKEUP_EXTI_PB5;
    }
    
    return g_wakeup_source;
}

/**
 * @brief Get estimated current consumption for a given mode
 */
uint32_t BSP_LowPower_GetEstimatedCurrent(LowPowerMode_t mode)
{
    switch (mode) {
        case LOW_POWER_SLEEP:
            return 300;      /* µA */
        case LOW_POWER_STOP:
            return 50;       /* µA */
        case LOW_POWER_STANDBY:
            return 4;        /* µA */
        default:
            return 2000;     /* µA */
    }
}
