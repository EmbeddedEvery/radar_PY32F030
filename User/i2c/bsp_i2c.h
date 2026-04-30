#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "py32f0xx_hal.h"

/* ------------------------------------------------------------------ */
/*  I2C peripheral                                                     */
/* ------------------------------------------------------------------ */
#define MEMS_I2C_INSTANCE               I2C1

/* ------------------------------------------------------------------ */
/*  Clock enables                                                      */
/* ------------------------------------------------------------------ */
#define MEMS_I2C_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

/* ------------------------------------------------------------------ */
/*  SDA : PA12  (GPIO_AF_1)                                            */
/*  SCL : PA11  (GPIO_AF_1)                                            */
/* ------------------------------------------------------------------ */
#define MEMS_I2C_SDA_PORT               GPIOA
#define MEMS_I2C_SDA_PIN                GPIO_PIN_12

#define MEMS_I2C_SCL_PORT               GPIOA
#define MEMS_I2C_SCL_PIN                GPIO_PIN_11

/* ------------------------------------------------------------------ */
/*  Timing (100 kHz)                                                   */
/* ------------------------------------------------------------------ */
#define MEMS_I2C_TIMING                 0x10320309U  /* 100 kHz @ 48 MHz */

/* ------------------------------------------------------------------ */
/*  API                                                                 */
/* ------------------------------------------------------------------ */
void              BSP_I2C_Init(void);
HAL_StatusTypeDef BSP_I2C_Write(uint8_t addr, uint8_t *data, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef BSP_I2C_Read(uint8_t addr, uint8_t *data, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef BSP_I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t value, uint32_t timeout);
HAL_StatusTypeDef BSP_I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *value, uint32_t timeout);

#endif /* __BSP_I2C_H */
