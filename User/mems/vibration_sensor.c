#include "mems/vibration_sensor.h"
#include "debug_log.h"
#include "mems/sc7a20h.h"

static EXTI_HandleTypeDef vibration_exti_handle;
static volatile uint8_t vibration_wake_flag = 0U;

static void VibrationSensor_ExtiCallback(void);

HAL_StatusTypeDef VibrationSensor_Init(void)
{
    GPIO_InitTypeDef gpio_init = {0};
    EXTI_ConfigTypeDef exti_config = {0};
    HAL_StatusTypeDef status;

    SC7A20H_WakeupConfig wakeup_config = {
        .threshold = 0x10U,
        .duration = 0x00U,
    };

    VIBRATION_INT_GPIO_CLK_ENABLE();

    gpio_init.Pin = VIBRATION_INT_PIN;
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(VIBRATION_INT_PORT, &gpio_init);

    vibration_exti_handle.Line = VIBRATION_INT_EXTI_LINE;
    vibration_exti_handle.PendingCallback = VibrationSensor_ExtiCallback;

    exti_config.Line = VIBRATION_INT_EXTI_LINE;
    exti_config.GPIOSel = VIBRATION_INT_EXTI_GPIOSEL;
    exti_config.Mode = EXTI_MODE_INTERRUPT;
    exti_config.Trigger = EXTI_TRIGGER_RISING;
    HAL_EXTI_SetConfigLine(&vibration_exti_handle, &exti_config);

    HAL_NVIC_SetPriority(VIBRATION_INT_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(VIBRATION_INT_IRQn);

    status = SC7A20H_Init(&wakeup_config);
    if (status == HAL_OK)
    {
        APP_LOG("Vibration sensor init ok");
    }
    else
    {
        APP_LOG("Vibration sensor init failed");
    }

    return status;
}

void VibrationSensor_IrqHandler(void)
{
    HAL_EXTI_IRQHandler(&vibration_exti_handle);
}

uint8_t VibrationSensor_HasWakeEvent(void)
{
    return vibration_wake_flag;
}

void VibrationSensor_ClearWakeEvent(void)
{
    vibration_wake_flag = 0U;
}

static void VibrationSensor_ExtiCallback(void)
{
    vibration_wake_flag = 1U;
    (void)SC7A20H_ClearInterrupt();
}
