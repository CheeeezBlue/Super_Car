/* Includes ------------------------------------------------------------------*/
#include "PID.h"

/* 原工程依赖 User_Lib.h 提供的 VAL_LIMIT，本工程未引入该头文件，此处补齐 */
#ifndef VAL_LIMIT
#define VAL_LIMIT(x, min, max)                 \
  do {                                         \
    if ((x) < (min)) { (x) = (min); }          \
    else if ((x) > (max)) { (x) = (max); }     \
  } while (0)
#endif


/**
 * @brief 初始化PID参数.
 * @Param PID: 指向PID_Info_TypeDef结构的指针，包含PID控制器的信息.
 * @retval PID错误状态.
 */
static PID_Status_e PID_Param_Init(PID_Info_TypeDef *PID)
{
  // 判断PID类型是否为空 若为空 返回PID_FAILED_INIT（初始化失败）
  if (PID->Type == PID_Type_None)
  {
    return PID_FAILED_INIT;
  }

  // 初始化微分滤波器
  if (PID->Param.Alpha > 0.f && PID->Param.Alpha < 1.f)
  {
    LowPassFilter1p_Init(&PID->Dout_LPF, PID->Param.Alpha);
  }

  // 清除PID错误计数
  PID->ERRORHandler.ErrorCount = 0;

  // 返回PID_ERROR_NONE（无错误状态）
  return PID_ERROR_NONE;
}
//------------------------------------------------------------------------------

/**
 * @brief 清除PID计算值，将所有输出赋0.
 * @Param PID: 指向PID_Info_TypeDef结构的指针，包含PID控制器的信息.
 * @retval 无.
 */
static void PID_Calc_Clear(PID_Info_TypeDef *PID)
{
  // 将所有输出赋0
  memset(PID->Err, 0, sizeof(PID->Err));
  PID->Integral = 0;

  PID->Pout = 0;
  PID->Iout = 0;
  PID->Dout = 0;
  PID->Output = 0;
}
//------------------------------------------------------------------------------

/**
 * @brief 初始化PID控制器.
 * @Param PID: 指向PID_Info_TypeDef结构的指针，包含PID控制器的信息.
 * @retval 无.
 */
void PID_Init(PID_Info_TypeDef *PID)
{
  PID->PID_Calc_Clear = PID_Calc_Clear;
  PID->PID_Param_Init = PID_Param_Init;

  PID->PID_Calc_Clear(PID);
  PID->ERRORHandler.Status = PID->PID_Param_Init(PID);
}
//------------------------------------------------------------------------------

/**
 * @brief 判断PID错误状态
 * @Param PID: 指向PID_Info_TypeDef结构的指针，包含PID控制器的信息.
 * @retval 无.
 */
static void PID_ErrorHandle(PID_Info_TypeDef *PID)
{
  /* 判断NAN/INF */
  if (isnan(PID->Output) == true || isinf(PID->Output) == true)
  {
    PID->ERRORHandler.Status = PID_CALC_NANINF;
  }
}
//------------------------------------------------------------------------------

/**
 * @brief  PID控制器计算函数
 * @Param  *PID 指向PID_TypeDef_t结构体的指针，包含指定PID控制器的配置信息.
 * @Param  Target  PID控制器的目标值
 * @Param  Measure  PID控制器的实际值
 * @retval PID输出值
 */
float PID_Calculate(PID_Info_TypeDef *PID, float Target, float Measure)
{
  /* 更新PID错误状态 */
  PID_ErrorHandle(PID);
  if (PID->ERRORHandler.Status != PID_ERROR_NONE)
  {
    PID->PID_Calc_Clear(PID);
    return 0;
  }

  /* 更新目标值/实际值 */
  PID->Target = Target;
  PID->Measure = Measure;

  /* 更新误差 */
  PID->Err[2] = PID->Err[1];
  PID->Err[1] = PID->Err[0];
  PID->Err[0] = PID->Target - PID->Measure;

  if (fabsf(PID->Err[0]) >= PID->Param.Deadband)
  {
    /* 更新PID控制器输出 */
    if (PID->Type == PID_POSITION)
    {
      /* 更新比例输出 */
      PID->Pout = PID->Param.KP * PID->Err[0];
      /* 更新PID积分 */
      if (PID->Param.KI != 0)
      {
        float integral_increment=0.0f;

        /* 梯形积分: 计算梯形面积, (上底+下底)*高/2 = (Err[0]+Err[1])/2, 高即1个采样周期 */
        if (PID->Param.Improvement & PID_Trapezoid_Integral)
        {
          integral_increment = (PID->Err[0] + PID->Err[1]) * 0.5f;
        }
        else
        {
          integral_increment = PID->Err[0];
        }
        /* 变速积分: CoefB>0 时启用, 误差小时积分作用更强
           y = 1             (|e| < A)
           y = (B-|e|)/(B-A) (A <= |e| < B)
           y = 0             (|e| >= B) */
        if ((PID->Param.CoefB > 0.0f) && (PID->Err[0] * PID->Iout > 0.0f))
        {
          float err_abs = fabsf(PID->Err[0]);

          if (err_abs >= PID->Param.CoefB)
          {
            /* |误差| >= B, 不使用积分 */
            integral_increment = 0.0f;
          }
          else if ((err_abs >= PID->Param.CoefA) && (PID->Param.CoefB > PID->Param.CoefA))
          {
            integral_increment *= (PID->Param.CoefB - err_abs) / (PID->Param.CoefB - PID->Param.CoefA);
          }
        }
        /* 积分分离: IntegralSeparation>0 时启用, |误差|超过阈值取消积分 */
        if ((PID->Param.IntegralSeparation > 0.0f) && (fabsf(PID->Err[0]) > PID->Param.IntegralSeparation))
        {
          integral_increment = 0.0f;
        }

        PID->Integral += integral_increment;
        VAL_LIMIT(PID->Integral, -PID->Param.LimitIntegral, PID->Param.LimitIntegral);
        PID->Iout = PID->Param.KI * PID->Integral;
      }
      else
      {
        PID->Integral = 0;
        PID->Iout = 0;
      }

      /* 更新微分输出: 微分先行仅对反馈量微分, 避免给定突变冲击 */
      if (PID->Param.Improvement & PID_Derivative_On_Measurement)
      {
        PID->Dout = PID->Param.KD * (PID->Last_Measure - PID->Measure);
      }
      else
      {
        PID->Dout = PID->Param.KD * (PID->Err[0] - PID->Err[1]);
      }
      if (PID->Param.Alpha > 0.f && PID->Param.Alpha < 1.f)
      {

        PID->Dout_LPF.Alpha = PID->Param.Alpha;
        PID->Dout = LowPassFilter1p_Update(&PID->Dout_LPF, PID->Dout);
      }
      float Feedforward = PID->Param.K_F * (PID->Target - PID->Last_Target);
      /* 更新PID输出 (叠加前馈量: K_F * (目标值 - 上次目标值)) */
      PID->Output = PID->Pout + PID->Iout + PID->Dout + Feedforward;
      VAL_LIMIT(PID->Output, -PID->Param.LimitOutput, PID->Param.LimitOutput);
    }
    else if (PID->Type == PID_DELTA)
    {
      /* 更新比例输出、积分输出、微分输出 */
      PID->Pout = PID->Param.KP * (PID->Err[0] - PID->Err[1]);
      PID->Iout = PID->Param.KI * (PID->Err[0]);
      PID->Dout = PID->Param.KD * (PID->Err[0] - 2.f * PID->Err[1] + PID->Err[2]);
      if (PID->Param.Alpha > 0.f && PID->Param.Alpha < 1.f)
      {
        PID->Dout_LPF.Alpha = PID->Param.Alpha;
        PID->Dout = LowPassFilter1p_Update(&PID->Dout_LPF, PID->Dout);
      }
      /* 更新PID输出 */
      PID->Output += PID->Pout + PID->Iout + PID->Dout;
      VAL_LIMIT(PID->Output, -PID->Param.LimitOutput, PID->Param.LimitOutput);
    }
  }
  else
  {
    PID->PID_Calc_Clear(PID);
  }
  /* 保存上一次目标值/实际值, 供前馈/微分先行使用 */
  PID->Last_Target = PID->Target;
  PID->Last_Measure = PID->Measure;

  return PID->Output;
}
//------------------------------------------------------------------------------
