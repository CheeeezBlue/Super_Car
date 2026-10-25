#include "service_motion.h"
#include "PID.h"

/* 速度环 PID 实例：左轮 / 右轮 */
PID_Info_TypeDef Left_Wheel_Velocity_PID;
PID_Info_TypeDef Right_Wheel_Velocity_PID;

/* 灰度循迹外环 PID 实例：一路误差 -> 一路差速，左右共用 */
PID_Info_TypeDef Line_Follow_PID;

/* 小车控制模式 */
static Running_Mode_Type Ser_Mode = MODE_IDLE;

/* 左右轮目标速度 */
static int16_t L_Speed_Target = 0;
static int16_t R_Speed_Target = 0;

/* 灰度快照：Car_Info_Update 采样后写入，供外环与调试使用 */
static float   Ser_Gray_Error      = 0.0f;
static uint8_t Ser_Gray_Count      = 0U;
static float   Ser_Gray_Last_Error = 0.0f;

/* 外环输出（纠偏量），供调试通道观察 */
static float   Ser_Line_Steer      = 0.0f;

/* PID 参数表（速度环与循迹环共用同一结构） */
typedef struct
{
    float KP;                       // 比例系数
    float KI;                       // 积分系数
    float KD;                       // 微分系数
    float Alpha;                    // 微分一阶滤波器系数
    float Deadband;                 // 死区：误差绝对值小于该值时停止计算
    float LimitIntegral;            // 积分限幅
    float LimitOutput;              // 总输出限幅
    float CoefA;                    // 变速积分全积分阈值 A
    float CoefB;                    // 变速积分最大阈值 B，>0 时启用
    float IntegralSeparation;       // 积分分离阈值，>0 时启用
    float K_F;                      // 前馈系数
    PID_Improvement_e Improvement;  // PID 改进方式
} PID_Cfg_Typedef;

/* 速度环参数 索引：0 左轮 / 1 右轮 */
static const PID_Cfg_Typedef Speed_PID_Cfg[2] = {
        /* Left */
    {
        3.1f,   // KP
        0.3f,   // KI
        0.5f,   // KD
        0.2f,   // Alpha
        0.0f,   // Deadband
        300.0f, // LimitIntegral
        100.0f, // LimitOutput
        0.0f,   // CoefA
        0.0f,   // CoefB
        0.0f,   // IntegralSeparation
        0.0f,   // K_F
        PID_Improvement_NONE // Improvement
    }, 
        /* Right */
    {
        3.1f,   // KP
        0.3f,   // KI
        0.5f,   // KD
        0.2f,   // Alpha
        0.0f,   // Deadband
        300.0f, // LimitIntegral
        100.0f, // LimitOutput
        0.0f,   // CoefA
        0.0f,   // CoefB
        0.0f,   // IntegralSeparation
        0.0f,   // K_F
        PID_Improvement_NONE // Improvement
    }
};

/* 灰度循迹外环参数 */
static const PID_Cfg_Typedef Line_PID_Cfg = {
    3.0f,       // KP            
    0.0f,       // KI            
    0.0f,       // KD            
    0.1f,       // Alpha         
    0.0f,       // Deadband      
    100.0f,     // LimitIntegral 
    100.0f,     // LimitOutput   
    0.0f,       // CoefA         
    0.0f,       // CoefB         
    0.0f,       // IntegralSeparation
    0.0f,       // K_F           
    PID_Improvement_NONE // Improvement
};


/**
 * @brief PID 参数装载（速度环 / 循迹环共用）
 * @param pid: 目标 PID 实例
 * @param cfg: 参数表条目
 */
static void PID_Load(PID_Info_TypeDef *pid, const PID_Cfg_Typedef *cfg)
{
    pid->Type = PID_POSITION;

    pid->Param.KP                 = cfg->KP;
    pid->Param.KI                 = cfg->KI;
    pid->Param.KD                 = cfg->KD;
    pid->Param.Alpha              = cfg->Alpha;
    pid->Param.Deadband           = cfg->Deadband;
    pid->Param.LimitIntegral      = cfg->LimitIntegral;
    pid->Param.LimitOutput        = cfg->LimitOutput;
    pid->Param.CoefA              = cfg->CoefA;
    pid->Param.CoefB              = cfg->CoefB;
    pid->Param.IntegralSeparation = cfg->IntegralSeparation;
    pid->Param.K_F                = cfg->K_F;
    pid->Param.Improvement        = cfg->Improvement;
}


/**
 * @brief 清空 PID 控制器的全部运行状态
 * @param pid: PID 实例
 */
static void PID_Clear(PID_Info_TypeDef *pid)
{
    pid->PID_Calc_Clear(pid);

    pid->Target       = 0.0f;
    pid->Last_Target  = 0.0f;
    pid->Measure      = 0.0f;
    pid->Last_Measure = 0.0f;
    pid->Dout_LPF.Input       = 0.0f;
    pid->Dout_LPF.Output      = 0.0f;
    pid->Dout_LPF.Initialized = false;
}


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
 * @brief 百分比 -> 每控制周期脉冲数
 * @param percent: 目标速度百分比
 */
static float Target_To_Pulse(int16_t percent)
{
    return (float)value_limit(percent, -CAR_MOTOR_COMMAND_MAX, CAR_MOTOR_COMMAND_MAX) * CAR_SPEED_PULSE_FULL_SCALE /
           (float)CAR_MOTOR_COMMAND_MAX;
}


/* ----------------------- 计算目标速度函数 ----------------------- */
/**
 * @brief  直线行驶
 * @param  base_speed:基础速度
 * @retval None
 */
static void Car_Manual_Calu(int16_t base_speed)
{
    /* 四轮同速 */
    L_Speed_Target = base_speed;
    R_Speed_Target = base_speed;
}


/**
 * @brief  巡线行驶：只给中位速度
 * @param  base_speed:基础速度
 * @retval None
 * @note   左右差速由 Car_Out 里的灰度外环（Line_Follow_PID）修正，
 *         这里不再直接算差速，避免内外环重复计算。
 */
static void Car_Line_Calu(int16_t base_speed)
{
    L_Speed_Target = base_speed;
    R_Speed_Target = base_speed;
}


/**
 * @brief  直线定距离行驶
 * @param  base_speed:基础速度
 * @param  Distance:  行驶距离
 * @retval None
 */
// static void Car_Straight_Calu(int16_t base_speed, int16_t Distance)
// {
//     L_Speed_Target = base_speed;
//     R_Speed_Target = base_speed;
    
//     /* 定距离行驶暂未开发 */
//     // pass 
//     // ...

// }


/* ----------------------- 核心控制实现 ----------------------- */

/**
 * @brief 电机服务类：初始化接口实现
 * @retval None
 */
void service_motion::Car_Init(void)
{
    for(int i = 0; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        Device_Motor[i].Init(Motor_Cfg[i].type, Motor_Cfg[i].pos, Motor_Cfg[i].sign);
    }
    for(int i = 0; i < ENCODER_COUNT; ++i)
    {
        Device_Encoder[i].Init(Encoder_Cfg[i].pos, Encoder_Cfg[i].sign);
    }
    /* 装载 PID 结构体参数 */
    PID_Load(&Left_Wheel_Velocity_PID,  &Speed_PID_Cfg[0]);
    PID_Load(&Right_Wheel_Velocity_PID, &Speed_PID_Cfg[1]);
    PID_Load(&Line_Follow_PID,          &Line_PID_Cfg);

    /* PID 结构体初始化 */
    PID_Init( &Left_Wheel_Velocity_PID);
    PID_Init(&Right_Wheel_Velocity_PID);
    PID_Init(&Line_Follow_PID);
}


/**
 * @brief  服务层：模式调度函数，调度控制模式
 * @param  Mode:控制模式
 * @retval None
 * @note   None
 */
void service_motion::Car_Mode_Update(Running_Mode_Type Mode, int16_t base_speed)
{
    /* 更新 Service Mode 状态 */
     uint8_t mode_changed_flag = (Ser_Mode != Mode) ? 1U : 0U;

     if (mode_changed_flag != 0U)
     {
         Car_Stop();
     }
     Ser_Mode = Mode;
     
    /* 根据模式控制 */
    switch(Ser_Mode)
    {
        /* 静止模式 */
        case MODE_IDLE:
            L_Speed_Target = 0;
            R_Speed_Target = 0;
            break;

        /* 闭环直线 */
        case MODE_MANUAL:
            Car_Manual_Calu(base_speed);
            break;

        /* 直走定距 */ 
        case MODE_STRAIGHT:
            L_Speed_Target = 0;
            R_Speed_Target = 0;
            break;

        /* 灰度循迹 */
        case MODE_LINE:
            Car_Line_Calu(base_speed);
            break;
        default:
            L_Speed_Target = 0;
            R_Speed_Target = 0;
            break;
    }
}


/**
 * @brief 电机服务类：电机数据更新接口实现
 */
void service_motion::Car_Info_Update()
{
    /* 更新两路编码器数据 */
    for(int i = 0; i < ENCODER_COUNT; ++i)
    {
        Device_Encoder[i].Info_Update();
    }

    /* 更新灰度传感器数据（只有循迹模式才需要，省下 8 通道 x 100us 的扫描时间） */
    if(Ser_Mode == MODE_LINE)
    {
        Device_Grayscale.Info_Update();

        Ser_Gray_Error      = Device_Grayscale.Get_Error();
        Ser_Gray_Count      = Device_Grayscale.Get_Black_Count();
        Ser_Gray_Last_Error = Device_Grayscale.Get_Last_Error();
    }
}


/**
 * @brief 电机服务类：外环 + 内环 计算接口实现
 * @param Left_Target:  左侧目标速度 -100% ~ 100%
 * @param Right_Target: 右侧目标速度 -100% ~ 100%
 */
void service_motion::Car_Out(int16_t Left_Target, int16_t Right_Target)
{
    /* --------------------- 外环：灰度循迹 --------------------- */
    if (Ser_Mode == MODE_LINE)
    {
        float gray_error;
        float steer;
        float limit;

        gray_error = (Ser_Gray_Count == 0U) ? Ser_Gray_Last_Error : Ser_Gray_Error;

        steer = PID_Calculate(&Line_Follow_PID, 0.0f, -gray_error);

        limit = (Left_Target >= 0) ? (float)Left_Target : (float)(-Left_Target);
        if (steer >  limit) { steer =  limit; }
        if (steer < -limit) { steer = -limit; }

        Ser_Line_Steer = steer;

        Left_Target  = (int16_t)((float)Left_Target  + steer);
        Right_Target = (int16_t)((float)Right_Target - steer);
    }
    else
    {
        Ser_Line_Steer = 0.0f;
    }

    /* --------------------- 内环：速度环 --------------------- */
    PID_Calculate(&Left_Wheel_Velocity_PID,
                  Target_To_Pulse(Left_Target),
                  -(float)Device_Encoder[0].Get_Delta());

    PID_Calculate(&Right_Wheel_Velocity_PID,
                  Target_To_Pulse(Right_Target),
                  -(float)Device_Encoder[1].Get_Delta());
}

/**
 * @brief 电机服务类：发送输出值接口实现
 */
void service_motion::Car_Send_Velocity()
{
    Device_Motor[WHEEL_FL].Set_Value((int16_t)Left_Wheel_Velocity_PID.Output);
    Device_Motor[WHEEL_FR].Set_Value((int16_t)Right_Wheel_Velocity_PID.Output);
    Device_Motor[WHEEL_RL].Set_Value((int16_t)Left_Wheel_Velocity_PID.Output);
    Device_Motor[WHEEL_RR].Set_Value((int16_t)Right_Wheel_Velocity_PID.Output);
}


/**
 * @brief  服务类：控制接口实现
 * @param  Mode: 控制模式
 * @param  base_speed:基础速度
 * @retval None
 */
void service_motion::Car_Control(Running_Mode_Type Mode, int16_t base_speed)
{
    /* 输入模式值检查 */
    if(Mode >= MODE_COUNT) return;

    base_speed = value_limit(base_speed, -CAR_MOTOR_COMMAND_MAX, CAR_MOTOR_COMMAND_MAX);

    /* 控制模式调度 */
    Car_Mode_Update(Mode, base_speed);

    /* 计算输出值 */
    Car_Out(L_Speed_Target,R_Speed_Target);

    /* 发送速度值 */
    Car_Send_Velocity();
}



/**
 * @brief 电机服务类：电机停止接口实现
 */
void service_motion::Car_Stop()
{
    L_Speed_Target = 0;
    R_Speed_Target = 0;

    for(int i = 0; i < BSP_TB6612_MOTOR_COUNT; ++i)
    {
        Device_Motor[i].Stop();
    }

    /* 清除 PID 状态 */
    PID_Clear(&Left_Wheel_Velocity_PID);
    PID_Clear(&Right_Wheel_Velocity_PID);
    PID_Clear(&Line_Follow_PID);

    Car_Reset();
}


/**
 * @brief 电机服务类：电机重置数据接口实现
 */
void service_motion::Car_Reset()
{
    L_Speed_Target = 0;
    R_Speed_Target = 0;
    
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
 * @brief 电机服务类：电机数据获取调试接口实现
 * @param Debug_Data[8]:打印的数据数组 - 上限 8 通道
 * @note  通道映射：
 *        [0][1] 左右目标速度  [2][3] 左右脉冲速度
 *        [4]    灰度误差      [5]    外环纠偏量
 *        [6][7] 左右实际脉冲增量
 */
void service_motion::Debug_Car_Get_Info(float Debug_Data[8])
{
    Debug_Data[0] = L_Speed_Target;
    Debug_Data[1] = R_Speed_Target;
    Debug_Data[2] = Device_Encoder[0].Get_Speed_Pulse();
    Debug_Data[3] = Device_Encoder[1].Get_Speed_Pulse();
    Debug_Data[4] = Ser_Gray_Error;
    Debug_Data[5] = Ser_Line_Steer;
    Debug_Data[6] = Device_Encoder[0].Get_Delta();
    Debug_Data[7] = Device_Encoder[1].Get_Delta();
}
