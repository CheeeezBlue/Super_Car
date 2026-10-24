#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    HAL_PWM_CH_FL = 0,
    HAL_PWM_CH_FR,
    HAL_PWM_CH_RL,
    HAL_PWM_CH_RR,
    HAL_PWM_CH_COUNT
} HalPwmChannel_t;

void HalPwm_Init(void);
void HalPwm_SetDuty(HalPwmChannel_t channel, uint16_t compare);
uint16_t HalPwm_GetPeriod(void);

#ifdef __cplusplus
}
#endif

#endif
