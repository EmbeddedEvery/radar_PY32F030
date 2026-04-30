#include "spi/bsp_spi.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_spi.h"

static HAL_StatusTypeDef BSP_SPI_WaitFlag(uint32_t timeout, uint32_t (*flag_getter)(SPI_TypeDef *));

void BSP_SPI_Init(void)
{
    LL_GPIO_InitTypeDef gpio_init = {0};

    MEMS_SPI_GPIO_CLK_ENABLE();
    MEMS_SPI_CS_GPIO_CLK_ENABLE();
    MEMS_SPI_CLK_ENABLE();

    /* Configure SCK (PA5, AF0) */
    gpio_init.Pin = MEMS_SPI_SCK_PIN;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_UP;
    gpio_init.Alternate = MEMS_SPI_SCK_AF;
    LL_GPIO_Init(MEMS_SPI_SCK_PORT, &gpio_init);

    /* Configure MOSI (PA7, AF0) - Master Output, Slave Input */
    gpio_init.Pin = MEMS_SPI_MOSI_PIN;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_UP;
    gpio_init.Alternate = MEMS_SPI_MOSI_AF;
    LL_GPIO_Init(MEMS_SPI_MOSI_PORT, &gpio_init);

    /* Configure MISO (PA6, AF0) - Master Input, Slave Output */
    gpio_init.Pin = MEMS_SPI_MISO_PIN;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_NO;
    gpio_init.Alternate = MEMS_SPI_MISO_AF;
    LL_GPIO_Init(MEMS_SPI_MISO_PORT, &gpio_init);

    /* Configure CS (PA1) - GPIO Output */
    gpio_init.Pin = MEMS_SPI_CS_PIN;
    gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio_init.Pull = LL_GPIO_PULL_UP;
    gpio_init.Alternate = 0;
    LL_GPIO_Init(MEMS_SPI_CS_PORT, &gpio_init);

    /* Configure SPI peripheral */
    LL_SPI_Disable(MEMS_SPI_INSTANCE);
    LL_SPI_SetMode(MEMS_SPI_INSTANCE, LL_SPI_MODE_MASTER);
    LL_SPI_SetTransferDirection(MEMS_SPI_INSTANCE, LL_SPI_FULL_DUPLEX);
    LL_SPI_SetClockPhase(MEMS_SPI_INSTANCE, LL_SPI_PHASE_1EDGE);
    LL_SPI_SetClockPolarity(MEMS_SPI_INSTANCE, LL_SPI_POLARITY_LOW);
    LL_SPI_SetBaudRatePrescaler(MEMS_SPI_INSTANCE, LL_SPI_BAUDRATEPRESCALER_DIV32);
    LL_SPI_SetTransferBitOrder(MEMS_SPI_INSTANCE, LL_SPI_MSB_FIRST);
    LL_SPI_SetDataWidth(MEMS_SPI_INSTANCE, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_SetNSSMode(MEMS_SPI_INSTANCE, LL_SPI_NSS_SOFT);
    LL_SPI_SetRxFIFOThreshold(MEMS_SPI_INSTANCE, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_Enable(MEMS_SPI_INSTANCE);

    /* Clear any stale data in RX FIFO before returning */
    while (LL_SPI_IsActiveFlag_RXNE(MEMS_SPI_INSTANCE) != 0U)
    {
        (void)LL_SPI_ReceiveData8(MEMS_SPI_INSTANCE);
    }

    /* Ensure CS is HIGH after SPI is enabled */
    BSP_SPI_CS_High();
}

HAL_StatusTypeDef BSP_SPI_TransmitReceive(uint8_t tx_data, uint8_t *rx_data, uint32_t timeout)
{
    uint32_t tick_start = HAL_GetTick();

    /* Wait for TXE (TX FIFO empty) */
    while (LL_SPI_IsActiveFlag_TXE(MEMS_SPI_INSTANCE) == 0U)
    {
        if ((HAL_GetTick() - tick_start) >= timeout)
        {
            return HAL_TIMEOUT;
        }
    }

    /* Send data */
    LL_SPI_TransmitData8(MEMS_SPI_INSTANCE, tx_data);

    /* Wait for RXNE (RX FIFO has data) */
    tick_start = HAL_GetTick();
    while (LL_SPI_IsActiveFlag_RXNE(MEMS_SPI_INSTANCE) == 0U)
    {
        if ((HAL_GetTick() - tick_start) >= timeout)
        {
            return HAL_TIMEOUT;
        }
    }

    /* Receive data */
    if (rx_data != NULL)
    {
        *rx_data = LL_SPI_ReceiveData8(MEMS_SPI_INSTANCE);
    }
    else
    {
        /* Discard received data to clear FIFO */
        (void)LL_SPI_ReceiveData8(MEMS_SPI_INSTANCE);
    }

    return HAL_OK;
}

HAL_StatusTypeDef BSP_SPI_Transmit(uint8_t tx_data, uint32_t timeout)
{
    return BSP_SPI_TransmitReceive(tx_data, NULL, timeout);
}

HAL_StatusTypeDef BSP_SPI_Receive(uint8_t *rx_data, uint32_t timeout)
{
    return BSP_SPI_TransmitReceive(0xFFU, rx_data, timeout);
}

void BSP_SPI_CS_Low(void)
{
    LL_GPIO_ResetOutputPin(MEMS_SPI_CS_PORT, MEMS_SPI_CS_PIN);
}

void BSP_SPI_CS_High(void)
{
    LL_GPIO_SetOutputPin(MEMS_SPI_CS_PORT, MEMS_SPI_CS_PIN);
}

static HAL_StatusTypeDef BSP_SPI_WaitFlag(uint32_t timeout, uint32_t (*flag_getter)(SPI_TypeDef *))
{
    uint32_t tick_start = HAL_GetTick();

    while (flag_getter(MEMS_SPI_INSTANCE) == 0U)
    {
        if ((HAL_GetTick() - tick_start) >= timeout)
        {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}
