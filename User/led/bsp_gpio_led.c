#include "led/bsp_gpio_led.h"
#include "py32f0xx_ll_bus.h"

void Bsp_Led_Init(void)
{
	GPIO_InitTypeDef	GPIO_Led_InitConfig;
	
	/* Enable GPIOA clock */
	LED2_GPIO_CLK_ENABLE();  
	
	GPIO_Led_InitConfig.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Led_InitConfig.Pull = GPIO_PULLUP;
	GPIO_Led_InitConfig.Speed = GPIO_SPEED_FREQ_HIGH;
	
	/* LED2 GPIO initialization */
	GPIO_Led_InitConfig.Pin = LED2_GPIO_PIN;
	HAL_GPIO_Init(LED2_GPIO_PORT, &GPIO_Led_InitConfig);
	
	/* Turn off LED2 by default */
	LED2(LED_OFF);
}

void Bsp_Led_PWM_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  /* Enable GPIOA, TIM1, and TIM3 clocks */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /* Configure PA3 (LED3) as TIM1_CH1 (AF13) */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure PA4 (LED4) as TIM3_CH3 (AF13) */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_13;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure TIM1 (for LED3) */
  TIM_InitStruct.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.CounterMode         = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Prescaler           = 24 - 1;   /* 1MHz timer clock (SYSCLK 24MHz) */
  TIM_InitStruct.Autoreload          = 1000 - 1; /* 1kHz PWM frequency */
  TIM_InitStruct.RepetitionCounter   = 0;
  LL_TIM_Init(TIM1, &TIM_InitStruct);

  /* Configure TIM3 (for LED4) */
  TIM_InitStruct.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.CounterMode         = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Prescaler           = 24 - 1;   /* 1MHz timer clock */
  TIM_InitStruct.Autoreload          = 1000 - 1; /* 1kHz PWM frequency */
  TIM_InitStruct.RepetitionCounter   = 0;
  LL_TIM_Init(TIM3, &TIM_InitStruct);

  /* Configure PWM Channel for TIM1_CH1 (Active LOW since LEDs are active-low) */
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_LOW;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_HIGH;
  TIM_OC_InitStruct.CompareValue = 0; /* Default 0% brightness (Off) */
  LL_TIM_OC_Init(TIM1, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);

  /* Configure PWM Channel for TIM3_CH3 */
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_PWM1;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_LOW;
  TIM_OC_InitStruct.OCIdleState = LL_TIM_OCIDLESTATE_HIGH;
  TIM_OC_InitStruct.CompareValue = 0; /* Default 0% brightness (Off) */
  LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH3, &TIM_OC_InitStruct);

  /* Enable TIM1 Outputs and Counter */
  LL_TIM_EnableAllOutputs(TIM1);
  LL_TIM_EnableCounter(TIM1);
  
  /* Enable TIM3 Counter */
  LL_TIM_EnableCounter(TIM3);
}

void Bsp_Led_PWM_SetBrightness(uint8_t led_num, uint16_t brightness)
{
  /* Limit brightness to maximum Auto-reload value (1000) */
  if (brightness > 1000)
  {
    brightness = 1000;
  }

  if (led_num == 3)
  {
    LL_TIM_OC_SetCompareCH1(TIM1, brightness);
  }
  else if (led_num == 4)
  {
    LL_TIM_OC_SetCompareCH3(TIM3, brightness);
  }
}
