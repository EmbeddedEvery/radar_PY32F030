# PY32F030xx 低功耗振动唤醒开发项目

本项目基于普冉（Puya）**PY32F030xx**（ARM Cortex-M0+ 内核，主频可达 48MHz）微控制器，配合士兰微（Silan）**SC7A20H** 三轴 MEMS 加速度计/振动传感器，实现了高可靠性、超低功耗的**外部振动唤醒系统**。

## 📌 项目亮点

1. **统一传感器抽象接口**：提供通用的振动传感器 API 接口（`VibrationSensor_Init` / `VibrationSensor_HasWakeEvent` 等），支持通过宏定义一键切换不同的传感器芯片，驱动层与应用层完美分离。
2. **多级别低功耗管理**：完整支持 Sleep、Stop 和 Standby 模式，其中 Standby 模式下功耗可低至 **4.5µA**（包含所有未用 GPIO 的防漏电配置）。
3. **SPI 诊断与调试机制**：包含完整的 SPI 诊断测试代码（`main_spi_test.c`）和串口日志重定向输出（`debug_log.h`），能够快速定位总线异常。

---

## 🔌 硬件连接与引脚分配

本样例基于官方开发板 **PY32F030_STK** 进行配置，MCU 与外设连接如下：

| MCU 引脚 | 信号名称 | 方向 | 对接外设 (SC7A20H / 开发板) | 功能描述 |
| :--- | :--- | :---: | :--- | :--- |
| **PA5** | SPI1_SCK | 输出 | SC7A20H SCL/SPC | SPI 串行时钟信号 |
| **PA7** | SPI1_MOSI | 输出 | SC7A20H SDA/SDI | SPI 主机输出从机输入 |
| **PA6** | SPI1_MISO | 输入 | SC7A20H SDO | SPI 主机输入从机输出 |
| **PA1** | SPI1_CS | 输出 | SC7A20H CS | SPI 硬件/软件片选信号 |
| **PA0** | EXTI0 | 输入 | SC7A20H INT1 | 振动中断输入引脚（上升沿触发唤醒） |
| **PA2** | USART1_TX | 输出 | 串口转USB模块 RXD | 调试串口发送（波特率 115200） |
| **PA11** | LED2 | 输出 | 板载绿色 LED | 运行状态指示灯 |
| **PA12** | LED3 | 输出 | 板载蓝色 LED | 错误状态指示灯 |

> ⚠️ **注意**：
> - SC7A20H 模块供电必须稳定在 **3.3V**（VDD/VDDIO）。
> - 片选引脚 (CS) 推荐外接 10kΩ 上拉电阻以防止总线浮动。
> - 在低功耗模式下，为防止引脚浮空产生漏电流，未使用的 GPIO 应通过软件配置为模拟输入或带下拉输入状态。

---

## 📂 项目目录结构说明

```text
GPIO_Toggle/
├── Drivers/                    # 固件库文件
│   ├── BSP/                    # 官方开发板支持包
│   ├── CMSIS/                  # ARM Cortex-M0+ 内核支持文件
│   └── PY32F0xx_HAL_Driver/    # 普冉官方的 HAL 与 LL 外设驱动库
├── MDK-ARM/                    # Keil MDK 工程目录
│   └── Project.uvprojx         # Keil 5 工程文件
├── EWARM/                      # IAR Embedded Workbench 工程目录
│   └── Project.eww             # IAR 工程主工作区
├── PDF/                        # 硬件规格书与设计参考资料
├── User/                       # 用户应用层与 BSP 驱动目录
│   ├── main.c                  # 应用程序主入口
│   ├── main.h                  # 主头文件
│   ├── main_spi_test.c         # SPI 诊断调试代码
│   ├── debug_log.h             # 串口日志宏定义封装
│   ├── mems/                   # MEMS 振动传感器驱动
│   │   ├── sc7a20h.c/h         # SC7A20H 寄存器配置与底层驱动
│   │   └── vibration_sensor.c/h# 抽离的统一振动传感器抽象接口层
│   ├── spi/                    # SPI 硬件底层封装
│   │   └── bsp_spi.c/h         # 基于 LL 库的 SPI 初始化及读写 API
│   ├── low_power/              # 低功耗管理模块
│   │   ├── bsp_low_power.c/h   # Sleep/Stop/Standby 状态机及漏电配置
│   │   └── LOW_POWER_GUIDE.md  # 详细的低功耗设计指南
│   ├── usart/                  # 串口调试模块
│   │   └── bsp_usart.c/h       # 串口初始化与 printf 重定向
│   ├── led/                    # LED 控制模块
│   ├── key/                    # 按键输入模块
│   ├── adc/                    # 模拟模数转换模块
│   ├── i2c/                    # I2C 通信接口
│   └── exit/                   # 外部中断管理模块
├── SPI_DEBUG_GUIDE.md          # SPI通信硬件调试步骤及常见问题对照表
└── sc7a20h_BUILD.md            # SC7A20H 驱动开发规格与技术目标
```

---

## ⚙️ 软件运行流程配置

在 `User/main.c` 中，您可以通过切换 `#define DEBUG_LOG_ENABLE` 宏值在两种工作模式之间切换：

### 1. 三轴数据测试模式 (`DEBUG_LOG_ENABLE = 1`)
- **工作机制**：跳过中断唤醒逻辑，直接初始化 SC7A20H 传感器，进入主循环。
- **现象**：每隔 1 秒读取并打印三轴加速度的原始数据以及对应的 mg 值，LED2 会翻转状态以指示系统正常工作。适用于验证 SPI 通信是否完全正常。

### 2. 振动唤醒主流程 (`DEBUG_LOG_ENABLE = 0`)
- **工作机制**：
  1. 初始化 `VibrationSensor` 并配置其中断触发阈值（如 `0x20U` 代表 ~1.28g @ ±2g，持续时间为 `0x01U` 即约 20ms）。
  2. 配置引脚 PA0 接收外部中断信号，检测到上升沿时置位唤醒标志，并调用 `SC7A20H_ClearInterrupt` 消除中断锁存。
  3. 系统可通过调用 `APP_EnterStopMode()` 进入 Stop 休眠模式，等待 SC7A20H 产生振动中断将 MCU 唤醒。

---

## 🛠️ 编译与开发环境

1. **Keil MDK**：建议使用 Keil MDK v5.28 或更高版本。双击打开 `MDK-ARM/Project.uvprojx` 即可直接编译、下载与调试。
2. **IAR EWARM**：使用 IAR 9.20 或更高版本。双击打开 `EWARM/Project.eww` 进行开发。
3. **调试工具**：支持 J-Link、ST-Link 或 DAP-Link。通过 SWD 接口下载程序。

---

## 📖 参考指南与调试文档

开发与联调过程中，请参阅以下详细说明：

* 📘 **SPI 调试**：有关如何解决 SPI 时钟异常（例如 26Hz）、MISO 返回 0xFF（未选通/悬空）、通信逻辑分析仪波形等，请参阅 `SPI_DEBUG_GUIDE.md`。
* 📘 **低功耗开发**：关于如何让 MCU 实现 Standby 下 4.5µA 的极限功耗，以及引脚状态配置要点，请参阅 `User/low_power/LOW_POWER_GUIDE.md`。
* 📘 **开发需求说明**：关于 SC7A20H 驱动的整体架构与实现设计原则，请参阅 `sc7a20h_BUILD.md`。
