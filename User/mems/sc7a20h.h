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
/*  CTRL_REG1 configuration macros                                     */
/* ------------------------------------------------------------------ */
/* ODR selection (bits [7:6]) */
#define SC7A20H_CTRL_REG1_ODR_POWER_DOWN    0x00U  /* Power-down mode */
#define SC7A20H_CTRL_REG1_ODR_100HZ         0x40U  /* 100 Hz */
#define SC7A20H_CTRL_REG1_ODR_200HZ         0x80U  /* 200 Hz */
#define SC7A20H_CTRL_REG1_ODR_400HZ         0xC0U  /* 400 Hz */

/* Low power mode (bit [5]) */
#define SC7A20H_CTRL_REG1_LPEN_DISABLED     0x00U
#define SC7A20H_CTRL_REG1_LPEN_ENABLED      0x20U

/* Axis enable (bits [4:2]) - XYZ */
#define SC7A20H_CTRL_REG1_Z_ENABLE          0x04U
#define SC7A20H_CTRL_REG1_Y_ENABLE          0x02U
#define SC7A20H_CTRL_REG1_X_ENABLE          0x01U
#define SC7A20H_CTRL_REG1_XYZ_ENABLE        0x07U

/* Common CTRL_REG1 configurations */
#define SC7A20H_CTRL_REG1_100HZ_NORMAL_XYZ  (SC7A20H_CTRL_REG1_ODR_100HZ | SC7A20H_CTRL_REG1_XYZ_ENABLE)
#define SC7A20H_CTRL_REG1_100HZ_LPMODE_XYZ  (SC7A20H_CTRL_REG1_ODR_100HZ | SC7A20H_CTRL_REG1_LPEN_ENABLED | SC7A20H_CTRL_REG1_XYZ_ENABLE)
#define SC7A20H_CTRL_REG1_200HZ_NORMAL_XYZ  (SC7A20H_CTRL_REG1_ODR_200HZ | SC7A20H_CTRL_REG1_XYZ_ENABLE)
#define SC7A20H_CTRL_REG1_200HZ_LPMODE_XYZ  (SC7A20H_CTRL_REG1_ODR_200HZ | SC7A20H_CTRL_REG1_LPEN_ENABLED | SC7A20H_CTRL_REG1_XYZ_ENABLE)
#define SC7A20H_CTRL_REG1_400HZ_NORMAL_XYZ  (SC7A20H_CTRL_REG1_ODR_400HZ | SC7A20H_CTRL_REG1_XYZ_ENABLE)
#define SC7A20H_CTRL_REG1_400HZ_LPMODE_XYZ  (SC7A20H_CTRL_REG1_ODR_400HZ | SC7A20H_CTRL_REG1_LPEN_ENABLED | SC7A20H_CTRL_REG1_XYZ_ENABLE)

/* ------------------------------------------------------------------ */
/*  CTRL_REG4 configuration macros                                     */
/* ------------------------------------------------------------------ */
/* Full-scale selection (bits [5:4]) */
#define SC7A20H_CTRL_REG4_FS_2G             0x00U  /* ±2g */
#define SC7A20H_CTRL_REG4_FS_4G             0x10U  /* ±4g */
#define SC7A20H_CTRL_REG4_FS_8G             0x20U  /* ±8g */
#define SC7A20H_CTRL_REG4_FS_16G            0x30U  /* ±16g */

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

/* Conversion functions for different ranges */
int16_t SC7A20H_RawToMg_2G(int16_t raw);  /* ±2g range */
int16_t SC7A20H_RawToMg_4G(int16_t raw);  /* ±4g range */
int16_t SC7A20H_RawToMg_8G(int16_t raw);  /* ±8g range */
int16_t SC7A20H_RawToMg_16G(int16_t raw); /* ±16g range */

/* Configuration functions */
HAL_StatusTypeDef SC7A20H_SetFullScale(uint8_t fs_value);  /* Set full scale: FS_2G/4G/8G/16G */
int8_t SC7A20H_GetFullScale(void);                         /* Get current full scale (0=±2g, 1=±4g, 2=±8g, 3=±16g) */
HAL_StatusTypeDef SC7A20H_SetODR(uint8_t odr_value);       /* Set output data rate */
HAL_StatusTypeDef SC7A20H_SetLowPowerMode(uint8_t enable); /* Enable/disable low power mode */
HAL_StatusTypeDef SC7A20H_SetAxis(uint8_t axis_mask);      /* Enable/disable axis: X/Y/Z */

#endif /* __SC7A20H_H */
