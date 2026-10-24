#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "stm32f1xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} HalGpioPin_t;

void HalGpio_Write(const HalGpioPin_t *pin, GPIO_PinState state);

#ifdef __cplusplus
}
#endif

#endif
