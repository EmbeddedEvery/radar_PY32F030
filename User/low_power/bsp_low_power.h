/**
 * @file    bsp_low_power.h
 * @brief   PY32F030 Low Power Mode Management API
 * @author  BSP Team
 * @date    2024
 */

#ifndef __BSP_LOW_POWER_H
#define __BSP_LOW_POWER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "py32f0xx_hal.h"

/**
 * @brief Low Power Mode Types
 */
typedef enum {
    LOW_POWER_SLEEP = 0,      /*!< Sleep mode: CPU off, peripherals on */
    LOW_POWER_STOP = 1,       /*!< Stop mode: <50µA */
    LOW_POWER_STANDBY = 2     /*!< Standby mode: ~4.5µA (Ultra-low) */
} LowPowerMode_t;

/**
 * @brief Wakeup Source Configuration
 */
typedef enum {
    WAKEUP_RTC_SECOND = 0,    /*!< RTC second interrupt wakeup */
    WAKEUP_RTC_ALARM = 1,     /*!< RTC alarm wakeup */
    WAKEUP_EXTI_PA0 = 2,      /*!< EXTI PA0 rising edge wakeup */
    WAKEUP_EXTI_PB5 = 3       /*!< EXTI PB5 rising edge wakeup */
} WakeupSource_t;

/**
 * @brief Initialize system for ultra-low power mode (4.5µA Standby)
 * 
 * This function configures:
 * - All GPIO to pull-down input (prevent leakage)
 * - Disable all peripheral clocks
 * - Stop HSE, LSE oscillators
 * - Initialize RTC for wakeup
 * 
 * @note Call this function before entering Standby mode
 * @retval HAL_OK if success, HAL_ERROR otherwise
 */
HAL_StatusTypeDef BSP_LowPower_Init(void);

/**
 * @brief Enter ultra-low power Standby mode (~4.5µA)
 * 
 * Characteristics:
 * - Core frequency: 0 Hz
 * - Peripherals: Off
 * - RAM: Not retained (must save state before calling)
 * - Wakeup: RTC interrupt or external pin
 * - Recovery: Full system reset on wakeup
 * 
 * @param wakeup_source Wakeup source selection
 * @return This function doesn't return (MCU resets on wakeup)
 */
void BSP_LowPower_EnterStandby(WakeupSource_t wakeup_source);

/**
 * @brief Enter Stop mode (<50µA)
 * 
 * Characteristics:
 * - Core frequency: 0 Hz
 * - Peripherals: Off (can be re-enabled)
 * - RAM: Retained
 * - Wakeup: Any enabled interrupt
 * - Recovery: Resume from same position
 * 
 * @note This is safer than Standby as code execution continues
 * @return void
 */
void BSP_LowPower_EnterStop(void);

/**
 * @brief Enter Sleep mode (~300-500µA)
 * 
 * Characteristics:
 * - Core frequency: 0 Hz
 * - Peripherals: Running normally
 * - RAM: Retained
 * - Wakeup: Any enabled interrupt
 * - Recovery: Resume from same position
 * 
 * @note Least intrusive, most compatible with existing code
 * @return void
 */
void BSP_LowPower_EnterSleep(void);

/**
 * @brief Configure GPIO for low power (all pull-down inputs)
 * 
 * Sets all GPIO ports to:
 * - Mode: Input
 * - Pull: Pull-down
 * - This prevents floating pin leakage current
 * 
 * @note Typically called during BSP_LowPower_Init()
 * @return void
 */
void BSP_LowPower_ConfigGPIO(void);

/**
 * @brief Disable all peripheral clocks to reduce power consumption
 * 
 * Disables clocks for:
 * - UART, SPI, I2C, ADC, DMA
 * - RTC (must be re-enabled if using RTC wakeup)
 * - All other non-essential peripherals
 * 
 * @note Typically called during BSP_LowPower_Init()
 * @return void
 */
void BSP_LowPower_DisablePeripherals(void);

/**
 * @brief Configure RTC for periodic wakeup (every N seconds)
 * 
 * @param interval_seconds Wakeup interval in seconds (1-3600)
 * @retval HAL_OK if success
 * 
 * @note Sets RTC alarm to trigger every 'interval_seconds'
 */
HAL_StatusTypeDef BSP_LowPower_ConfigRTCWakeup(uint32_t interval_seconds);

/**
 * @brief Configure external pin for wakeup
 * 
 * @param port GPIO port (GPIOA, GPIOB, etc.)
 * @param pin GPIO pin number (GPIO_PIN_0 to GPIO_PIN_15)
 * @param trigger_type Rising or falling edge
 * @retval HAL_OK if success
 * 
 * @note PA0 and PB5 have dedicated wakeup capability
 */
HAL_StatusTypeDef BSP_LowPower_ConfigExternalWakeup(GPIO_TypeDef *port, uint16_t pin, uint8_t trigger_type);

/**
 * @brief Check wakeup reason after exiting Standby mode
 * 
 * @return Wakeup source identification:
 *         - WAKEUP_RTC_SECOND: RTC second interrupt
 *         - WAKEUP_EXTI_PA0: External PA0 pin
 *         - WAKEUP_EXTI_PB5: External PB5 pin
 */
WakeupSource_t BSP_LowPower_GetWakeupReason(void);

/**
 * @brief Get current estimated power consumption
 * 
 * @param mode Low power mode
 * @return Estimated current in microamps (µA)
 */
uint32_t BSP_LowPower_GetEstimatedCurrent(LowPowerMode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LOW_POWER_H */
