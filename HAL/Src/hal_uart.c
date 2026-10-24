#include "hal_uart.h"

#include <stdarg.h>
#include <stdio.h>

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "string.h"

#define HAL_UART_DEBUG_TIMEOUT_MS 100U
#define HAL_UART_PRINTF_BUFFER    128U

void HalUart_DebugWrite(const uint8_t *data, uint16_t length)
{
    if ((data == 0) || (length == 0U))
    {
        return;
    }

    (void)HAL_UART_Transmit(&huart2, (uint8_t *)data, length,
                            HAL_UART_DEBUG_TIMEOUT_MS);
}

void HalUart_DebugPrintf(const char *format, ...)
{
    char buffer[HAL_UART_PRINTF_BUFFER];
    va_list args;
    int length;

    if (format == 0)
    {
        return;
    }

    va_start(args, format);
    length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length <= 0)
    {
        return;
    }

    if ((uint32_t)length >= sizeof(buffer))
    {
        length = (int)sizeof(buffer) - 1;
    }

    HalUart_DebugWrite((const uint8_t *)buffer, (uint16_t)length);
}

/**
 * @brief  通过USART发送数据到VOFA+（JustFloat协议）
 * @note   VOFA+是一款数据可视化工具，JustFloat是其中一种协议
 * @param  SendValue1 第一个float数据
 * @param  SendValue2 第二个float数据
 * @param  SendValue3 第三个float数据
 * @retval None
 */
void USART_Vofa_Justfloat_Transmit(float SendValue1, float SendValue2, float SendValue3)
{

	static uint8_t Tx_Buf[16]; // 发送缓冲（静态分配在数据区）

	uint8_t *p1 = (uint8_t *)&SendValue1; // 指向第一个 float 的字节表示
	uint8_t *p2 = (uint8_t *)&SendValue2; // 指向第二个 float 的字节表示
	uint8_t *p3 = (uint8_t *)&SendValue3; // 指向第三个 float 的字节表示

	Tx_Buf[0] = p1[0];
	Tx_Buf[1] = p1[1];
	Tx_Buf[2] = p1[2];
	Tx_Buf[3] = p1[3]; // 拷贝第一个 float 的 4 字节
	Tx_Buf[4] = p2[0];
	Tx_Buf[5] = p2[1];
	Tx_Buf[6] = p2[2];
	Tx_Buf[7] = p2[3]; // 拷贝第二个 float 的 4 字节
	Tx_Buf[8] = p3[0];
	Tx_Buf[9] = p3[1];
	Tx_Buf[10] = p3[2];
	Tx_Buf[11] = p3[3]; // 拷贝第三个 float 的 4 字节
	Tx_Buf[12] = 0x00;
	Tx_Buf[13] = 0x00;
	Tx_Buf[14] = 0x80;
	Tx_Buf[15] = 0x7F; // JustFloat 帧尾 0x7F800000（小端）

	HalUart_DebugWrite(Tx_Buf, (uint16_t)sizeof(Tx_Buf));
}


/**
 * @brief  通过USART发送数据到VOFA+（JustFloat协议）
 * @note   VOFA+是一款数据可视化工具，JustFloat是其中一种协议
 * @param  value1 第 1 个float数据
 * @param  value2 第 2 个float数据
 * @param  value3 第 3 个float数据
 * @param  value4 第 4 个float数据
 * @param  value5 第 5 个float数据
 * @param  value6 第 6 个float数据
 * @param  value7 第 7 个float数据
 * @param  value8 第 8 个float数据
 * @retval None
 */
void USART2_Vofa_Justfloat_Transmit(float value1,float value2,
									float value3,float value4,
									float value5,float value6,
									float value7,float value8)
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
	tx_buffer[32] = 0x00U;
	tx_buffer[33] = 0x00U;
	tx_buffer[34] = 0x80U;
	tx_buffer[35] = 0x7FU;
	HalUart_DebugWrite(tx_buffer, (uint16_t)sizeof(tx_buffer));
}

