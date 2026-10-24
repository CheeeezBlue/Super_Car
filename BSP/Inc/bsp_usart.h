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
    USART_CMD_SPD,          /* SPD:<v>       左右同速 */
    USART_CMD_LR,           /* L:<v> R:<v>   左右差速 */
    USART_CMD_OPEN,         /* O:<v>         开环输出 */
    USART_CMD_STOP,         /* STOP          停车 */
    USART_CMD_QUERY         /* ?             查询状态 */
} BspUsartCmdType_t;

/* 解析后的命令 */
typedef struct
{
    BspUsartCmdType_t type;
    int8_t            left;         /* 左轮指令：-100 ~ 100 */
    int8_t            right;        /* 右轮指令：-100 ~ 100 */
    int16_t           value;        /* 通用数值 */
    uint8_t           left_set  : 1;
    uint8_t           right_set : 1;
} BspUsartCmd_t;


void    Bsp_Usart_Init(void);

/* VOFA+ JustFloat：n 个 float 小端 + 帧尾 00 00 80 7F */
void    Bsp_Usart_Vofa_Transmit3(float value1, float value2, float value3);
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
