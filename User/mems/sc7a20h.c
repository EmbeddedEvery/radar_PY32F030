#include "py32f0xx_hal.h"
#include "mems/sc7a20h.h"
#include "spi/bsp_spi.h"
#include "debug_log.h"

/* ------------------------------------------------------------------ */
/*  Internal timeout (ms) for SPI operations                           */
/* ------------------------------------------------------------------ */
#define SC7A20H_SPI_TIMEOUT             100U

/* ------------------------------------------------------------------ */
/*  Write one register via SPI                                         */
/* ------------------------------------------------------------------ */
static HAL_StatusTypeDef SC7A20H_WriteReg(uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef status;

    BSP_SPI_CS_Low();
    
    /* Send register address (Write: bit 7 = 0) */
    status = BSP_SPI_Transmit(reg & 0x7FU, SC7A20H_SPI_TIMEOUT);
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
/*    CTRL_REG1 = 0x57  ODR=100 Hz, normal power, X+Y+Z enabled       */
/*    CTRL_REG4 = 0x01  FS=±2 g                                       */
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
    /* ODR=100Hz, normal power, all axes enabled */
    SC7A20H_WriteReg(SC7A20H_CTRL_REG1, 0x57U);
    /* FS=±2g, SIM=0 (4-wire SPI mode, not 3-wire) */
    SC7A20H_WriteReg(SC7A20H_CTRL_REG4, 0x00U);

    APP_LOG("SC7A20H: init ok, thr=0x%02X dur=0x%02X",
            config->threshold, config->duration);

    return HAL_OK;
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_ClearInterrupt                                              */
/*  Dummy function for I2C (not needed, interrupt handling is in      */
/*  vibration_sensor.c EXTI callback).                                 */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_ClearInterrupt(void)
{
    return HAL_OK;
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_ReadAccel                                                   */
/*  Read X, Y, Z acceleration data from output registers               */
/*  Registers: 0x28-0x29 (X), 0x2A-0x2B (Y), 0x2C-0x2D (Z)             */
/* ------------------------------------------------------------------ */
HAL_StatusTypeDef SC7A20H_ReadAccel(SC7A20H_AccelData *accel)
{
    HAL_StatusTypeDef status;
    uint8_t x_l, x_h, y_l, y_h, z_l, z_h;

    if (accel == NULL)
    {
        return HAL_ERROR;
    }

    /* Read X-axis data (OUT_X_L=0x28, OUT_X_H=0x29) */
    status = SC7A20H_ReadReg(0x28U, &x_l);
    if (status != HAL_OK) return status;

    status = SC7A20H_ReadReg(0x29U, &x_h);
    if (status != HAL_OK) return status;

    /* Read Y-axis data (OUT_Y_L=0x2A, OUT_Y_H=0x2B) */
    status = SC7A20H_ReadReg(0x2AU, &y_l);
    if (status != HAL_OK) return status;

    status = SC7A20H_ReadReg(0x2BU, &y_h);
    if (status != HAL_OK) return status;

    /* Read Z-axis data (OUT_Z_L=0x2C, OUT_Z_H=0x2D) */
    status = SC7A20H_ReadReg(0x2CU, &z_l);
    if (status != HAL_OK) return status;

    status = SC7A20H_ReadReg(0x2DU, &z_h);
    if (status != HAL_OK) return status;

    /* Combine low and high bytes (little-endian format) */
    accel->x = (int16_t)((x_h << 8) | x_l);
    accel->y = (int16_t)((y_h << 8) | y_l);
    accel->z = (int16_t)((z_h << 8) | z_l);

    return HAL_OK;
}

/* ------------------------------------------------------------------ */
/*  SC7A20H_PrintAccel                                                  */
/*  Read and print X, Y, Z acceleration data                            */
/* ------------------------------------------------------------------ */
void SC7A20H_PrintAccel(void)
{
    SC7A20H_AccelData accel;
    HAL_StatusTypeDef status;

    status = SC7A20H_ReadAccel(&accel);
    if (status == HAL_OK)
    {
        APP_LOG("Accel - X: %6d, Y: %6d, Z: %6d (mg)",
                accel.x, accel.y, accel.z);
    }
    else
    {
        APP_LOG("Accel - Failed to read acceleration data (status=%d)", status);
    }
}

