#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#include "py32f0xx_hal.h"
#include "py32f0xx_ll_spi.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_bus.h"

/* ------------------------------------------------------------------ */
/*  SPI peripheral                                                      */
/* ------------------------------------------------------------------ */
#define MEMS_SPI_INSTANCE               SPI1

/* ------------------------------------------------------------------ */
/*  Clock enables                                                       */
/* ------------------------------------------------------------------ */
#define MEMS_SPI_CLK_ENABLE()           LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SPI1)
#define MEMS_SPI_GPIO_CLK_ENABLE()      LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)
#define MEMS_SPI_CS_GPIO_CLK_ENABLE()   LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA)

/* ------------------------------------------------------------------ */
/*  SCK  : PA5  AF0                                                     */
/* ------------------------------------------------------------------ */
#define MEMS_SPI_SCK_PORT               GPIOA
#define MEMS_SPI_SCK_PIN                LL_GPIO_PIN_5
#define MEMS_SPI_SCK_AF                 LL_GPIO_AF_0

/* ------------------------------------------------------------------ */
/*  MOSI : PA7  AF0 (主机输出 -> 传感器输入 SDI)                       */
/* ------------------------------------------------------------------ */
#define MEMS_SPI_MOSI_PORT              GPIOA
#define MEMS_SPI_MOSI_PIN               LL_GPIO_PIN_7
#define MEMS_SPI_MOSI_AF                 LL_GPIO_AF_0

/* ------------------------------------------------------------------ */
/*  MISO : PA6  AF0 (主机输入 <- 传感器输出 SDO)                      */
/* ------------------------------------------------------------------ */
#define MEMS_SPI_MISO_PORT              GPIOA
#define MEMS_SPI_MISO_PIN               LL_GPIO_PIN_6
#define MEMS_SPI_MISO_AF                 LL_GPIO_AF_0

/* ------------------------------------------------------------------ */
/*  CS   : PA1  GPIO output (SW NSS)                                    */
/* ------------------------------------------------------------------ */
#define MEMS_SPI_CS_PORT                GPIOA
#define MEMS_SPI_CS_PIN                 LL_GPIO_PIN_1

/* ------------------------------------------------------------------ */
/*  API                                                                 */
/* ------------------------------------------------------------------ */
void              BSP_SPI_Init(void);
HAL_StatusTypeDef BSP_SPI_TransmitReceive(uint8_t tx_data, uint8_t *rx_data, uint32_t timeout);
HAL_StatusTypeDef BSP_SPI_Transmit(uint8_t tx_data, uint32_t timeout);
HAL_StatusTypeDef BSP_SPI_Receive(uint8_t *rx_data, uint32_t timeout);
void              BSP_SPI_CS_Low(void);
void              BSP_SPI_CS_High(void);

#endif /* __BSP_SPI_H */
