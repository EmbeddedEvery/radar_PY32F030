#include "i2c/bsp_i2c.h"

static I2C_HandleTypeDef hi2c = {0};

void BSP_I2C_Init(void)
{
    GPIO_InitTypeDef gpio_init = {0};

    MEMS_I2C_GPIO_CLK_ENABLE();
    
    /* Enable I2C1 clock manually */
    RCC->APBENR1 |= (1 << 21);  /* I2C1EN bit */

    /* Configure SCL pin (PA11) */
    gpio_init.Pin       = MEMS_I2C_SCL_PIN;
    gpio_init.Mode      = GPIO_MODE_AF_OD;
    gpio_init.Pull      = GPIO_PULLUP;
    gpio_init.Speed     = GPIO_SPEED_FREQ_HIGH;
    gpio_init.Alternate = 6;  /* I2C1 on AF6 for PA11 */
    HAL_GPIO_Init(MEMS_I2C_SCL_PORT, &gpio_init);

    /* Configure SDA pin (PA12) */
    gpio_init.Pin       = MEMS_I2C_SDA_PIN;
    gpio_init.Alternate = 6;  /* I2C1 on AF6 for PA12 */
    HAL_GPIO_Init(MEMS_I2C_SDA_PORT, &gpio_init);

    /* Configure I2C */
    hi2c.Instance             = MEMS_I2C_INSTANCE;
    hi2c.Init.ClockSpeed      = 100000;      /* 100 kHz */
    hi2c.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c.Init.OwnAddress1     = 0x00;
    hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    HAL_I2C_Init(&hi2c);
}

HAL_StatusTypeDef BSP_I2C_Write(uint8_t addr, uint8_t *data, uint16_t len, uint32_t timeout)
{
    if (data == NULL || len == 0)
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Master_Transmit(&hi2c, (addr << 1), data, len, timeout);
}

HAL_StatusTypeDef BSP_I2C_Read(uint8_t addr, uint8_t *data, uint16_t len, uint32_t timeout)
{
    if (data == NULL || len == 0)
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Master_Receive(&hi2c, (addr << 1) | 1, data, len, timeout);
}

HAL_StatusTypeDef BSP_I2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t value, uint32_t timeout)
{
    uint8_t buf[2] = {reg, value};

    return BSP_I2C_Write(addr, buf, 2, timeout);
}

HAL_StatusTypeDef BSP_I2C_ReadReg(uint8_t addr, uint8_t reg, uint8_t *value, uint32_t timeout)
{
    HAL_StatusTypeDef status;

    /* Send register address */
    status = BSP_I2C_Write(addr, &reg, 1, timeout);
    if (status != HAL_OK)
    {
        return status;
    }

    /* Read register value */
    return BSP_I2C_Read(addr, value, 1, timeout);
}
