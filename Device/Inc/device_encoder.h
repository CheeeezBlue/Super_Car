#ifndef DEVICE_ENCODER_H
#define DEVICE_ENCODER_H

#include <stdint.h>

#include "car_config.h"
#include "hal_encoder.h"

/*
 * 编码器数据。
 *   一阶量：delta / count    —— 直接由硬件得到
 *   二阶量：speed / distance —— 由原始量 + 采样周期 + 机械常数计算得到
 */
class Device_Encoder_Data_Typedef
{
private:
    friend class Device_Encoder_Info_Typedef;   /* 允许 Info 写入 */

    /*   原始数据   */
    int16_t delta;              // 本周期脉冲增量
    int32_t count;              // 32 位累计脉冲

    /*   二阶数据   */
    float   speed_pulse;        // 脉冲速度：单位 counts/s
    float   speed_mm_s;         // 线速度：  单位 mm/s（未标定）
    float   distance_mm;        // 累计里程：单位 mm  （未标定）

    /*   接口   */
public:

    void    Reset_Data(void);

};

/* 编码器类 */
class Device_Encoder_Info_Typedef
{
private:
    EncoderPos_t                 pos;   // 车上位置
    int8_t                      sign;   // 方向极性: 1:正，-1:反
    Device_Encoder_Data_Typedef Data;   // 编码器数据

public:
    void    Init(EncoderPos_t pos, int8_t sign);   // 初始化
    int8_t  Get_Encoder_sign (void);               // 获取编码器的方向
    int16_t Get_Delta        (void);               // 获取本周期脉冲增量
    int32_t Get_Count        (void);               // 获取 32 位累计脉冲
    float   Get_Speed_Pulse  (void);               // 获取脉冲速度：单位 counts/s
    float   Get_Speed_mm_s   (void);               // 获取线速度：  单位 mm/s
    float   Get_Distance_mm  (void);               // 获取累计里程：单位 mm

    void    Reset();                               // 重置数据
    void    Info_Update();                         // 更新编码器数据

};

#endif
