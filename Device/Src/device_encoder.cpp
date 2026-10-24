#include "device_encoder.h"


/**
* @brief  编码器数据类：重置数据接口实现
* @retval None
 */
void Device_Encoder_Data_Typedef::Reset_Data()
{
    delta       =   0;
    count       =   0;
    speed_pulse = 0.f;
    speed_mm_s  = 0.f;
    distance_mm = 0.f;
}


/**
* @brief  编码器类：获取编码器数据的接口实现
* @retval None
 */
int8_t Device_Encoder_Info_Typedef::Get_Encoder_sign()
{
    return sign;
}

int16_t Device_Encoder_Info_Typedef::Get_Delta()
{
    return Data.delta;
}

int32_t Device_Encoder_Info_Typedef::Get_Count()
{
    return Data.count;
}

float Device_Encoder_Info_Typedef::Get_Speed_Pulse()
{
    return Data.speed_pulse;
}

float Device_Encoder_Info_Typedef::Get_Speed_mm_s()
{
    return Data.speed_mm_s;
}

float Device_Encoder_Info_Typedef::Get_Distance_mm()
{
    return Data.distance_mm;
}


/**
 * @brief 编码器类：编码器初始化接口实现
 * @retval None
 */
void Device_Encoder_Info_Typedef::Init(EncoderPos_t pos, int8_t sign)
{
    this->pos  = pos;
    this->sign = sign;

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
}


/**
 * @brief 编码器类：更新编码器数据接口实现
 * @retval None
 */
void Device_Encoder_Info_Typedef::Info_Update()
{
    /* 读取原始数据：按装配极性翻转，正转时增量为正 */
    Data.delta  = (int16_t)(HalEncoder_ReadDelta(pos) * sign);
    Data.count += Data.delta;

    /* 计算二阶数据 */
    Data.speed_pulse = (float)Data.delta * (1000.0f / CAR_CONTROL_PERIOD_MS);
    Data.speed_mm_s  = Data.speed_pulse * CAR_WHEEL_CIRCUMFERENCE_MM / CAR_ENCODER_CNT_PER_OUT_REV;
    Data.distance_mm += (float)Data.delta * CAR_WHEEL_CIRCUMFERENCE_MM / CAR_ENCODER_CNT_PER_OUT_REV;
}
