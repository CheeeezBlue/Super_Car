#include "device_motor.h"

/* 宏定义 */
#define Min_Value -100
#define Max_Value  100



/**
 * @brief 通用：限幅函数
 * @retval None
 */
static int16_t value_limit(int16_t value, int16_t min_value, int16_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}


/**
* @brief  电机数据类：获取电机数据的接口实现
* @retval None
 */
void Device_Motor_Data_Typedef::Reset_Data()
{
    command = 0;
    speed_pulse  = 0.0f;
    speed_mm_s   = 0.0f;
    distance_mm  = 0.0f;
    target_speed = 0.0f;
    actual_speed = 0.0f;
}


/**
* @brief  电机类：获取电机数据的接口实现
* @retval None
 */
int16_t Device_Motor_Info_Typedef::Get_Motor_command()
{
    return Data.command;
}


/**
* @brief  电机类：获取电机数据的接口实现
* @retval None
 */
float Device_Motor_Info_Typedef::Get_Target_Speed()
{
    return Data.target_speed;
}          


/**
* @brief  电机类：获取电机数据的接口实现
* @retval None
 */
float Device_Motor_Info_Typedef::Get_Actual_Speed()
{
    return Data.actual_speed;
}


/**
* @brief  电机类：获取电机数据的接口实现
* @retval None
 */
int8_t Device_Motor_Info_Typedef::Get_Motor_sign()
{
    return sign;
}


/**
 * @brief 电机类：电机初始化接口实现
 * @retval None
 */
void Device_Motor_Info_Typedef::Init(Motor_Typedef type, BspTb6612Motor_t motor, int8_t sign)
{
    Type       = type;
    Motor      = motor;
    this->sign = sign;

    Data.Reset_Data();

    Bsp_tb6612_Init();
}

/**
 * @brief 电机类：设置电机指令接口实现
 * @retval None
 */
void Device_Motor_Info_Typedef::Set_Value(int16_t value)
{
    value = value_limit(value, Min_Value, Max_Value);

    Data.command = value;

    Bsp_tb6612_SetMotor(Motor, value * Get_Motor_sign());
}

/**
 * @brief 电机类：停止电机接口实现
 * @retval None
 */
void Device_Motor_Info_Typedef::Stop()
{
    Bsp_tb6612_StopAll();
}

/**
 * @brief 电机类：重置电机数据接口实现
 * @retval None
 */
void Device_Motor_Info_Typedef::Reset()
{
    Data.Reset_Data();
}

/**
 * @brief 电机类：更新电机控制状态接口实现
 * @retval None
 */
void Device_Motor_Info_Typedef::Control_Update()
{
}
