#ifndef BSP_TB6612_H
#define BSP_TB6612_H

#include <stdint.h>
#include "car_config.h"
#include "hal_gpio.h"
#include "hal_pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    BSP_TB6612_FL = 0,
    BSP_TB6612_FR,
    BSP_TB6612_RL,
    BSP_TB6612_RR,
    BSP_TB6612_MOTOR_COUNT
} BspTb6612Motor_t;


void Bsp_tb6612_Init(void);
void Bsp_tb6612_SetMotor(BspTb6612Motor_t motor, int16_t command);
void Bsp_tb6612_Stop(BspTb6612Motor_t motor);

#ifdef __cplusplus
}
#endif

#endif
