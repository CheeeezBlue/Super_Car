#ifndef DEVICE_GRAYSCALE_H
#define DEVICE_GRAYSCALE_H

#include <stdint.h>

#include "bsp_grayscale.h"
#include "car_config.h"

/*
 * 灰度数据：
 *   原始量：values[]    每通道 0 = 黑线，1 = 白底
 *   一阶量：black_count 黑点个数
 *   二阶量：error       黑线重心的偏移量，-3.5(最左) ~ +3.5(最右)
 *   历史量：last_error  上一次"检测到黑线"时的误差，供上层丢线处理
 */
class Device_Grayscale_Data_Typedef
{
private:
    friend class Device_Grayscale_Info_Typedef;   /* 允许 Info 写入 */

    uint8_t values[CAR_GRAY_CHANNEL_COUNT];       // 每通道电平
    uint8_t black_count;                          // 黑点个数
    float   error;                                // 重心偏移：负=偏左，正=偏右
    float   last_error;                           // 上一次有效（有黑线）的重心偏移

    /*   接口   */
public:

    void    Reset_Data(void);

};

/* 灰度传感器类 */
class Device_Grayscale_Info_Typedef
{
private:
    Device_Grayscale_Data_Typedef Data;   // 灰度数据

public:
    void    Init(void);                          // 初始化
    void    Info_Update(void);                   // 采样并更新数据
    void    Reset(void);                         // 重置数据

    uint8_t Get_Black_Count(void);               // 获取黑点个数
    float   Get_Error(void);                     // 获取重心偏移
    float   Get_Last_Error(void);                // 获取上一次有效的重心偏移

};

#endif
