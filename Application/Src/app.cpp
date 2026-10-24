#include "app.h"

#include "bsp_usart.h"
#include "car_config.h"
#include "hal_time.h"
#include "hal_uart.h"
#include "service_motion.h"

static service_motion motion;

/* 调试变量 */
float Debug_Data[8] = {0};

/* 当前左右轮指令，由串口命令修改 */
#if CAR_STARTUP_MOTOR_TEST
static int16_t cmd_left  = CAR_STARTUP_TEST_COMMAND;
static int16_t cmd_right = CAR_STARTUP_TEST_COMMAND;
#else
static int16_t cmd_left  = 0;
static int16_t cmd_right = 0;
#endif


static void App_Query(void);
static void App_CmdHandle(const char *line);
static void App_CmdPoll(void);


void App_Init(void)
{
    motion.Car_Init();

    Bsp_Usart_Init();

    HalUart_DebugPrintf("USART Ready\r\n");
    HalUart_DebugPrintf("CMD: SPD:<v> | L:<v> R:<v> | O:<v> | STOP | ?\r\n");
}


void App_Run(void)
{
    static uint32_t last_tick = 0U;

    /* 命令轮询不占控制周期，每圈都跑 */
    App_CmdPoll();

    /* 固定 10 ms 控制周期：编码器差分与速度换算都依赖这个周期 */
    if ((HalTime_GetMs() - last_tick) >= CAR_CONTROL_PERIOD_MS)
    {
        last_tick += CAR_CONTROL_PERIOD_MS;

        motion.Car_Set_Differential(cmd_left, cmd_right);
        motion.Car_Update();
    }
}


void App_SetDifferential(int16_t left, int16_t right)
{
    cmd_left  = left;
    cmd_right = right;
}


void App_Stop(void)
{
    cmd_left  = 0;
    cmd_right = 0;

    motion.Car_Stop();
}


void App_Debug(void)
{
    motion.Debug_Car_Get_Info(Debug_Data);
    Bsp_Usart_Vofa_Transmit8( Debug_Data[0], Debug_Data[1],
                              Debug_Data[2], Debug_Data[3],
                              Debug_Data[4], Debug_Data[5],
                              Debug_Data[6], Debug_Data[7]);
}


/**
 * @brief 串口命令轮询：取一整行并分发
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
 */
static void App_CmdHandle(const char *line)
{
    BspUsartCmd_t cmd;

    Bsp_Usart_ParseCmd(line, &cmd);

    switch (cmd.type)
    {
    case USART_CMD_SPD:
    case USART_CMD_OPEN:
        cmd_left  = (int16_t)cmd.value;
        cmd_right = (int16_t)cmd.value;
        HalUart_DebugPrintf("SPD:%d\r\n", (int)cmd_left);
        break;

    case USART_CMD_LR:
        if (cmd.left_set != 0U)
        {
            cmd_left = (int16_t)cmd.left;
        }
        if (cmd.right_set != 0U)
        {
            cmd_right = (int16_t)cmd.right;
        }
        HalUart_DebugPrintf("L:%d R:%d\r\n", (int)cmd_left, (int)cmd_right);
        break;

    case USART_CMD_STOP:
        cmd_left  = 0;
        cmd_right = 0;
        motion.Car_Stop();
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
 */
static void App_Query(void)
{
    float debug[8];

    motion.Debug_Car_Get_Info(debug);

    HalUart_DebugPrintf("L:%d R:%d ENC:%d,%d SPD:%d,%d\r\n",
                        (int)cmd_left,
                        (int)cmd_right,
                        (int)debug[0],
                        (int)debug[4],
                        (int)debug[1],
                        (int)debug[5]);
}
