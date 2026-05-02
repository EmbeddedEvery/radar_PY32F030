#ifndef __SC7A20H_H
#define __SC7A20H_H

#include "py32f0xx_hal.h"
#include "spi/bsp_spi.h"

/* ------------------------------------------------------------------ */
/*  SPI Specific defines                                               */
/* ------------------------------------------------------------------ */
#define SC7A20H_SPI_READ_BIT            0x80U
#define SC7A20H_SPI_MS_BIT              0x40U  /* Multiple byte read/write if needed */

/* ------------------------------------------------------------------ */
/*  Device identity                                                     */
/* ------------------------------------------------------------------ */
#define SC7A20H_WHO_AM_I_REG            0x0FU
#define SC7A20H_EXPECTED_ID             0x11U

/* ------------------------------------------------------------------ */
/*  Control registers                                                   */
/* ------------------------------------------------------------------ */
#define SC7A20H_CTRL_REG1               0x20U
#define SC7A20H_CTRL_REG2               0x21U
#define SC7A20H_CTRL_REG3               0x22U
#define SC7A20H_CTRL_REG4               0x23U
#define SC7A20H_CTRL_REG5               0x24U
#define SC7A20H_CTRL_REG6               0x25U

/* CTRL_REG2 bits */
#define SC7A20H_CTRL_REG2_BOOT          0x80U  /* Boot bit for soft reset */

/* ------------------------------------------------------------------ */
/*  INT1 registers                                                      */
/* ------------------------------------------------------------------ */
#define SC7A20H_INT1_CFG                0x30U
#define SC7A20H_INT1_SRC                0x31U
#define SC7A20H_INT1_THS                0x32U
#define SC7A20H_INT1_DURATION           0x33U

/* ------------------------------------------------------------------ */
/*  Wakeup configuration                                                */
/* ------------------------------------------------------------------ */
typedef struct
{
    uint8_t threshold;  /*!< Wakeup threshold  (1 LSB ≈ 16 mg @ ±2 g) */
    uint8_t duration;   /*!< Wakeup duration   (1 LSB = 1 / ODR)       */
} SC7A20H_WakeupConfig;

/* ------------------------------------------------------------------ */
/*  Acceleration data                                                   */
/* ------------------------------------------------------------------ */
typedef struct
{
    int16_t x;  /*!< X-axis acceleration */
    int16_t y;  /*!< Y-axis acceleration */
    int16_t z;  /*!< Z-axis acceleration */
} SC7A20H_AccelData;

/* ------------------------------------------------------------------ */
/*  API                                                                 */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_Init(const SC7A20H_WakeupConfig *config);
HAL_StatusTypeDef SC7A20H_ReadWhoAmI(uint8_t *id);
HAL_StatusTypeDef SC7A20H_ReadAccel(SC7A20H_AccelData *accel);
void SC7A20H_PrintAccel(void);
HAL_StatusTypeDef SC7A20H_ClearInterrupt(void);

#endif /* __SC7A20H_H */
