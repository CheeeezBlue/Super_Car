#ifndef CAR_CONFIG_H
#define CAR_CONFIG_H

#include <stdint.h>

/* --------------------- 时间参数 --------------------- */

/* 控制周期 */
#define CAR_CONTROL_PERIOD_MS      10U

/* Debug 周期 */
#define CAR_DEBUG_PERIOD_MS        50U

/*  72 MHz / (3599 + 1) = 20 kHz. */
#define CAR_PWM_PERIOD           3599U


/* --------------------- 电机参数 --------------------- */

/* 速度环量纲基准 */
#define CAR_SPEED_PULSE_FULL_SCALE 150.0f

/* 满占空比对应的车速 mm/s */
#define CAR_MAX_SPEED_MM_S         2000.0f

/* 电机速度指令最大值 */
#define CAR_MOTOR_COMMAND_MAX     100

/* 电机方向极性 */
#define CAR_MOTOR_SIGN_FL          1
#define CAR_MOTOR_SIGN_FR          1
#define CAR_MOTOR_SIGN_RL          1
#define CAR_MOTOR_SIGN_RR          1


/* --------------------- 编码器参数 --------------------- */

/* 编码器方向极性 */
#define CAR_ENCODER_SIGN_LEFT      1
#define CAR_ENCODER_SIGN_RIGHT     1

/* 编码器与机械参数 */
#define CAR_ENCODER_PPR            13U     /* 编码器线数（每转脉冲数） */
#define CAR_ENCODER_MULTIPLIER     4U      /* TI12 模式 = 4 倍频 */
#define CAR_ENCODER_CNT_PER_MOTOR_REV \
    (CAR_ENCODER_PPR * CAR_ENCODER_MULTIPLIER)              /* 52 */

#define CAR_GEAR_RATIO             30U     /* 减速比 */
#define CAR_ENCODER_CNT_PER_OUT_REV   \
    (CAR_ENCODER_CNT_PER_MOTOR_REV * CAR_GEAR_RATIO)        /* 1560 */

#define CAR_WHEEL_DIAMETER_MM      65.0f   /* 轮径 */
#define CAR_WHEEL_CIRCUMFERENCE_MM (3.14159265f * CAR_WHEEL_DIAMETER_MM)


/* --------------------- 灰度传感器参数 --------------------- */

#define CAR_GRAY_CHANNEL_COUNT     8U      /* 通道数 */
#define CAR_GRAY_SETTLE_US         100U    /* 切换通道后的建立时间 */


/* --------------------- 灰度循迹参数 --------------------- */

/* 循迹基础速度（百分比）：左右轮的中位速度 */
#define CAR_LINE_BASE_SPEED        30

#endif
