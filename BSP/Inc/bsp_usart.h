#ifndef BSP_USART_H
#define BSP_USART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 单行命令最大长度（含结束符） */
#define BSP_USART_LINE_SIZE        64U

/* 解析后的命令类型 */
typedef enum
{
    USART_CMD_NONE = 0,
    USART_CMD_SPD,          /* SPD:<v>       目标速度（百分比） */
    USART_CMD_STOP,         /* STOP          停车 */
    USART_CMD_MODE,         /* MD:<n>        切换运行模式 0=IDLE 1=MANUAL 2=STRAIGHT 3=LINE */
    USART_CMD_QUERY         /* ?             查询状态 */
} BspUsartCmdType_t;

/* 解析后的命令 */
typedef struct
{
    BspUsartCmdType_t type;
    int16_t           value;        /* 通用数值：速度百分比 / 模式编号 */
} BspUsartCmd_t;


void    Bsp_Usart_Init(void);

/* VOFA+ JustFloat：n 个 float 小端 + 帧尾 00 00 80 7F */
void    Bsp_Usart_Vofa_Transmit8(float value1, float value2,
                                 float value3, float value4,
                                 float value5, float value6,
                                 float value7, float value8);

/* 命令行：从接收缓冲区取一整行（非阻塞） */
uint8_t Bsp_Usart_GetLine(char *buffer, uint16_t maxlen);
void    Bsp_Usart_ParseCmd(const char *line, BspUsartCmd_t *cmd);

#ifdef __cplusplus
}
#endif

#endif
