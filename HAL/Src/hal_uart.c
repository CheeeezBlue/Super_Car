#include "hal_uart.h"

#include <stdarg.h>
#include <stdio.h>

#include "stm32f1xx_hal.h"
#include "usart.h"

#define HAL_UART_DEBUG_TIMEOUT_MS 100U
#define HAL_UART_PRINTF_BUFFER    128U

/* ------------------------------------------------------------------
 * 接收环形缓冲区
 *   head 由中断写入，tail 由主循环读取，两者都只在单侧修改，
 *   16 位下标的自然回绕配合掩码即可，不需要关中断。
 * ------------------------------------------------------------------ */
static volatile uint8_t  rx_buffer[HAL_UART_RX_BUFFER_SIZE];
static volatile uint16_t rx_head;
static volatile uint16_t rx_tail;

/* HAL 单字节接收落点 */
static uint8_t rx_byte;

static void rx_push(uint8_t byte)
{
    uint16_t next = (uint16_t)((rx_head + 1U) & (HAL_UART_RX_BUFFER_SIZE - 1U));

    if (next != rx_tail)
    {
        rx_buffer[rx_head] = byte;
        rx_head = next;
    }
}


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
 * @brief  初始化 USART2 接收：清溢出标志并启动单字节中断接收
 * @note   上电前若线上有残留电平，ORE 会置位且不会自动清除，
 *         必须先读 SR 再读 DR 把它清掉，否则一开接收就报溢出。
 */
void HalUart_Init(void)
{
    rx_head = 0U;
    rx_tail = 0U;

    __HAL_UART_CLEAR_OREFLAG(&huart2);

    (void)HAL_UART_Receive_IT(&huart2, &rx_byte, 1U);
}


/**
 * @brief  环形缓冲区中待读取的字节数
 */
uint16_t HalUart_Available(void)
{
    return (uint16_t)((rx_head - rx_tail) & (HAL_UART_RX_BUFFER_SIZE - 1U));
}


/**
 * @brief  从环形缓冲区取一个字节
 * @param  byte 取到的字节
 * @retval 1=取到，0=缓冲区空
 */
uint8_t HalUart_ReadByte(uint8_t *byte)
{
    if ((byte == 0) || (rx_head == rx_tail))
    {
        return 0U;
    }

    *byte = rx_buffer[rx_tail];
    rx_tail = (uint16_t)((rx_tail + 1U) & (HAL_UART_RX_BUFFER_SIZE - 1U));

    return 1U;
}


/**
 * @brief  丢弃缓冲区中尚未读取的字节
 */
void HalUart_Flush(void)
{
    rx_tail = rx_head;
}


/**
 * @brief  接收完成回调：入环形缓冲区并重新挂载下一字节
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        rx_push(rx_byte);
        (void)HAL_UART_Receive_IT(&huart2, &rx_byte, 1U);
    }
}


/**
 * @brief  错误回调：清溢出并重新挂载接收，避免一次 ORE 之后永久收不到数据
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        __HAL_UART_CLEAR_OREFLAG(&huart2);
        (void)HAL_UART_Receive_IT(&huart2, &rx_byte, 1U);
    }
}
