# SC7A20H SPI通信调试指南

## 当前问题总结

从逻辑分析仪波形分析，发现两个主要问题：

1. **SPI时钟频率异常**: 26.48 Hz (应为kHz-MHz级别)
2. **从机无响应**: MISO返回0xFF而非期望的0x11

## 排查步骤

### 第1步：检查硬件连接

```
MCU(PY32F030)          SC7A20H
-----------------------------------------
PA5 (SCK)    ------>   SCL/SPC
PA7 (MOSI)   ------>   SDA/SDI
PA6 (MISO)   <------   SDO
PA1 (CS)     ------>   CS
GND          ------>   GND
3.3V         ------>   VDD/VDDIO
```

**检查项**:
- [ ] 所有信号线连接牢固
- [ ] MISO线(PA6)是否悬空？用万用表测量
- [ ] SC7A20H供电是否正常(3.3V)
- [ ] CS片选是否有上拉电阻(建议10kΩ)

### 第2步：验证SPI模式配置

SC7A20H数据手册要求：**SPI Mode 0** 或 **Mode 3**
- Mode 0: CPOL=0, CPHA=0
- Mode 3: CPOL=1, CPHA=1 ✅ (当前配置)

**检查代码** ([bsp_spi.c](User/spi/bsp_spi.c)):
```c
LL_SPI_SetClockPhase(MEMS_SPI_INSTANCE, LL_SPI_PHASE_2EDGE);    // CPHA=1
LL_SPI_SetClockPolarity(MEMS_SPI_INSTANCE, LL_SPI_POLARITY_HIGH); // CPOL=1
```

**建议**: 如果Mode 3不工作，尝试Mode 0:
```c
LL_SPI_SetClockPhase(MEMS_SPI_INSTANCE, LL_SPI_PHASE_1EDGE);    // CPHA=0
LL_SPI_SetClockPolarity(MEMS_SPI_INSTANCE, LL_SPI_POLARITY_LOW); // CPOL=0
```

### 第3步：解决时钟频率问题

**检查系统时钟配置** ([system_py32f0xx.c](User/system_py32f0xx.c)):

期望的SPI时钟计算：
```
SPI_CLK = APB_CLK / Prescaler
例如：24MHz / 32 = 750 kHz (合理)
```

如果当前只有26Hz，说明系统时钟可能只有：
```
26 Hz × 32 = 832 Hz (显然错误！)
```

**排查**:
1. 检查 `SystemInit()` 是否正确配置了HSI/HSE
2. 检查 `SystemCoreClock` 变量值
3. 确认SPI时钟源使能

**调试代码示例**:
```c
// 在main函数开始处添加
uint32_t system_clk = HAL_RCC_GetSysClockFreq();
uint32_t apb_clk = HAL_RCC_GetPCLK1Freq();
APP_LOG("SystemClock: %lu Hz", system_clk);
APP_LOG("APB Clock: %lu Hz", apb_clk);
```

### 第4步：检查CS时序

**当前代码流程** ([sc7a20h.c](User/mems/sc7a20h.c#L34-L49)):
```c
BSP_SPI_CS_Low();                                    // 1. CS拉低
status = BSP_SPI_Transmit(reg | 0x80, timeout);      // 2. 发送地址
status = BSP_SPI_Receive(value, timeout);            // 3. 接收数据
BSP_SPI_CS_High();                                   // 4. CS拉高
```

**可能需要的改进**:
```c
BSP_SPI_CS_Low();
HAL_Delay(1);  // 添加建立时间延迟
status = BSP_SPI_Transmit(reg | 0x80, timeout);
// 不要在这里释放CS
status = BSP_SPI_Receive(value, timeout);
HAL_Delay(1);  // 添加保持时间
BSP_SPI_CS_High();
```

### 第5步：简化测试

创建最小测试代码，单独测试通信：

```c
void test_spi_loopback(void)
{
    uint8_t tx = 0xA5;
    uint8_t rx = 0x00;
    
    // 先断开MISO，短接MOSI和MISO测试环回
    BSP_SPI_CS_Low();
    BSP_SPI_TransmitReceive(tx, &rx, 100);
    BSP_SPI_CS_High();
    
    if (rx == tx) {
        APP_LOG("SPI Loopback OK");
    } else {
        APP_LOG("SPI Loopback FAIL: sent 0x%02X, got 0x%02X", tx, rx);
    }
}
```

### 第6步：使用调试输出

在关键位置添加日志：

```c
static HAL_StatusTypeDef SC7A20H_ReadReg(uint8_t reg, uint8_t *value)
{
    HAL_StatusTypeDef status;
    
    APP_LOG("Read reg 0x%02X", reg);
    
    BSP_SPI_CS_Low();
    status = BSP_SPI_Transmit(reg | SC7A20H_SPI_READ_BIT, SC7A20H_SPI_TIMEOUT);
    APP_LOG("  Sent cmd: 0x%02X, status=%d", reg | 0x80, status);
    
    if (status == HAL_OK) {
        status = BSP_SPI_Receive(value, SC7A20H_SPI_TIMEOUT);
        APP_LOG("  Received: 0x%02X, status=%d", *value, status);
    }
    
    BSP_SPI_CS_High();
    return status;
}
```

## 快速检查清单

- [ ] **硬件**: 用万用表检查SC7A20H供电(3.3V)
- [ ] **硬件**: 用示波器/逻辑分析仪验证CS信号正常翻转
- [ ] **硬件**: 确认MISO不是一直悬空(应该有上拉/下拉)
- [ ] **软件**: 打印SystemCoreClock确认系统时钟
- [ ] **软件**: 尝试降低SPI分频(DIV32->DIV64->DIV128)
- [ ] **软件**: 尝试SPI Mode 0
- [ ] **软件**: 添加CS前后延迟

## 常见原因对照表

| 症状 | 可能原因 | 解决方法 |
|------|---------|---------|
| MISO一直0xFF | CS未连接/时序错 | 检查CS连接和时序 |
| MISO一直0xFF | 从机未上电 | 检查VDD供电 |
| MISO一直0xFF | SPI模式不对 | 尝试Mode 0和Mode 3 |
| MISO一直0x00 | 上拉问题 | 检查MISO上拉 |
| 时钟超慢(Hz级) | 系统时钟未配置 | 检查SystemInit() |
| 通信不稳定 | 线太长/干扰 | 缩短连线，加滤波电容 |

## 参考资料

- SC7A20H数据手册：第4.2节 SPI接口
- PY32F030参考手册：第25章 SPI
- 当前SPI配置：[bsp_spi.c](User/spi/bsp_spi.c#L54-L61)
- 当前读取代码：[sc7a20h.c](User/mems/sc7a20h.c#L34-L49)
