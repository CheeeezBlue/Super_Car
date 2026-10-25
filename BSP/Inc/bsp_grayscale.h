#ifndef BSP_GRAYSCALE_H
#define BSP_GRAYSCALE_H

#include <stdint.h>
#include "car_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 通道电平含义：0 = 黑线，1 = 白底（与 Model 工程一致） */
#define BSP_GRAY_BLACK      0U
#define BSP_GRAY_WHITE      1U

void Bsp_Grayscale_Init(void);
void Bsp_Grayscale_ReadAll(uint8_t *values);

#ifdef __cplusplus
}
#endif

#endif
