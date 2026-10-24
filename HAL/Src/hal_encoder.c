#include "hal_encoder.h"

#include "stm32f1xx_hal.h"
#include "tim.h"

static TIM_HandleTypeDef *const encoder_handles[ENCODER_COUNT] = {
    &htim3,
    &htim4
};

static uint16_t previous_count[ENCODER_COUNT];

void HalEncoder_Init(void)
{
    static uint8_t initialized = 0U;
    uint32_t i;

    /*
     * Board init runs exactly once. HAL_TIM_Encoder_Start() refuses a channel
     * that is already running (returns HAL_ERROR -> Error_Handler).
     * Device_Encoder_Info_Typedef::Init() calls this function once per encoder,
     * so the guard is mandatory.
     */
    if (initialized != 0U)
    {
        return;
    }
    initialized = 1U;

    for (i = 0U; i < ENCODER_COUNT; ++i)
    {
        __HAL_TIM_SET_COUNTER(encoder_handles[i], 0U);

        if (HAL_TIM_Encoder_Start(encoder_handles[i], TIM_CHANNEL_ALL) != HAL_OK)
        {
            Error_Handler();
        }

        previous_count[i] = 0U;
    }
}

int16_t HalEncoder_ReadDelta(EncoderPos_t encoder)
{
    uint16_t current_count;
    int16_t delta;

    if (encoder >= ENCODER_COUNT)
    {
        return 0;
    }

    current_count = (uint16_t)__HAL_TIM_GET_COUNTER(encoder_handles[encoder]);
    delta = (int16_t)(current_count - previous_count[encoder]);
    previous_count[encoder] = current_count;

    return delta;
}

uint16_t HalEncoder_ReadCount(EncoderPos_t encoder)
{
    if (encoder >= ENCODER_COUNT)
    {
        return 0U;
    }

    return (uint16_t)__HAL_TIM_GET_COUNTER(encoder_handles[encoder]);
}

void HalEncoder_Reset(EncoderPos_t encoder)
{
    if (encoder >= ENCODER_COUNT)
    {
        return;
    }

    __HAL_TIM_SET_COUNTER(encoder_handles[encoder], 0U);
    previous_count[encoder] = 0U;
}
