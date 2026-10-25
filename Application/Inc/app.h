#ifndef APP_H
#define APP_H

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void App_Init(void) ;
void App_Run(void)  ;   // 控制入口
void App_Debug(void);

#ifdef __cplusplus
}
#endif

#endif
