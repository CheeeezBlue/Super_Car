#ifndef SERVICE_MOTION_H
#define SERVICE_MOTION_H

#include <stdint.h>
#include "device_encoder.h"
#include "device_motor.h"


typedef enum
{
    WHEEL_FL = 0,
    WHEEL_FR,
    WHEEL_RL,
    WHEEL_RR
}WheelPos_t;


class service_motion
{
private:
    Device_Motor_Info_Typedef   Device_Motor[4];    // 四个驱动电机
    Device_Encoder_Info_Typedef Device_Encoder[2];  // 两路编码器（左后 / 右后）
public:
    void Car_Init();
    void Car_Run(int16_t value);
    void Car_Set_Differential(int16_t left, int16_t right);
    void Car_Stop();
    void Car_Update();
    void Car_Reset();

    /* Debug */
    void Debug_Car_Get_Info(float Debug_Data[8]);
};

#ifdef __cplusplus
extern "C" { 
#endif
/* C文件接口 */

#ifdef __cplusplus
}
#endif

#endif
