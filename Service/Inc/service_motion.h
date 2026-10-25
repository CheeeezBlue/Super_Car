#ifndef SERVICE_MOTION_H
#define SERVICE_MOTION_H

#include <stdint.h>
#include "device_encoder.h"
#include "device_motor.h"
#include "device_grayscale.h"


typedef enum
{
    WHEEL_FL = 0,
    WHEEL_FR,
    WHEEL_RL,
    WHEEL_RR
}WheelPos_t;


typedef enum
{
	MODE_IDLE = 0U,   	/* 静止模式 */
	MODE_MANUAL	  ,     /* 闭环直线 */
	MODE_STRAIGHT ,    	/* 直走定距 - 暂不开发 - 静止处理 */
	MODE_LINE	  ,     /* 灰度循迹 */
	MODE_COUNT	  
}Running_Mode_Type;


class service_motion
{
private:
    Device_Motor_Info_Typedef   Device_Motor[4];    // 四个驱动电机
    Device_Encoder_Info_Typedef Device_Encoder[2];  // 两路编码器（左后 / 右后）
    Device_Grayscale_Info_Typedef Device_Grayscale;

    void Car_Reset();
    void Car_Stop();
    void Car_Send_Velocity();
    void Car_Out(int16_t Left_Target, int16_t Right_Target);
    void Car_Mode_Update(Running_Mode_Type Mode, int16_t base_speed);

public:
    /* APP 控制接口 */
    void Car_Init();
    void Car_Info_Update();
    void Car_Control(Running_Mode_Type Mode, int16_t base_speed);

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
