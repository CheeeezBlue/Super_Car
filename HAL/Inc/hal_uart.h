#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 接收环形缓冲区大小，必须是 2 的幂 */
#define HAL_UART_RX_BUFFER_SIZE    256U

/* USART2：发送（阻塞） */
void HalUart_DebugWrite(const uint8_t *data, uint16_t length);
void HalUart_DebugPrintf(const char *format, ...);

/* USART2：接收（中断 + 环形缓冲区） */
void     HalUart_Init(void);
uint16_t HalUart_Available(void);
uint8_t  HalUart_ReadByte(uint8_t *byte);
void     HalUart_Flush(void);

#ifdef __cplusplus
}
#endif

#endif
