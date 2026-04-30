#ifndef __VIBRATION_SENSOR_H
#define __VIBRATION_SENSOR_H

#include "py32f0xx_hal.h"

#define VIBRATION_SENSOR_CHIP_SC7A20H      1U

#ifndef VIBRATION_SENSOR_CHIP
#define VIBRATION_SENSOR_CHIP              VIBRATION_SENSOR_CHIP_SC7A20H
#endif

#define VIBRATION_INT_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define VIBRATION_INT_PORT                 GPIOA
#define VIBRATION_INT_PIN                  GPIO_PIN_0
#define VIBRATION_INT_EXTI_LINE            EXTI_LINE_0
#define VIBRATION_INT_EXTI_GPIOSEL         EXTI_GPIOA
#define VIBRATION_INT_IRQn                 EXTI0_1_IRQn

HAL_StatusTypeDef VibrationSensor_Init(void);
void VibrationSensor_IrqHandler(void);
uint8_t VibrationSensor_HasWakeEvent(void);
void VibrationSensor_ClearWakeEvent(void);

#endif /* __VIBRATION_SENSOR_H */
