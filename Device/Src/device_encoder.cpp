#include "device_encoder.h"


/**
* @brief  编码器数据类：重置数据接口实现
* @retval None
 */
void Device_Encoder_Data_Typedef::Reset_Data()
{
    delta       = 0;
    speed_pulse = 0.f;
}


/**
* @brief  编码器类：获取编码器数据的接口实现
* @retval None
 */
int16_t Device_Encoder_Info_Typedef::Get_Delta()
{
    return Data.delta;
}

float Device_Encoder_Info_Typedef::Get_Speed_Pulse()
{
    return Data.speed_pulse;
}


/**
 * @brief 编码器类：编码器初始化接口实现
 * @retval None
 */
void Device_Encoder_Info_Typedef::Init(EncoderPos_t pos, int8_t sign)
{
    this->pos    = pos;
    this->sign   = sign;
    this->primed = 0U;

    Data.Reset_Data();

    HalEncoder_Init();
}


/**
 * @brief 编码器类：重置编码器数据接口实现
 * @retval None
 */
void Device_Encoder_Info_Typedef::Reset()
{
    HalEncoder_Reset(pos);

    Data.Reset_Data();

    primed = 0U;    /* 计数器已清零，下一次采样同样是"跨越未知时长"，需要丢弃 */
}


/**
 * @brief 编码器类：更新编码器数据接口实现
 * @retval None
 */
void Device_Encoder_Info_Typedef::Info_Update()
{
    /*
     * 首拍丢弃：从 HalEncoder_Init/Reset 清零到第一个控制周期之间，
     * 间隔并不是一个 CAR_CONTROL_PERIOD_MS（中间还夹着 App_Init、串口输出等），
     * 这段增量直接当速度用会明显偏大。这里只读一次刷新 HAL 内部基准。
     */
    if (primed == 0U)
    {
        (void)HalEncoder_ReadDelta(pos);

        Data.Reset_Data();

        primed = 1U;
        return;
    }

    /* 读取原始数据：按装配极性翻转，正转时增量为正 */
    Data.delta = (int16_t)(HalEncoder_ReadDelta(pos) * sign);

    /* 二阶数据：脉冲速度，单位 counts/s */
    Data.speed_pulse = (float)Data.delta * (1000.0f / CAR_CONTROL_PERIOD_MS);
}
