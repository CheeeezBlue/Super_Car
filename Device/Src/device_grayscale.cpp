#include "device_grayscale.h"


/**
* @brief  灰度数据类：重置数据接口实现
* @retval None
 */
void Device_Grayscale_Data_Typedef::Reset_Data()
{
    uint8_t i;

    for (i = 0U; i < CAR_GRAY_CHANNEL_COUNT; ++i)
    {
        values[i] = BSP_GRAY_WHITE;
    }

    black_count = 0U;
    error       = 0.0f;
    last_error  = 0.0f;
}


/**
* @brief  灰度类：初始化接口实现
* @retval None
 */
void Device_Grayscale_Info_Typedef::Init()
{
    Data.Reset_Data();

    Bsp_Grayscale_Init();
}


/**
* @brief  灰度类：重置数据接口实现
* @retval None
 */
void Device_Grayscale_Info_Typedef::Reset()
{
    Data.Reset_Data();
}


/**
* @brief  灰度类：更新灰度数据接口实现
* @retval None
 */
void Device_Grayscale_Info_Typedef::Info_Update()
{
    uint8_t i;
    uint8_t count = 0U;
    float   sum   = 0.0f;

    /* 读取原始数据：一次扫描 8 个通道 */
    Bsp_Grayscale_ReadAll(Data.values);

    /* 一阶数据：黑点个数 */
    for (i = 0U; i < CAR_GRAY_CHANNEL_COUNT; ++i)
    {
        if (Data.values[i] == BSP_GRAY_BLACK)
        {
            sum += (float)i;
            ++count;
        }
    }

    Data.black_count = count;

    /* 二阶数据：加权平均法求黑线重心，再减去阵列中心
     * 全白（丢线）时给 0，按直行处理；上层可以通过 Get_Black_Count 区分丢线 */
    if (count == 0U)
    {
        Data.error = 0.0f;
    }
    else
    {
        Data.error = (sum / (float)count) - ((float)(CAR_GRAY_CHANNEL_COUNT - 1U) * 0.5f);

        /* 只在"确实看到黑线"时刷新历史量：丢线后上层靠它判断该往哪边找线 */
        Data.last_error = Data.error;
    }
}


/**
* @brief  灰度类：获取灰度数据的接口实现
* @retval None
 */
uint8_t Device_Grayscale_Info_Typedef::Get_Black_Count()
{
    return Data.black_count;
}

float Device_Grayscale_Info_Typedef::Get_Error()
{
    return Data.error;
}

float Device_Grayscale_Info_Typedef::Get_Last_Error()
{
    return Data.last_error;
}
