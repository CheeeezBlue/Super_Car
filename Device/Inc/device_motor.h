#ifndef DEVICE_MOTOR_H
#define DEVICE_MOTOR_H

#include <stdint.h>
#include "bsp_tb6612.h"

typedef enum
{
    MG310  = 0U,
    Others = 1U,
}Motor_Typedef;

class Device_Motor_Data_Typedef
{
private:
    friend class Device_Motor_Info_Typedef;

    /*   状态   */
    float   speed_pulse;    // 脉冲速度，单位 counts/s
    float   speed_mm_s ;    // 线速度，单位 mm/s（未标定）
    float   distance_mm;    // 累计里程，单位 mm（未标定）

    /* 指令数据 */
    int16_t command;        // 电机目标指令值：-100% ~ 100%

    /* 闭环数据 */
    float   target_speed;   // 目标速度
    float   actual_speed;   // 实际速度

    /*   接口   */
public:

    void    Reset_Data(void);

};

class Device_Motor_Info_Typedef
{
private:
    Motor_Typedef             Type ;      // 电机类型枚举（后续接入其他电机的接口）
    BspTb6612Motor_t          Motor;      // 电机位置类型
    int8_t                    sign ;      // 电机方向: 1:正，-1:反
    Device_Motor_Data_Typedef Data ;      // 电机数据类

public:
    void Init(Motor_Typedef type, BspTb6612Motor_t motor, int8_t sign);// 初始化
    int8_t  Get_Motor_sign   (void);      // 获取电机的方向
    int16_t Get_Motor_command(void);      // 获取电机指令的发送值
    float   Get_Target_Speed (void);      // 获取目标速度
    float   Get_Actual_Speed (void);      // 获取实际速度

    void Set_Value(int16_t value);        // 设置参数
    void Set_Target_Speed(float value);   // 写入目标速度
    void Set_Actual_Speed(float value);   // 写入实际速度
    void Stop();                          // 停止电机
    void Reset();                         // 重置数据
    void Control_Update();                // 更新电机控制状态
    
};

#ifdef __cplusplus
extern "C" {
#endif

// void    Motor_Init(void);
// void    Motor_Set(void);
// void    Motor_Stop(void);
// int16_t DeviceMotor_GetCommand(void);

#ifdef __cplusplus
}
#endif

#endif
