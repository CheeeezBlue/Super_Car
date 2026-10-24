#include "bsp_usart.h"

#include <stdlib.h>
#include <string.h>

#include "hal_uart.h"

/* 行装配缓冲：内部维护，调用方每次拿到的都是完整一行 */
static char     line_buffer[BSP_USART_LINE_SIZE];
static uint16_t line_length;


/**
 * @brief  USART 板级初始化：启动接收中断
 */
void Bsp_Usart_Init(void)
{
    line_length = 0U;

    HalUart_Init();
}


/**
 * @brief  VOFA+ JustFloat 发送（3 通道）
 */
void Bsp_Usart_Vofa_Transmit3(float value1, float value2, float value3)
{
    static uint8_t tx_buffer[16];

    const float values[3] = { value1, value2, value3 };

    for (uint8_t channel = 0U; channel < 3U; ++channel)
    {
        memcpy(&tx_buffer[channel * 4U], &values[channel], sizeof(float));
    }

    /* JustFloat 帧尾 0x7F800000（小端） */
    tx_buffer[12] = 0x00U;
    tx_buffer[13] = 0x00U;
    tx_buffer[14] = 0x80U;
    tx_buffer[15] = 0x7FU;

    HalUart_DebugWrite(tx_buffer, (uint16_t)sizeof(tx_buffer));
}


/**
 * @brief  VOFA+ JustFloat 发送（8 通道）
 */
void Bsp_Usart_Vofa_Transmit8(float value1, float value2,
                              float value3, float value4,
                              float value5, float value6,
                              float value7, float value8)
{
    static uint8_t tx_buffer[36];

    const float values[8] = {
        value1, value2, value3, value4,
        value5, value6, value7, value8
    };

    for (uint8_t channel = 0U; channel < 8U; ++channel)
    {
        memcpy(&tx_buffer[channel * 4U], &values[channel], sizeof(float));
    }

    /* JustFloat 帧尾 0x7F800000（小端） */
    tx_buffer[32] = 0x00U;
    tx_buffer[33] = 0x00U;
    tx_buffer[34] = 0x80U;
    tx_buffer[35] = 0x7FU;

    HalUart_DebugWrite(tx_buffer, (uint16_t)sizeof(tx_buffer));
}


/**
 * @brief  从接收缓冲区提取一整行（以 '\n' 结束，'\r' 被忽略）
 * @param  buffer 输出缓冲
 * @param  maxlen 输出缓冲长度
 * @retval 1=取到完整一行，0=还没有
 * @note   超长行整行丢弃，避免后续命令全部错位
 */
uint8_t Bsp_Usart_GetLine(char *buffer, uint16_t maxlen)
{
    uint8_t byte;

    if ((buffer == 0) || (maxlen < 2U))
    {
        return 0U;
    }

    while (HalUart_ReadByte(&byte) != 0U)
    {
        if (byte == '\r')
        {
            continue;
        }

        if (byte == '\n')
        {
            uint16_t copy = (line_length < (uint16_t)(maxlen - 1U))
                            ? line_length : (uint16_t)(maxlen - 1U);

            memcpy(buffer, line_buffer, copy);
            buffer[copy] = '\0';

            uint16_t had = line_length;
            line_length = 0U;

            if (had != 0U)
            {
                return 1U;
            }
            continue;           /* 空行，继续等下一行 */
        }

        if (line_length < (BSP_USART_LINE_SIZE - 1U))
        {
            line_buffer[line_length] = (char)byte;
            ++line_length;
        }
        else
        {
            line_length = 0U;   /* 超长丢弃 */
        }
    }

    return 0U;
}


/**
 * @brief  文本命令解析
 * @note   支持的格式：
 *           SPD:50          左右同速
 *           L:50 R:30       左右差速（可只给一侧）
 *           R:30 L:50       同上，顺序无关
 *           O:40            开环输出
 *           STOP            停车
 *           ?               查询状态
 */
void Bsp_Usart_ParseCmd(const char *line, BspUsartCmd_t *cmd)
{
    const char *other;

    if ((line == 0) || (cmd == 0))
    {
        return;
    }

    cmd->type      = USART_CMD_NONE;
    cmd->left      = 0;
    cmd->right     = 0;
    cmd->value     = 0;
    cmd->left_set  = 0U;
    cmd->right_set = 0U;

    if (line[0] == '\0')
    {
        return;
    }

    if (strncmp(line, "SPD:", 4) == 0)
    {
        cmd->type  = USART_CMD_SPD;
        cmd->value = (int16_t)atoi(line + 4);
    }
    else if (strncmp(line, "O:", 2) == 0)
    {
        cmd->type  = USART_CMD_OPEN;
        cmd->value = (int16_t)atoi(line + 2);
    }
    else if (strncmp(line, "L:", 2) == 0)
    {
        cmd->type = USART_CMD_LR;
        cmd->left = (int8_t)atoi(line + 2);
        cmd->left_set = 1U;

        other = strstr(line, "R:");
        if (other != 0)
        {
            cmd->right = (int8_t)atoi(other + 2);
            cmd->right_set = 1U;
        }
    }
    else if (strncmp(line, "R:", 2) == 0)
    {
        cmd->type = USART_CMD_LR;
        cmd->right = (int8_t)atoi(line + 2);
        cmd->right_set = 1U;

        other = strstr(line, "L:");
        if (other != 0)
        {
            cmd->left = (int8_t)atoi(other + 2);
            cmd->left_set = 1U;
        }
    }
    else if (strncmp(line, "STOP", 4) == 0)
    {
        cmd->type = USART_CMD_STOP;
    }
    else if (line[0] == '?')
    {
        cmd->type = USART_CMD_QUERY;
    }
}
