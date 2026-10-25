#include "bsp_grayscale.h"

#include "hal_gpio.h"
#include "hal_time.h"

/*
 * 八路灰度模块引脚（与 Docs/First_Board_Pins.md 第六节一致）：
 *   OUT  PA5  输入      当前选中通道的数字量
 *   AD0  PA4  输出      通道选择 bit0
 *   AD1  PA0  输出      通道选择 bit1
 *   AD2  PA1  输出      通道选择 bit2
 * 四个引脚已在 CubeMX 的 MX_GPIO_Init() 里配置好，这里只负责选择与读取。
 */
static const HalGpioPin_t gray_out = { GPIOA, GPIO_PIN_5 };

static const HalGpioPin_t gray_ad[3] = {
    { GPIOA, GPIO_PIN_4 },   /* AD0 */
    { GPIOA, GPIO_PIN_0 },   /* AD1 */
    { GPIOA, GPIO_PIN_1 }    /* AD2 */
};


/**
 * @brief 选择灰度通道
 * @param ch: 通道号 0 ~ 7
 */
static void Bsp_Grayscale_SelectChannel(uint8_t ch)
{
    HalGpio_Write(&gray_ad[0], ((ch >> 0) & 1U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HalGpio_Write(&gray_ad[1], ((ch >> 1) & 1U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HalGpio_Write(&gray_ad[2], ((ch >> 2) & 1U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


/**
 * @brief 灰度模块初始化
 * @note  引脚本身由 CubeMX 配置，这里只把选择线拉低（选中通道 0）。
 *        加一次性守卫与 BSP/Device 其它模块保持一致，重复调用无副作用。
 */
void Bsp_Grayscale_Init(void)
{
    static uint8_t initialized = 0U;

    if (initialized != 0U)
    {
        return;
    }
    initialized = 1U;

    Bsp_Grayscale_SelectChannel(0U);
}


/**
 * @brief 读取全部通道
 * @param values: 输出缓冲，长度至少 CAR_GRAY_CHANNEL_COUNT
 * @retval None
 * @note  逐通道切换 + 等待建立时间后读一次 OUT。
 *        8 通道 x 100 us ≈ 0.8 ms，放在 10 ms 控制周期里可以接受。
 */
void Bsp_Grayscale_ReadAll(uint8_t *values)
{
    uint8_t ch;

    if (values == 0)
    {
        return;
    }

    for (ch = 0U; ch < CAR_GRAY_CHANNEL_COUNT; ++ch)
    {
        Bsp_Grayscale_SelectChannel(ch);

        /* 多路选择器切换 + 比较器输出稳定 */
        HalTime_DelayUs(CAR_GRAY_SETTLE_US);

        values[ch] = (HalGpio_Read(&gray_out) == GPIO_PIN_SET) ? BSP_GRAY_WHITE : BSP_GRAY_BLACK;
    }
}
