#include "hal_gpio.h"

void HalGpio_Write(const HalGpioPin_t *pin, GPIO_PinState state)
{
    if (pin == 0)
    {
        return;
    }

    HAL_GPIO_WritePin(pin->port, pin->pin, state);
}
