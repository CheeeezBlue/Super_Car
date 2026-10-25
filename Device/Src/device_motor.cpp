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
* @brief  电机数据类：重置数据接口实现
* @retval None
 */
void Device_Motor_Data_Typedef::Reset_Data()
{
    command = 0;
}


/**
* @brief  电机类：获取电机方向接口实现
* @retval 电机方向极性
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
    Bsp_tb6612_Stop(Motor);
}

/**
 * @brief 电机类：重置电机数据接口实现
 * @retval None
 */
void Device_Motor_Info_Typedef::Reset()
{
    Data.Reset_Data();
}
