#include "py32f0xx_hal.h"
#include "mems/sc7a20h.h"
#include "spi/bsp_spi.h"
#include "debug_log.h"
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Internal timeout (ms) for SPI operations                           */
/* ------------------------------------------------------------------ */
#define SC7A20H_SPI_TIMEOUT             100U

/* Sensitivity constants for different ranges */
/* FS[1:0] = 00: ±2g, 1024 LSB/g */
#define SC7A20H_SENSITIVITY_2G_NUM  1000L
#define SC7A20H_SENSITIVITY_2G_DEN  1024L

/* FS[1:0] = 01: ±4g, 512 LSB/g */
#define SC7A20H_SENSITIVITY_4G_NUM  2000L
#define SC7A20H_SENSITIVITY_4G_DEN  1024L

/* FS[1:0] = 10: ±8g, 256 LSB/g */
#define SC7A20H_SENSITIVITY_8G_NUM  4000L
#define SC7A20H_SENSITIVITY_8G_DEN  1024L

/* FS[1:0] = 11: ±16g, 128 LSB/g */
#define SC7A20H_SENSITIVITY_16G_NUM 8000L
#define SC7A20H_SENSITIVITY_16G_DEN 1024L

/* Conversion for ±2g */
int16_t SC7A20H_RawToMg_2G(int16_t raw)
{
    int32_t mg = ((int32_t)raw * SC7A20H_SENSITIVITY_2G_NUM) / SC7A20H_SENSITIVITY_2G_DEN;
    return (int16_t)mg;
}

/* Conversion for ±4g */
int16_t SC7A20H_RawToMg_4G(int16_t raw)
{
    int32_t mg = ((int32_t)raw * SC7A20H_SENSITIVITY_4G_NUM) / SC7A20H_SENSITIVITY_4G_DEN;
    return (int16_t)mg;
}

/* Conversion for ±8g */
int16_t SC7A20H_RawToMg_8G(int16_t raw)
{
    int32_t mg = ((int32_t)raw * SC7A20H_SENSITIVITY_8G_NUM) / SC7A20H_SENSITIVITY_8G_DEN;
    return (int16_t)mg;
}

/* Conversion for ±16g */
int16_t SC7A20H_RawToMg_16G(int16_t raw)
{
    int32_t mg = ((int32_t)raw * SC7A20H_SENSITIVITY_16G_NUM) / SC7A20H_SENSITIVITY_16G_DEN;
    return (int16_t)mg;
}

/* Default conversion function (±2g) */
static int16_t SC7A20H_RawToMg(int16_t raw)
{
    return SC7A20H_RawToMg_2G(raw);
}

/* ------------------------------------------------------------------ */
/*  Write one register via SPI                                         */
/* ------------------------------------------------------------------ */
static HAL_StatusTypeDef SC7A20H_WriteReg(uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef status;

    BSP_SPI_CS_Low();
    
    /* Send register address (Write: bit 7 = 0) */
    status = BSP_SPI_Transmit(reg & 0x3FU, SC7A20H_SPI_TIMEOUT);
    if (status == HAL_OK)
    {
        /* Send data */
        status = BSP_SPI_Transmit(value, SC7A20H_SPI_TIMEOUT);
    }

    BSP_SPI_CS_High();
    return status;
}

/* ------------------------------------------------------------------ */
/*  Read one register via SPI                                          */
/* ------------------------------------------------------------------ */
static HAL_StatusTypeDef SC7A20H_ReadReg(uint8_t reg, uint8_t *value)
{
    HAL_StatusTypeDef status;

    BSP_SPI_CS_Low();

    /* 1. 发送寄存器地址 (Read: bit 7 = 1) */
    status = BSP_SPI_Transmit(reg | SC7A20H_SPI_READ_BIT, SC7A20H_SPI_TIMEOUT);
    if (status == HAL_OK)
    {
        /* 2. 紧接着接收数据 (此时主机自动发送 0xFF 以产生时钟) */
        status = BSP_SPI_Receive(value, SC7A20H_SPI_TIMEOUT);
    }

    BSP_SPI_CS_High();
    return status;
}

/* ------------------------------------------------------------------ */
/*  Read multiple registers via SPI (with auto-increment)              */
/* ------------------------------------------------------------------ */
static HAL_StatusTypeDef SC7A20H_ReadRegMulti(uint8_t reg, uint8_t *data, uint8_t len)
{
    HAL_StatusTypeDef status;

    BSP_SPI_CS_Low();

    /* Send register address with auto-increment bit (bit 6 = 1) and read bit (bit 7 = 1) */
    uint8_t reg_addr = reg | SC7A20H_SPI_READ_BIT | SC7A20H_SPI_MS_BIT;
    status = BSP_SPI_Transmit(reg_addr, SC7A20H_SPI_TIMEOUT);
    
    if (status == HAL_OK)
    {
        /* Receive multiple bytes with auto-increment */
        for (uint8_t i = 0; i < len; i++)
        {
            status = BSP_SPI_Receive(&data[i], SC7A20H_SPI_TIMEOUT);
            if (status != HAL_OK) break;
        }
    }

    BSP_SPI_CS_High();
    return status;
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_SetFullScale                                                */
/*  Set the full-scale range of the accelerometer                      */
/*  fs_value: SC7A20H_CTRL_REG4_FS_2G/4G/8G/16G                        */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_SetFullScale(uint8_t fs_value)
{
    uint8_t ctrl_reg4 = 0x00U;
    HAL_StatusTypeDef status;

    /* Read current CTRL_REG4 */
    status = SC7A20H_ReadReg(SC7A20H_CTRL_REG4, &ctrl_reg4);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Clear FS bits [5:4] and set new value */
    ctrl_reg4 = (ctrl_reg4 & 0xCFU) | (fs_value & 0x30U);
    
    /* Write back */
    return SC7A20H_WriteReg(SC7A20H_CTRL_REG4, ctrl_reg4);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_GetFullScale                                                */
/*  Get the current full-scale range setting from CTRL_REG4            */
/*  Returns: 0=±2g, 1=±4g, 2=±8g, 3=±16g (-1 on error)               */
/* ------------------------------------------------------------------ */
int8_t SC7A20H_GetFullScale(void)
{
    uint8_t ctrl_reg4 = 0x00U;
    HAL_StatusTypeDef status;

    /* Read current CTRL_REG4 */
    status = SC7A20H_ReadReg(SC7A20H_CTRL_REG4, &ctrl_reg4);
    if (status != HAL_OK)
    {
        return -1;  /* Error */
    }

    /* Extract FS bits [5:4] and shift right to get 0-3 */
    return (int8_t)((ctrl_reg4 >> 4) & 0x03U);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_SetODR                                                      */
/*  Set the output data rate (sampling frequency)                      */
/*  odr_value: SC7A20H_CTRL_REG1_ODR_100HZ/200HZ/400HZ                 */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_SetODR(uint8_t odr_value)
{
    uint8_t ctrl_reg1 = 0x00U;
    HAL_StatusTypeDef status;

    /* Read current CTRL_REG1 */
    status = SC7A20H_ReadReg(SC7A20H_CTRL_REG1, &ctrl_reg1);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Clear ODR bits [7:6] and set new value */
    ctrl_reg1 = (ctrl_reg1 & 0x3FU) | (odr_value & 0xC0U);
    
    /* Write back */
    return SC7A20H_WriteReg(SC7A20H_CTRL_REG1, ctrl_reg1);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_SetLowPowerMode                                             */
/*  Enable or disable low power mode                                   */
/*  enable: 1 = enable LPen, 0 = disable LPen                          */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_SetLowPowerMode(uint8_t enable)
{
    uint8_t ctrl_reg1 = 0x00U;
    HAL_StatusTypeDef status;

    /* Read current CTRL_REG1 */
    status = SC7A20H_ReadReg(SC7A20H_CTRL_REG1, &ctrl_reg1);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Clear LPen bit [5] */
    ctrl_reg1 &= 0xDFU;
    
    /* Set LPen if enabled */
    if (enable)
    {
        ctrl_reg1 |= SC7A20H_CTRL_REG1_LPEN_ENABLED;
    }
    
    /* Write back */
    return SC7A20H_WriteReg(SC7A20H_CTRL_REG1, ctrl_reg1);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_SetAxis                                                     */
/*  Enable or disable individual axes                                  */
/*  axis_mask: combination of X/Y/Z enable bits                        */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_SetAxis(uint8_t axis_mask)
{
    uint8_t ctrl_reg1 = 0x00U;
    HAL_StatusTypeDef status;

    /* Read current CTRL_REG1 */
    status = SC7A20H_ReadReg(SC7A20H_CTRL_REG1, &ctrl_reg1);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Clear axis bits [4:2] and set new value */
    ctrl_reg1 = (ctrl_reg1 & 0xF8U) | (axis_mask & 0x07U);
    
    /* Write back */
    return SC7A20H_WriteReg(SC7A20H_CTRL_REG1, ctrl_reg1);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_ReadWhoAmI                                                  */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_ReadWhoAmI(uint8_t *id)
{
    return SC7A20H_ReadReg(SC7A20H_WHO_AM_I_REG, id);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_Init                                                        */
/*                                                                      */
/*  Register configuration:                                             */
/*    CTRL_REG1 = 0x5F  ODR=100 Hz, LPen=1, X+Y+Z enabled             */
/*    CTRL_REG4 = 0x00  FS=±2 g                                       */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_Init(const SC7A20H_WakeupConfig *config)
{
    HAL_StatusTypeDef status;
    uint8_t who_am_i = 0U;
    uint8_t retry_count = 0U;

    if (config == NULL)
    {
        return HAL_ERROR;
    }

    /* Power-up delay */
    HAL_Delay(100);

    /* Perform soft reset via BOOT bit in CTRL_REG2 */
    APP_LOG("SC7A20H: Performing soft reset...");
    SC7A20H_WriteReg(SC7A20H_CTRL_REG2, SC7A20H_CTRL_REG2_BOOT);
    
    /* Wait for reset to complete (typically 5-10ms) */
    HAL_Delay(100);

    /* Try to read WHO_AM_I up to 5 times */
    for (retry_count = 0U; retry_count < 5U; retry_count++)
    {
        status = SC7A20H_ReadWhoAmI(&who_am_i);
        
        APP_LOG("SC7A20H: WHO_AM_I=0x%02X (expected 0x%02X), attempt %d",
                who_am_i, SC7A20H_EXPECTED_ID, retry_count + 1);
        
        if (status == HAL_OK && who_am_i == SC7A20H_EXPECTED_ID)
        {
            APP_LOG("SC7A20H: Chip detected successfully");
            break;  /* Success */
        }
        
        if (retry_count < 4U)
        {
            APP_LOG("SC7A20H: Retrying after reset...");
            HAL_Delay(100);
        }
    }

    if (status != HAL_OK)
    {
        APP_LOG("SC7A20H: SPI read error");
        return HAL_ERROR;
    }

    if (who_am_i != SC7A20H_EXPECTED_ID)
    {
        APP_LOG("SC7A20H: WHO_AM_I mismatch! Got 0x%02X, expected 0x%02X", who_am_i, SC7A20H_EXPECTED_ID);
        APP_LOG("SC7A20H: Scanning registers 0x00-0x40 to find valid data...");
        
        for (uint8_t scan_addr = 0x00U; scan_addr <= 0x40U; scan_addr++)
        {
            uint8_t scan_val = 0U;
            HAL_StatusTypeDef scan_status = SC7A20H_ReadReg(scan_addr, &scan_val);
            if (scan_status == HAL_OK && scan_val != 0xFFU && scan_val != 0x00U)
            {
                APP_LOG("  Reg[0x%02X] = 0x%02X", scan_addr, scan_val);
                if (scan_val == 0x11U)
                {
                    APP_LOG("  ^ Found 0x11 at 0x%02X (expected at 0x0F)", scan_addr);
                }
            }
        }
        
        APP_LOG("SC7A20H: chip not found");
        return HAL_ERROR;
    }

    /* --- Configure device --- */
    /* ODR=100Hz, LPen=1, XYZ enabled, FS=±4g */
    SC7A20H_SetODR(SC7A20H_CTRL_REG1_ODR_100HZ);
    SC7A20H_SetLowPowerMode(1);
    SC7A20H_SetAxis(SC7A20H_CTRL_REG1_XYZ_ENABLE);
    SC7A20H_SetFullScale(SC7A20H_CTRL_REG4_FS_4G);

    /* Route AOI1 (Interrupt 1) to physical INT1 pin */
    SC7A20H_WriteReg(SC7A20H_CTRL_REG3, SC7A20H_CTRL_REG3_I1_AOI1);

    /* Latch Interrupt 1 request on INT1_SRC register (remains active until INT1_SRC is read) */
    SC7A20H_WriteReg(SC7A20H_CTRL_REG5, SC7A20H_CTRL_REG5_LIR_INT1);

    /* Configure INT1 wakeup threshold and duration */
    /* Enable X, Y, Z high event interrupts only (no low event to avoid triggering when static) */
    uint8_t int1_cfg = SC7A20H_INT1_CFG_XHIE |
                       SC7A20H_INT1_CFG_YHIE |
                       SC7A20H_INT1_CFG_ZHIE;
    (void)SC7A20H_SetInt1Config(int1_cfg, config->threshold, config->duration);

    APP_LOG("SC7A20H: init ok, thr=0x%02X dur=0x%02X",
            config->threshold, config->duration);

    return HAL_OK;
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_SetInt1Config                                               */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_SetInt1Config(uint8_t int1_cfg, uint8_t threshold, uint8_t duration)
{
    HAL_StatusTypeDef status;

    status = SC7A20H_WriteReg(SC7A20H_INT1_CFG, int1_cfg);
    if (status != HAL_OK) {
        return status;
    }

    status = SC7A20H_WriteReg(SC7A20H_INT1_THS, threshold);
    if (status != HAL_OK) {
        return status;
    }

    return SC7A20H_WriteReg(SC7A20H_INT1_DURATION, duration);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_ClearInterrupt                                              */
/*  Read INT1 source register to clear pending interrupt status        */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_ClearInterrupt(void)
{
    uint8_t src = 0U;
    return SC7A20H_ReadReg(SC7A20H_INT1_SRC, &src);
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_ReadAccel                                                   */
/*  Read X, Y, Z acceleration data from output registers               */
/*  Registers: 0x28-0x29 (X), 0x2A-0x2B (Y), 0x2C-0x2D (Z)             */
/*  Uses multi-byte read mode with auto-increment                      */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_ReadAccel(SC7A20H_AccelData *accel)
{
    HAL_StatusTypeDef status;
    uint8_t buf[6] = {0};

    if (accel == NULL)
    {
        return HAL_ERROR;
    }

    /* Read all 6 bytes in one transaction (0x28-0x2D) with auto-increment */
    status = SC7A20H_ReadRegMulti(0x28U, buf, 6);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Combine low and high bytes and convert from 12-bit to 16-bit */
    /* SC7A20H outputs 12-bit data in the high 12 bits, need to right shift by 4 bits */
    accel->x = (int16_t)(((int16_t)((buf[1] << 8) | buf[0])) >> 4);
    accel->y = (int16_t)(((int16_t)((buf[3] << 8) | buf[2])) >> 4);
    accel->z = (int16_t)(((int16_t)((buf[5] << 8) | buf[4])) >> 4);

    return HAL_OK;
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_PrintAccel                                                  */
/*  Read and print X, Y, Z acceleration data from output registers     */
/*  Automatically detects FS range and uses appropriate conversion     */
/* ------------------------------------------------------------------ */
void SC7A20H_PrintAccel(void)
{
    SC7A20H_AccelData accel;
    HAL_StatusTypeDef status;
    int16_t x_mg, y_mg, z_mg;
    int8_t fs_range;

    status = SC7A20H_ReadAccel(&accel);
    if (status == HAL_OK)
    {
        /* Get current FS range from CTRL_REG4 */
        fs_range = SC7A20H_GetFullScale();

        /* Convert to mg based on detected range */
        switch (fs_range)
        {
            case 0:  /* ±2g */
                x_mg = SC7A20H_RawToMg_2G(accel.x);
                y_mg = SC7A20H_RawToMg_2G(accel.y);
                z_mg = SC7A20H_RawToMg_2G(accel.z);
                break;
            case 1:  /* ±4g */
                x_mg = SC7A20H_RawToMg_4G(accel.x);
                y_mg = SC7A20H_RawToMg_4G(accel.y);
                z_mg = SC7A20H_RawToMg_4G(accel.z);
                break;
            case 2:  /* ±8g */
                x_mg = SC7A20H_RawToMg_8G(accel.x);
                y_mg = SC7A20H_RawToMg_8G(accel.y);
                z_mg = SC7A20H_RawToMg_8G(accel.z);
                break;
            case 3:  /* ±16g */
                x_mg = SC7A20H_RawToMg_16G(accel.x);
                y_mg = SC7A20H_RawToMg_16G(accel.y);
                z_mg = SC7A20H_RawToMg_16G(accel.z);
                break;
            default:  /* Error reading FS range, default to ±2g */
                x_mg = SC7A20H_RawToMg_2G(accel.x);
                y_mg = SC7A20H_RawToMg_2G(accel.y);
                z_mg = SC7A20H_RawToMg_2G(accel.z);
                break;
        }

        APP_LOG("Accel - X:%6d Y:%6d Z:%6d (mg) [FS:%dg]", x_mg, y_mg, z_mg, (1 << fs_range) * 2);
    }
    else
    {
        APP_LOG("Accel - Failed to read acceleration data (status=%d)", status);
    }
}

