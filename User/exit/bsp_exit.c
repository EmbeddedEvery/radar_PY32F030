/**
  ******************************************************************************
  * @file     bsp_gpio_key.c
  * @author   embedfire
  * @version  V1.0
  * @date     2024
  * @brief    按键应用bsp（中断模式）
  ******************************************************************************
  * @attention
  *
  * 实验平台：野火 ebf_py32f002b PY32开发板 
  * 论坛      ：http://www.firebbs.cn
  * 官网      ：https://embedfire.com/
  * 淘宝      ：https://yehuosm.tmall.com/
  *
  ******************************************************************************
  */
#include "exit/bsp_exit.h"


EXTI_HandleTypeDef key1_exti_handle;
EXTI_HandleTypeDef key2_exti_handle;


/**
  * @brief  KEY初始化
  * @param  无
  * @note  	无
  * @retval 无
  */
void Bsp_EXIT_Init(void)
{
    GPIO_InitTypeDef	GPIO_Key_InitConfig;
    EXTI_ConfigTypeDef  EXTI_Key_InitConfig;
    
    /* Key GPIO时钟使能 */
    Key1_GPIO_CLK_ENABLE();                          
    Key2_GPIO_CLK_ENABLE(); 
    
    /* 配置外部中断线 */
    key1_exti_handle.Line = EXTI_LINE_5;
    /* 注册外部中断线挂起回调函数 */
    key1_exti_handle.PendingCallback = Key1_Callback;
    /* GPIO 外部中断端口选择 */
    EXTI_Key_InitConfig.GPIOSel = EXTI_GPIOA;
    /* 配置外部中断线 */
    EXTI_Key_InitConfig.Line = EXTI_LINE_5;
    /* 配置内核处理外部中断的模式（中断/事件/无） */
    EXTI_Key_InitConfig.Mode = EXTI_MODE_INTERRUPT;
    /* 配置外部中断触发边沿 */
    EXTI_Key_InitConfig.Trigger = EXTI_TRIGGER_FALLING;
    /* 配置指定的 EXTI 线路 */
    HAL_EXTI_SetConfigLine(&key1_exti_handle, &EXTI_Key_InitConfig);
    /* 配置中断优先级 */
    HAL_NVIC_SetPriority(Key1_IRQ, 1, 0);
    /* 初始化EXIT中断 */
    HAL_NVIC_EnableIRQ(Key1_IRQ);
    
    /* 配置外部中断线 */
    key2_exti_handle.Line = EXTI_LINE_6;
    /* 注册外部中断线挂起回调函数 */
    key2_exti_handle.PendingCallback = Key2_Callback;
    /* GPIO 外部中断端口选择 */
    EXTI_Key_InitConfig.GPIOSel = EXTI_GPIOA;
    /* 配置外部中断线 */
    EXTI_Key_InitConfig.Line = EXTI_LINE_6;
    /* 配置内核处理外部中断的模式（中断/事件/无） */
    EXTI_Key_InitConfig.Mode = EXTI_MODE_INTERRUPT;
    /* 配置外部中断触发边沿 */
    EXTI_Key_InitConfig.Trigger = EXTI_TRIGGER_FALLING;
    /* 配置指定的 EXTI 线路 */
    HAL_EXTI_SetConfigLine(&key2_exti_handle, &EXTI_Key_InitConfig);
    /* 配置中断优先级 */
    HAL_NVIC_SetPriority(Key2_IRQ, 1, 0);
    /* 初始化EXIT中断 */
    HAL_NVIC_EnableIRQ(Key2_IRQ);
    
    
    GPIO_Key_InitConfig.Mode = GPIO_MODE_INPUT;
    GPIO_Key_InitConfig.Pull = GPIO_PULLUP;
    GPIO_Key_InitConfig.Speed = GPIO_SPEED_FREQ_LOW;
    /* Key GPIO 初始化 */
    GPIO_Key_InitConfig.Pin = Key1_GPIO_PIN;
    HAL_GPIO_Init(Key1_GPIO_PORT, &GPIO_Key_InitConfig);
    /* Key GPIO 初始化 */
    GPIO_Key_InitConfig.Pin = Key2_GPIO_PIN;
    HAL_GPIO_Init(Key2_GPIO_PORT, &GPIO_Key_InitConfig);
}


/**
  * @brief  KEY1中断回调函数
  * @param  无
  * @note  	无
  * @retval 无
  */
void Key1_Callback(void)
{
    LED3_TOGGLE(); 
}

/**
  * @brief  KEY2中断回调函数
  * @param  无
  * @note  	无
  * @retval 无
  */
void Key2_Callback(void)
{
    LED2_TOGGLE(); 
}
