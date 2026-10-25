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

    /* 指令数据：最后一次下发的指令值，Keil Watch 窗口可直接观察 */
    int16_t command;        // 电机目标指令值：-100% ~ 100%

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

    int8_t  Get_Motor_sign(void);         // 获取电机的方向（仅供内部使用）

public:
    void Init(Motor_Typedef type, BspTb6612Motor_t motor, int8_t sign);  // 初始化

    void Set_Value(int16_t value);        // 设置参数
    void Stop();                          // 停止电机
    void Reset();                         // 重置数据

};

#endif
