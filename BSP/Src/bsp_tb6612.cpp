#include "bsp_tb6612.h"

#include "car_config.h"
#include "hal_gpio.h"
#include "hal_pwm.h"

typedef struct
{
    HalGpioPin_t in1;
    HalGpioPin_t in2;
    HalPwmChannel_t pwm;
} BspTb6612MotorConfig_t;

static const BspTb6612MotorConfig_t motor_config[BSP_TB6612_MOTOR_COUNT] = {
    { { GPIOB, GPIO_PIN_12 }, { GPIOB, GPIO_PIN_13 }, HAL_PWM_CH_FL  },
    { { GPIOB, GPIO_PIN_14 }, { GPIOB, GPIO_PIN_15 }, HAL_PWM_CH_FR  },
    { { GPIOB, GPIO_PIN_0  }, { GPIOB, GPIO_PIN_1  }, HAL_PWM_CH_RL  },
    { { GPIOB, GPIO_PIN_5  }, { GPIOA, GPIO_PIN_12 }, HAL_PWM_CH_RR  }
};

static int16_t clamp_command(int16_t command)
{
    if (command > CAR_MOTOR_COMMAND_MAX)
    {
        return CAR_MOTOR_COMMAND_MAX;
    }
    if (command < -CAR_MOTOR_COMMAND_MAX)
    {
        return -CAR_MOTOR_COMMAND_MAX;
    }
    return command;
}

static uint16_t command_to_compare(int16_t command)
{
    uint32_t magnitude = (command < 0) ? (uint32_t)(-command) : (uint32_t)command;
    return (uint16_t)((magnitude * HalPwm_GetPeriod()) / CAR_MOTOR_COMMAND_MAX);
}

void Bsp_tb6612_Init(void)
{
    static uint8_t initialized = 0U;
    uint32_t i;

    /*
     * Board init runs exactly once. HalPwm_Init() starts each TIM1 channel,
     * and HAL_TIM_PWM_Start() refuses a channel that is already running
     * (returns HAL_ERROR -> Error_Handler). Device_Motor_Info_Typedef::Init()
     * calls this function once per motor, so the guard is mandatory.
     */
    if (initialized != 0U)
    {
        return;
    }
    initialized = 1U;

    HalPwm_Init();

    for (i = 0U; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        HalGpio_Write(&motor_config[i].in1, GPIO_PIN_RESET);
        HalGpio_Write(&motor_config[i].in2, GPIO_PIN_RESET);
    }
}

void Bsp_tb6612_SetMotor(BspTb6612Motor_t motor, int16_t command)
{
    const BspTb6612MotorConfig_t *config;
    int16_t effective_command;

    if (motor >= BSP_TB6612_MOTOR_COUNT)
    {
        return;
    }

    config = &motor_config[motor];
    effective_command = clamp_command(command);

    if (effective_command > 0)
    {
        HalGpio_Write(&config->in1, GPIO_PIN_SET);
        HalGpio_Write(&config->in2, GPIO_PIN_RESET);
    }
    else if (effective_command < 0)
    {
        HalGpio_Write(&config->in1, GPIO_PIN_RESET);
        HalGpio_Write(&config->in2, GPIO_PIN_SET);
    }
    else
    {
        HalGpio_Write(&config->in1, GPIO_PIN_RESET);
        HalGpio_Write(&config->in2, GPIO_PIN_RESET);
    }

    HalPwm_SetDuty(config->pwm, command_to_compare(effective_command));
}

void Bsp_tb6612_StopAll(void)
{
    uint32_t i;

    for (i = 0U; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        Bsp_tb6612_SetMotor((BspTb6612Motor_t)i, 0);
    }
}
