#include "hal_time.h"

#include "stm32f1xx_hal.h"

uint32_t HalTime_GetMs(void)
{
    return HAL_GetTick();
}
