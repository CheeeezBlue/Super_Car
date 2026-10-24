#include "hal_pwm.h"

#include "tim.h"
#include "stm32f1xx_hal.h"

static const uint32_t channel_map[HAL_PWM_CH_COUNT] = {
    TIM_CHANNEL_1,
    TIM_CHANNEL_2,
    TIM_CHANNEL_3,
    TIM_CHANNEL_4
};

void HalPwm_Init(void)
{
    uint32_t i;

    for (i = 0U; i < HAL_PWM_CH_COUNT; ++i)
    {
        if (HAL_TIM_PWM_Start(&htim1, channel_map[i]) != HAL_OK)
        {
            Error_Handler();
        }
        HalPwm_SetDuty((HalPwmChannel_t)i, 0U);
    }
}

void HalPwm_SetDuty(HalPwmChannel_t channel, uint16_t compare)
{
    uint16_t period;

    if (channel >= HAL_PWM_CH_COUNT)
    {
        return;
    }

    period = HalPwm_GetPeriod();
    if (compare > period)
    {
        compare = period;
    }

    __HAL_TIM_SET_COMPARE(&htim1, channel_map[channel], compare);
}

uint16_t HalPwm_GetPeriod(void)
{
    return (uint16_t)__HAL_TIM_GET_AUTORELOAD(&htim1);
}
