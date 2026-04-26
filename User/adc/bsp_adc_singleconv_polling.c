/**
  ******************************************************************************
  * @file     bsp_adc_singleconv.c
  * @author   embedfire
  * @version  V1.0
  * @date     2024
  * @brief    adc转换
  ******************************************************************************
  * @attention
  *
  * 实验平台：野火 ebf_py32f030 PY32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */
#include "adc/bsp_adc_singleconv_polling.h"




ADC_HandleTypeDef             AdcHandle;
ADC_ChannelConfTypeDef        sConfig;
uint32_t adc_value[3];


/**
  * @brief  adc传感器初始化
  * @param  无
  * @note  	无
  * @retval 无
  */
void Bsp_ADC_SingleConv_Polling_Init(void)
{
    __HAL_RCC_ADC_FORCE_RESET();
    __HAL_RCC_ADC_RELEASE_RESET();                                         /* 复位ADC */
    __HAL_RCC_ADC_CLK_ENABLE();                                            /* ADC时钟使能 */
    
    AdcHandle.Instance = ADC1;
    /* ADC校准 */
    HAL_ADCEx_Calibration_Start(&AdcHandle);
    
    AdcHandle.Instance = ADC1;
    AdcHandle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;              /* 设置ADC时钟 */
    AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;                        /* 转换分辨率12bit */
    AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;                        /* 数据右对齐 */
    AdcHandle.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;              /* 扫描序列方向：向上 */
    AdcHandle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                     /* 单次采样 */
    AdcHandle.Init.LowPowerAutoWait = ENABLE;                              /* 等待转换模式开启 */
    AdcHandle.Init.ContinuousConvMode = DISABLE;                           /* 单次转换模式 */
    AdcHandle.Init.DiscontinuousConvMode = DISABLE;                        /* 不使能非连续模式 */
    AdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                  /* 软件触发 */
    AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;   /* 触发边沿无 */
    AdcHandle.Init.DMAContinuousRequests = DISABLE;                        /* 不使能DMA */
    AdcHandle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                     /* 当过载发生时，覆盖上一个值 */
    AdcHandle.Init.SamplingTimeCommon=ADC_SAMPLETIME_13CYCLES_5;           /* 设置采样周期 */
    /* 初始化ADC */
    HAL_ADC_Init(&AdcHandle);
    
    sConfig.Channel = ADC_CHANNEL_0;                                       /* ADC通道选择 */
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;                                /* 设置加入规则组通道 */
    /* 配置ADC通道 */
    HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
    
    sConfig.Channel = ADC_CHANNEL_1;                                       /* ADC通道选择 */
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;                                /* 设置加入规则组通道 */
    /* 配置ADC通道 */
    HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
    
    sConfig.Channel = ADC_CHANNEL_4;                                       /* ADC通道选择 */
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;                                /* 设置加入规则组通道 */
    /* 配置ADC通道 */
    HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
}




