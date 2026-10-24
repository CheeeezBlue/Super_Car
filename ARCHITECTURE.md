# Super_Car software architecture

## Current milestone

The current milestone is open-loop bring-up:

1. Start TIM1 CH1-CH4 PWM at 20 kHz.
2. Drive four TB6612 motor channels through `Device_Motor_Info_Typedef`.
3. Keep both USART2 and USART3 available.
4. Start the TIM3/TIM4 encoder counters and sample them every 10 ms through
   `Device_Encoder_Info_Typedef::Info_Update`.
5. Emit a fixed open-loop command from `App_Run()` for bench validation, and
   stream eight debug channels to VOFA+ on USART2.

Lift the wheels before the first run. `App_Run()` currently drives all four
motors with a fixed command. The command must be large enough to overcome the
MG310 gearbox stiction: 20 % duty is frequently too low to start a loaded
four-wheel chassis, 40-60 % is a more realistic first test value.

Two hardware preconditions cannot be detected by the firmware:

- `STBY` on both TB6612 boards must be tied high (3.3 V) or driven by a GPIO.
  The firmware does not control it, so a floating `STBY` yields no output at all.
- `VM` must be powered and its ground shared with the MCU.

The code is written against the CubeMX-generated HAL project. Generated files
remain in `Core/` and `Drivers/`; application code is kept outside those
directories.

Project-owned implementations from `BSP/` upward (`BSP/`, `Device/`,
`Service/`, and `Application/`) use `.cpp`. The `HAL/` wrapper and CubeMX
generated sources remain C. Public headers retain `.h` and guard C-facing
declarations with `#ifdef __cplusplus` / `extern "C"`.

Toolchain note: the project builds with ARM Compiler 5.06 (armcc) without the
`--cpp11` switch. In-class member initialisers (`int8_t sign = 1;`) and
`for (int i = 0; ...)` are accepted in this configuration. GNU range
initialisers (`[0 ... 3]`) and C++ designated initialisers (`.field = value`)
are not.

## Layer rules

| Layer | Directory | Allowed dependency |
|---|---|---|
| Application | `Application/` | Service, Device, BSP, HAL wrapper |
| Service | `Service/` | Device; BSP motor-position enum for the assembly table |
| Device abstraction | `Device/` | BSP type enum, board polarity from `Config/` |
| BSP | `BSP/` | HAL wrapper, board pin mapping |
| HAL wrapper | `HAL/` | CubeMX generated handles and STM32 HAL |
| Generated hardware | `Core/`, `Drivers/` | STM32CubeMX / STM32 HAL |

Upper layers must not include `main.h`, `gpio.h`, `tim.h`, `usart.h`, or call
HAL functions directly.

`Service/` holds the single assembly table (`Motor_Cfg` in
`service_motion.cpp`) that binds each wheel position to a motor type and a
polarity. It is the only place allowed to name `BSP_TB6612_*` positions; the
device layer only stores the value it is handed.

`Bsp_tb6612_Init()` is a one-shot board initialiser. It starts the TIM1 PWM
channels, and `HAL_TIM_PWM_Start()` rejects a channel that is already running,
so a second call would trap in `Error_Handler()`. An internal guard makes
repeated calls harmless.

## Current call path

```text
main
  -> App_Init
       -> service_motion::Car_Init
            -> Device_Motor_Info_Typedef::Init      (x4, from Motor_Cfg)
                 -> Bsp_tb6612_Init                 (one-shot, starts TIM1 PWM)
                      -> HalPwm_Init -> HAL_TIM_PWM_Start
  -> App_Run  (while(1), 10 ms software tick)
       -> service_motion::Car_Run(value)
            -> Device_Motor_Info_Typedef::Set_Value (x4)
                 -> Bsp_tb6612_SetMotor
                      -> HalGpio_Write / HalPwm_SetDuty
       -> service_motion::Car_Update
            -> Device_Encoder_Info_Typedef::Info_Update (x2)
                 -> HalEncoder_ReadDelta -> TIM3 / TIM4 CNT
  -> App_Debug
       -> service_motion::Debug_Car_Get_Info
       -> USART2_Vofa_Justfloat_Transmit (8 channels)
```

`App_Stop` -> `service_motion::Car_Stop` -> `Device_Motor_Info_Typedef::Stop`.
Note that `Stop()` currently forwards to `Bsp_tb6612_StopAll()`, so stopping a
single wheel stops all four.

## Motor command convention

Motor commands use the range `-100..100`, in percent of full duty:

- `0`: coast stop, IN1=0 and IN2=0
- positive: IN1=1 and IN2=0
- negative: IN1=0 and IN2=1

`CAR_MOTOR_COMMAND_MAX` in `Config/Inc/car_config.h` is the single source of
this range; `bsp_tb6612.cpp` scales it to the TIM1 auto-reload value.

Polarity is owned by the device layer. `Device_Motor_Info_Typedef::sign`
(default `1`, set from `CAR_MOTOR_SIGN_*` at assembly time) is applied inside
`Set_Value()`. It lives on the info object, not on the data object, so
`Reset_Data()` cannot wipe a calibrated polarity. The BSP layer applies no
polarity: it drives IN1/IN2 straight from the sign of the command it receives.
Validate each wheel at low duty before running the whole vehicle.

`Device_Encoder_Info_Typedef::sign` follows the same rule and is applied to
`HalEncoder_ReadDelta()` inside `Info_Update()`, so a forward turn always
yields a positive delta.

## Planned increments

1. Add a line-based debug command parser on USART2.
2. Add a safety timeout that stops all motors when commands expire.
3. Add grey-scale sensor service and line-following state.
4. Add closed-loop speed control (PID) on top of the encoder sampling.
5. Add formal application states: safe, manual, line-follow, fault.
