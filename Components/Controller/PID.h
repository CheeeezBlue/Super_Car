/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef PID_H
#define PID_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "LPF.h"

/* Exported types ------------------------------------------------------------*/
/**
 * @brief PID控制器的错误状态枚举。
 */
typedef enum
{
  PID_ERROR_NONE = 0x00U,  /*!< 无错误 */
  PID_FAILED_INIT = 0x01U, /*!< 初始化失败 */
  PID_CALC_NANINF = 0x02U, /*!< 产生了非数(NaN)或无穷大 */
  PID_Status_NUM,
} PID_Status_e;

/**
 * @brief PID控制器的类型枚举。
 */
typedef enum
{
  PID_Type_None = 0x00U, /*!< 无类型 */
  PID_POSITION = 0x01U,  /*!< 位置式 pid */
  PID_DELTA = 0x02U,     /*!< 增量式 pid */
  PID_TYPE_NUM,
} PID_Type_e;

/**
 * @brief PID控制器的改进方式枚举。
 */
typedef enum
{
  PID_Improvement_NONE = 0x00U,          /*!< 无 */
  PID_Derivative_On_Measurement = 0x01U, /*!< 微分先行 */
  PID_Trapezoid_Integral = 0x02U,        /*!< 梯形积分 */
} PID_Improvement_e;
/**
 * @brief PID错误处理信息的结构体。
 */
typedef struct
{
  uint16_t ErrorCount; /*!< 错误状态判断计数 */
  PID_Status_e Status; /*!< 错误状态 */
} PID_ErrorHandler_Typedef;

/**
 * @brief PID控制器参数的结构体。
 */
typedef struct
{
  float KP;                      // 比例系数
  float KI;                      // 积分系数
  float KD;                      // 微分系数
  float Alpha;                   // 微分一阶滤波器系数
  float Deadband;                // 死区 当误差绝对值小于死区 PID停止计算。
  float LimitIntegral;           // 积分限幅
  float LimitOutput;             // 总输出限幅
  float CoefA;                   // 变速积分全积分阈值 A, |误差|<A 时全积分
  float CoefB;                   // 变速积分最大阈值 B, |误差|>=B 时不积分, >0 时启用变速积分
  float IntegralSeparation;      // 积分分离阈值, |误差|超过该值时取消积分, >0 时启用积分分离
  float K_F;                     // 前馈系数 , 前馈量 = K_F*(目标值-上次目标值)
  PID_Improvement_e Improvement; // PID改进方式

} PID_Parameter_Typedef;
/**
 * @brief PID控制器信息的结构体。
 */
typedef struct _PID_TypeDef
{
  PID_Type_e Type; // PID类型 位置式or增量式 通常使用位置式

  float Target;       // 目标值
  float Last_Target;  // 上一次目标值, 前馈使用
  float Measure;      // 实际值
  float Last_Measure; // 上一次实际值, 微分先行使用

  float Err[3];   // 误差 目标值-期望值 = 误差 当前以及过去两次的误差
  float Integral; // 误差积分值 误差累加
  float Pout;     // KP * 误差值 比例输出
  float Iout;     // KI * 误差积分 积分输出
  float Dout;     // KD * 误差微分（差分）微分输出
  float Output;   // 总输出 Pout + Iout + Dout = Output

  LowPassFilter1p_Info_TypeDef Dout_LPF; // 微分输出的一阶滤波器

  PID_Parameter_Typedef Param;           // PID参数结构体
  PID_ErrorHandler_Typedef ERRORHandler; // PID错误处理结构体

  /**
   * @brief 初始化PID参数的函数指针，检查PID类型并初始化微分滤波器。
   * @param PID: 指向_pid_TypeDef结构的指针，包含PID控制器的信息。
   * @retval PID错误状态 返回PID是否初始化成功。
   */
  PID_Status_e (*PID_Param_Init)(struct _PID_TypeDef *PID);

  /**
   * @brief 清除pid计算函数的简短指针。
   * @param PID:指向_pid_TypeDef结构的指针，包含PID控制器的信息。
   * @retval 无.
   */
  void (*PID_Calc_Clear)(struct _PID_TypeDef *PID);

} PID_Info_TypeDef;

/* Exported functions prototypes ---------------------------------------------*/
/**
 * @brief 初始化PID控制器。
 */
extern void PID_Init(PID_Info_TypeDef *Pid);
/**
 * @brief  计算PID控制器
 */
extern float PID_Calculate(PID_Info_TypeDef *PID, float Target, float Measure);

#ifdef __cplusplus
}
#endif

#endif // PID_H
