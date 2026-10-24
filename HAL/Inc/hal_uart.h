#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* USART2 */
void HalUart_DebugWrite(const uint8_t *data, uint16_t length);
void HalUart_DebugPrintf(const char *format, ...);
void USART_Vofa_Justfloat_Transmit( float SendValue1,
                                    float SendValue2,
                                    float SendValue3);
void USART2_Vofa_Justfloat_Transmit(float value1,float value2,
                                    float value3,float value4,
                                    float value5,float value6,
                                    float value7,float value8);

#ifdef __cplusplus
}
#endif

#endif
