#include "app.h"
#include "bsp_usart.h"
#include "car_config.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "service_motion.h"

static service_motion    motion;
static Running_Mode_Type App_Mode;

/* 调试变量 */
float Debug_Data[8] = {0};

/* 基础速度，可由串口命令修改 */
static int16_t base_speed = 0;


static void App_Query(void);
static void App_CmdHandle(const char *line);
static void App_CmdPoll(void);


/**
 * @brief 应用层：小车初始化函数
 * @retval None
 */
void App_Init(void)
{
    motion.Car_Init();
    Bsp_Usart_Init();
    App_Mode = MODE_IDLE;

    HalUart_DebugPrintf("USART Ready\r\n");
}

/**
 * @brief 应用层：Debug 函数，用于调试
 * @retval None
 */
void App_Debug(void)
{
    motion.Debug_Car_Get_Info(Debug_Data);
    Bsp_Usart_Vofa_Transmit8( Debug_Data[0], Debug_Data[1],
                              Debug_Data[2], Debug_Data[3],
                              Debug_Data[4], Debug_Data[5],
                              Debug_Data[6], Debug_Data[7]);
}


/**
 * @brief 应用层：RUN —— 控制唯一入口
 * @retval None
 */
void App_Run(void)
{
    static uint32_t last_ctrl  = 0U;   /* 10 ms 控制周期 */
    static uint32_t last_debug = 0U;   /* 50 ms 调试周期 */

    uint32_t now = HalTime_GetMs();

    /* 命令轮询 */
    App_CmdPoll();

    /* 10 ms 控制周期：先采样 -> PID 控制器 -> 输出 PWM */
    if ((now - last_ctrl) >= CAR_CONTROL_PERIOD_MS)
    {
        last_ctrl += CAR_CONTROL_PERIOD_MS;
        
        motion.Car_Info_Update();
        motion.Car_Control(App_Mode, base_speed);
    }

    /* 50 ms VOFA+ 串口发送周期 */
    if ((now - last_debug) >= CAR_DEBUG_PERIOD_MS)
    {
        last_debug += CAR_DEBUG_PERIOD_MS;

        App_Debug();
    }
}


/* ----------------------- 串口中间函数 ----------------------- */

/**
 * @brief 串口命令轮询：取一整行并分发
 * @retval None
 */
static void App_CmdPoll(void)
{
    char line[BSP_USART_LINE_SIZE];

    if (Bsp_Usart_GetLine(line, (uint16_t)sizeof(line)) != 0U)
    {
        App_CmdHandle(line);
    }
}


/**
 * @brief 命令分发
 * @param line
 * @retval None
 */
static void App_CmdHandle(const char *line)
{
    BspUsartCmd_t cmd;

    Bsp_Usart_ParseCmd(line, &cmd);

    switch (cmd.type)
    {
    case USART_CMD_SPD:
        /* 只设置目标速度，不改变运行模式：
         * 静止模式下收到 SPD 只记下速度，车不动；起步由 MD 命令决定 */
        base_speed = (int16_t)cmd.value;
        HalUart_DebugPrintf("SPD:%d\r\n", (int)base_speed);
        break;

    case USART_CMD_MODE:
        if ((cmd.value >= 0) && (cmd.value < (int16_t)MODE_COUNT))
        {
            App_Mode = (Running_Mode_Type)cmd.value;
            HalUart_DebugPrintf("MODE:%d\r\n", (int)cmd.value);
        }
        else
        {
            HalUart_DebugPrintf("ERR:MODE\r\n");
        }
        break;

    case USART_CMD_STOP:
        App_Mode = MODE_IDLE;

        HalUart_DebugPrintf("STOP\r\n");
        break;

    case USART_CMD_QUERY:
        App_Query();
        break;

    default:
        HalUart_DebugPrintf("ERR:%s\r\n", line);
        break;
    }
}


/**
 * @brief 状态查询：指令 / 编码器增量 / 脉冲速度
 * @retval None
 * @note  Debug_Car_Get_Info 的通道映射：
 *        [0][1] speed_pulse 左/右   [4][5] delta 左/右
 */
static void App_Query(void)
{
    float debug[8];

    motion.Debug_Car_Get_Info(debug);

    HalUart_DebugPrintf("L:%d R:%d ENC:%d,%d SPD:%d,%d\r\n",
                        (int)base_speed ,
                        (int)base_speed,
                        (int)debug[4] ,
                        (int)debug[5] ,
                        (int)debug[0] ,
                        (int)debug[1]);
}

