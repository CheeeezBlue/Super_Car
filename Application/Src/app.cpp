#include "app.h"

#include "car_config.h"
#include "service_motion.h"
#include "hal_time.h"
#include "hal_uart.h"

static service_motion motion;

/* 调试变量 */
float Debug_Data[8] = {0};

void App_Init(void)
{
    motion.Car_Init();
    HalUart_DebugPrintf("USART Ready\r\n");
}


void App_Run(void)
{
    static uint32_t last_tick = 0U;
    int16_t test_value = 20;

    /* 固定 10 ms 控制周期：编码器差分与速度换算都依赖这个周期 */
    if ((HalTime_GetMs() - last_tick) >= CAR_CONTROL_PERIOD_MS)
    {
        last_tick += CAR_CONTROL_PERIOD_MS;

        motion.Car_Run(test_value);
        motion.Car_Update();
    }
}


void App_SetDifferential(int16_t left, int16_t right)
{
    // (void)left;
    // (void)right;
}


void App_Stop(void)
{
    motion.Car_Stop();
}


void App_Debug(void)
{
    motion.Debug_Car_Get_Info(Debug_Data);
    USART2_Vofa_Justfloat_Transmit( Debug_Data[0], Debug_Data[1],
                                    Debug_Data[2], Debug_Data[3], 
                                    Debug_Data[4], Debug_Data[5],
                                    Debug_Data[6], Debug_Data[7]);
}

