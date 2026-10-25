#include "hal_gpio.h"

void HalGpio_Write(const HalGpioPin_t *pin, GPIO_PinState state)
{
    if (pin == 0)
    {
        return;
    }

    HAL_GPIO_WritePin(pin->port, pin->pin, state);
}

GPIO_PinState HalGpio_Read(const HalGpioPin_t *pin)
{
    if (pin == 0)
    {
        return GPIO_PIN_RESET;
    }

    return HAL_GPIO_ReadPin(pin->port, pin->pin);
}
