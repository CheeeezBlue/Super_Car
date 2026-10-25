#ifndef HAL_TIME_H
#define HAL_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t HalTime_GetMs(void);

/**
 * @brief 微秒级忙等延时
 * @param us: 延时长度，单位 us
 * @note  基于 DWT 周期计数器，首次调用时自动使能，不占用定时器。
 *        等待期间被中断打断会略微拉长实际延时，对传感器建立时间无影响。
 */
void     HalTime_DelayUs(uint16_t us);

#ifdef __cplusplus
}
#endif

#endif
