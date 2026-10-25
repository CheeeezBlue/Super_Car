#ifndef DEVICE_ENCODER_H
#define DEVICE_ENCODER_H

#include <stdint.h>

#include "car_config.h"
#include "hal_encoder.h"

/*
 * 编码器数据。
 *   delta       本周期脉冲增量（硬件直接得到）
 *   speed_pulse 脉冲速度 = delta × (1000 / 控制周期)
 */
class Device_Encoder_Data_Typedef
{
private:
    friend class Device_Encoder_Info_Typedef;   /* 允许 Info 写入 */

    /*   原始数据   */
    int16_t delta;              // 本周期脉冲增量

    /*   二阶数据   */
    float   speed_pulse;        // 脉冲速度：单位 counts/s

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
    uint8_t                     primed; // 0:首个采样待丢弃  1:已进入正常周期采样
    Device_Encoder_Data_Typedef Data;   // 编码器数据

public:
    void    Init(EncoderPos_t pos, int8_t sign);   // 初始化
    int16_t Get_Delta        (void);               // 获取本周期脉冲增量
    float   Get_Speed_Pulse  (void);               // 获取脉冲速度：单位 counts/s

    void    Reset();                               // 重置数据
    void    Info_Update();                         // 更新编码器数据

};

#endif
