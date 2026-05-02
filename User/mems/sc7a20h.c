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
        APP_LOG("SC7A20H: chip not found");
        return HAL_ERROR;
    }

    /* --- Configure device --- */
    /* ODR=100Hz, normal power, all axes enabled */
    SC7A20H_WriteReg(SC7A20H_CTRL_REG1, 0x57U);
    /* FS=±2g */
    SC7A20H_WriteReg(SC7A20H_CTRL_REG4, 0x01U);

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

