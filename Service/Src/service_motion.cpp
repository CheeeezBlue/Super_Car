#include "service_motion.h"


/* 四轮装配表：电机型号 / 电机方位 / 方向极性 */
static const struct
{
    Motor_Typedef    type;
    BspTb6612Motor_t pos;
    int8_t           sign;
} Motor_Cfg[BSP_TB6612_MOTOR_COUNT] = {
    { MG310, BSP_TB6612_FL, CAR_MOTOR_SIGN_FL },
    { MG310, BSP_TB6612_FR, CAR_MOTOR_SIGN_FR },
    { MG310, BSP_TB6612_RL, CAR_MOTOR_SIGN_RL },
    { MG310, BSP_TB6612_RR, CAR_MOTOR_SIGN_RR }
};

static const struct 
{
    EncoderPos_t  pos;
    int8_t       sign;
} Encoder_Cfg[ENCODER_COUNT] = {
    {ENCODER_LEFT ,CAR_ENCODER_SIGN_LEFT },
    {ENCODER_RIGHT,CAR_ENCODER_SIGN_RIGHT}
};


/**
 * @brief 电机服务类：初始化接口实现
 */
void service_motion::Car_Init()
{
    for(int i = 0; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        Device_Motor[i].Init(Motor_Cfg[i].type, Motor_Cfg[i].pos, Motor_Cfg[i].sign);
    }
    for(int i = 0; i < ENCODER_COUNT; ++i)
    {
        Device_Encoder[i].Init(Encoder_Cfg[i].pos, Encoder_Cfg[i].sign);
    }
}


/**
 * @brief 电机服务类：运行接口实现
 * @param value: 电机控制命令值 -100% ~ 100%
 */
void service_motion::Car_Run(int16_t value)
{
    for(int i = 0; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        Device_Motor[i].Set_Value(value);
    }
}


/**
 * @brief 电机服务类：左右差速接口实现
 * @param left:  左侧（FL / RL）指令 -100% ~ 100%
 * @param right: 右侧（FR / RR）指令 -100% ~ 100%
 */
void service_motion::Car_Set_Differential(int16_t left, int16_t right)
{
    Device_Motor[WHEEL_FL].Set_Value(left);
    Device_Motor[WHEEL_FR].Set_Value(right);
    Device_Motor[WHEEL_RL].Set_Value(left);
    Device_Motor[WHEEL_RR].Set_Value(right);
}


/**
 * @brief 电机服务类：电机停止接口实现
 */
void service_motion::Car_Stop()
{
    for(int i = 0; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        Device_Motor[i].Stop();
    }
    Car_Reset();
}


/**
 * @brief 电机服务类：电机重置数据接口实现
 */
void service_motion::Car_Reset()
{
    for(int i = 0; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        Device_Motor[i].Reset();
    }

    for(int i = 0; i < ENCODER_COUNT; ++i)
    {
        Device_Encoder[i].Reset();
    }
}


/**
 * @brief 电机服务类：电机数据更新接口实现
 */
void service_motion::Car_Update()
{
    for(int i = 0; i < ENCODER_COUNT; ++i)
    {
        Device_Encoder[i].Info_Update();
    }
}


/**
 * @brief 电机服务类：电机数据获取调试接口实现
 */
void service_motion::Debug_Car_Get_Info(float Debug_Data[8])
{
    Debug_Data[0] = Device_Encoder[0].Get_Speed_Pulse();
    Debug_Data[1] = Device_Encoder[1].Get_Speed_Pulse();
    Debug_Data[2] = Device_Encoder[0].Get_Speed_mm_s();
    Debug_Data[3] = Device_Encoder[1].Get_Speed_mm_s();
    Debug_Data[4] = Device_Encoder[0].Get_Delta();
    Debug_Data[5] = Device_Encoder[1].Get_Delta();
    Debug_Data[6] = Device_Encoder[0].Get_Count();
    Debug_Data[7] = Device_Encoder[1].Get_Count();
}

