#include "hal_time.h"

#include "stm32f1xx_hal.h"

/* DWT 周期计数器是否已使能 */
static uint8_t dwt_ready = 0U;


uint32_t HalTime_GetMs(void)
{
    return HAL_GetTick();
}


/**
 * @brief 使能 DWT 周期计数器
 * @note  顺序不能反：先开 DEMCR.TRCENA，再开 DWT.CTRL.CYCCNTENA，否则计数器不跑。
 */
static void HalTime_DwtInit(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DWT->CYCCNT = 0U;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    dwt_ready = 1U;
}


void HalTime_DelayUs(uint16_t us)
{
    uint32_t start;
    uint32_t ticks;

    if (us == 0U)
    {
        return;
    }

    if (dwt_ready == 0U)
    {
        HalTime_DwtInit();
    }

    /* 72 MHz 下 1 us = 72 个周期；us 最大 65535 -> 约 4.7M 周期，32 位足够 */
    ticks = (uint32_t)us * (SystemCoreClock / 1000000U);
    start = DWT->CYCCNT;

    while ((DWT->CYCCNT - start) < ticks)
    {
        /* 忙等 */
    }
}
