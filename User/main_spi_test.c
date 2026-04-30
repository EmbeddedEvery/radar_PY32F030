/**
 * @file    spi_test.c (临时诊断代码)
 * 
 * 目的：逐字节观察 SPI 总线上发生了什么
 * 编译后添加到 main() 中 VibrationSensor_Init() 前面
 */

#include "spi/bsp_spi.h"
#include "debug_log.h"

void SPI_DiagnosticTest(void)
{
    uint8_t rx_val = 0;
    HAL_StatusTypeDef status;

    APP_LOG("=== SPI Diagnostic Start ===");
    
    /* 尝试读 WHO_AM_I，观察返回值 */
    BSP_SPI_CS_Low();
    APP_LOG("CS pulled LOW");
    
    /* 发送读命令 (0x8F = 0x0F | 0x80) */
    status = BSP_SPI_Transmit(0x8FU, 10);
    APP_LOG("Sent 0x8F, status=%d", status);
    
    /* 发送 dummy byte 接收数据 */
    status = BSP_SPI_Receive(&rx_val, 10);
    APP_LOG("Received 0x%02X, status=%d", rx_val, status);
    
    BSP_SPI_CS_High();
    APP_LOG("CS pulled HIGH");

    /* 分析返回值 */
    if (rx_val == 0x11)
    {
        APP_LOG("SUCCESS: Got expected WHO_AM_I=0x11");
    }
    else if (rx_val == 0xFF)
    {
        APP_LOG("ERROR: Got 0xFF - likely CS not toggling or MISO floating");
        APP_LOG("  → Check: CS pin wired? MISO pin wired?");
        APP_LOG("  → Try: invert CS logic in bsp_spi.c");
    }
    else if (rx_val == 0x00)
    {
        APP_LOG("ERROR: Got 0x00 - possible SPI mode wrong (try mode 0)");
    }
    else
    {
        APP_LOG("Got 0x%02X - unexpected value", rx_val);
    }
    
    APP_LOG("=== SPI Diagnostic End ===");
}
