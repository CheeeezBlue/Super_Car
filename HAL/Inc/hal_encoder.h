#ifndef HAL_ENCODER_H
#define HAL_ENCODER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 编码器安装位置 */
typedef enum
{
    ENCODER_LEFT = 0,
    ENCODER_RIGHT,
    ENCODER_COUNT
} EncoderPos_t;

void HalEncoder_Init(void);
int16_t HalEncoder_ReadDelta(EncoderPos_t encoder);
void HalEncoder_Reset(EncoderPos_t encoder);

#ifdef __cplusplus
}
#endif

#endif
