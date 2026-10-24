# Super_Car

基于 **STM32F103C8T6** 的四驱智能巡线小车，分层驱动框架 + 双路编码器测速。

当前处于**开环台架验证 + 编码器采样**阶段，尚未接入速度闭环。

---

## 硬件组成

| 项目 | 型号 / 说明 |
|---|---|
| 主控 | STM32F103C8T6，72 MHz，64 KB Flash / 20 KB SRAM |
| 电机 | 轮趣科技 MG310 减速电机 ×4，带霍尔编码器（13 线） |
| 驱动 | TB6612FNG ×2，分别驱动前组（FL/FR）与后组（RL/RR） |
| 编码器 | 仅左后 RL（TIM3）、右后 RR（TIM4）接入 MCU，按差速模型推算左右速度 |
| 循迹 | 八路灰度模块（预留） |
| 调试 | USART2 → VOFA+（JustFloat 协议，8 通道） |

---

## 引脚分配

| 功能 | 外设 | 引脚 |
|---|---|---|
| 四路电机 PWM | TIM1 CH1–CH4 | PA8 / PA9 / PA10 / PA11 |
| 左侧编码器 | TIM3 Encoder | PA6 / PA7 |
| 右侧编码器 | TIM4 Encoder | PB6 / PB7 |
| 硬件 I2C | I2C1 Remap | PB8 / PB9 |
| 调试串口 | USART2 | PA2 / PA3 |
| 预留通信串口 | USART3 | PB10 / PB11 |
| 八路灰度模块 | GPIO | PA0 / PA1 / PA4 / PA5 |
| 调试下载 | SWD | PA13 / PA14 |

方向控制 GPIO：

| 车轮 | PWMA/B | AIN1/BIN1 | AIN2/BIN2 |
|---|---|---|---|
| FL | PA8 | PB12 | PB13 |
| FR | PA9 | PB14 | PB15 |
| RL | PA10 | PB0 | PB1 |
| RR | PA11 | PB5 | PA12 |

> `STBY` 由固件之外的硬件拉高（3.3 V）。若悬空，TB6612 输出为高阻，程序再对也不转。
> `VM` 必须供电，且与主控共地。

完整的按轮位接线表、TB6612 公共信号、CubeMX 各外设参数（TIM1 / TIM3 / TIM4 / I2C1 / USART）见
**[Docs/First_Board_Pins.md](Docs/First_Board_Pins.md)**。

---

## 软件分层

| 层 | 目录 | 职责 | 语言 |
|---|---|---|---|
| Application | `Application/` | 编排、控制周期 | C++ |
| Service | `Service/` | 差速分配、装配表、调试数据打包 | C++ |
| Device | `Device/` | 电机 / 编码器抽象、极性、单位换算 | C++ |
| BSP | `BSP/` | TB6612 引脚映射、指令 → 占空比 | C++ |
| HAL | `HAL/` | 外设封装（PWM / GPIO / TIM 编码器 / 串口 / 时基） | C |
| 生成代码 | `Core/`、`Drivers/` | STM32CubeMX + STM32 HAL | C |

上层不得直接 `#include "main.h"` / `tim.h` / `gpio.h`，也不得直接调用 HAL 函数。

### 实际调用链

```text
main
  └─ App_Init
       ├─ service_motion::Car_Init
       │    ├─ Device_Motor_Info_Typedef::Init      ×4  (装配表 Motor_Cfg)
       │    │    └─ Bsp_tb6612_Init                 (一次性，启动 TIM1 PWM)
       │    └─ Device_Encoder_Info_Typedef::Init    ×2  (装配表 Encoder_Cfg)
       │         └─ HalEncoder_Init                 (一次性，启动 TIM3/TIM4)
       └─ Bsp_Usart_Init
            └─ HalUart_Init                         (清 ORE，挂单字节接收中断)

  └─ while (1)
       ├─ App_Run
       │    ├─ App_CmdPoll                          (每圈都跑，不占控制周期)
       │    │    └─ Bsp_Usart_GetLine → ParseCmd → App_CmdHandle
       │    ├─ service_motion::Car_Set_Differential(left, right)
       │    │    └─ Device_Motor_Info_Typedef::Set_Value   ×4
       │    │         └─ Bsp_tb6612_SetMotor → HalGpio_Write / HalPwm_SetDuty
       │    └─ service_motion::Car_Update
       │         └─ Device_Encoder_Info_Typedef::Info_Update ×2
       │              └─ HalEncoder_ReadDelta → TIM3 / TIM4 CNT
       └─ App_Debug
            └─ Debug_Car_Get_Info → Bsp_Usart_Vofa_Transmit8
```

`Bsp_tb6612_Init()` 与 `HalEncoder_Init()` 都是**一次性板级初始化**：`HAL_TIM_PWM_Start` / `HAL_TIM_Encoder_Start` 会拒绝已在运行的通道并返回 `HAL_ERROR`，重复调用会掉进 `Error_Handler()`，因此两者内部都带 `static` 守卫。

---

## 目录结构

```text
Super_Car/
├── Application/     app.h / app.cpp          控制周期与调试输出
├── Service/         service_motion.*         电机 + 编码器的装配与调度
├── Device/          device_motor.*           电机抽象
│                    device_encoder.*         编码器抽象
├── BSP/             bsp_tb6612.*             TB6612 板级映射
│                    bsp_usart.*              VOFA 组帧 + 命令行解析
├── HAL/             hal_pwm / hal_gpio / hal_encoder / hal_uart / hal_time
├── Config/          car_config.h             全部可调参数集中于此
├── Core/            CubeMX 生成（main / tim / gpio / usart / i2c）
├── Drivers/         STM32 HAL + CMSIS
├── MDK-ARM/         Keil 工程
├── Docs/            详细引脚表与 CubeMX 配置
├── Super_Car.ioc    CubeMX 工程文件
└── ARCHITECTURE.md  分层规则与设计约束
```

---

## 编译与烧录

1. 安装 **Keil MDK 5** + `STM32F1xx_DFP` 器件包（ARMCC 5.06）。
2. 打开 `MDK-ARM/Super_Car.uvprojx`，`Rebuild`，`Download`。
3. 需要改外设配置时，用 **STM32CubeMX** 打开 `Super_Car.ioc` 重新生成；`Core/` 与 `Drivers/` 属于生成区，手写代码放在其余目录。

---

## 关键参数

全部集中在 `Config/Inc/car_config.h`：

| 宏 | 值 | 说明 |
|---|---|---|
| `CAR_MOTOR_COMMAND_MAX` | `100` | 指令范围 `-100..100`（百分比），BSP 按此缩放到 ARR |
| `CAR_PWM_PERIOD` | `3599` | TIM1 ARR，72 MHz → 20 kHz |
| `CAR_CONTROL_PERIOD_MS` | `10` | 控制周期，编码器差分与速度换算都基于它 |
| `CAR_ENCODER_PPR` | `13` | MG310 霍尔编码器线数 |
| `CAR_ENCODER_MULTIPLIER` | `4` | TI12 模式 4 倍频 → 52 counts/电机轴圈 |
| `CAR_GEAR_RATIO` | `30` | **待实测确认** |
| `CAR_WHEEL_DIAMETER_MM` | `65.0` | **待实测确认** |

> 减速比与轮径只影响 `speed_mm_s` 和里程；`speed_pulse`（counts/s）不依赖它们，闭环先用后者。

---

## 调试（VOFA+）

- 串口：**USART2 / 115200 / 8-N-1**
- 协议：**JustFloat**，**8 通道**

| 通道 | 内容 |
|---|---|
| 0 / 1 / 2 / 3 | 左编码器 `delta` / `speed_pulse` / `speed_mm_s` / `count` |
| 4 / 5 / 6 / 7 | 右编码器 同上 |

左右对称排列，通道 0 与通道 4 直接对比即为左右差速。

---

## 串口命令

**USART2 / 115200 / 8-N-1**，每条命令以 `\n` 或 `\r\n` 结束（`\r` 被忽略）。

| 命令 | 作用 | 示例 |
|---|---|---|
| `SPD:<v>` | 左右同速，`v` 取 `-100..100` | `SPD:50` |
| `L:<v> R:<v>` | 左右差速，可只给一侧 | `L:60 R:40` |
| `O:<v>` | 开环输出（同 `SPD`，语义上用于台架） | `O:40` |
| `STOP` | 停车并清空编码器数据 | `STOP` |
| `?` | 查询：指令 / 编码器增量 / 脉冲速度 | `?` |

上电后默认 **停止**，必须发命令才动。台架自检可把 `car_config.h` 的
`CAR_STARTUP_MOTOR_TEST` 置 `1`，配合 `CAR_STARTUP_TEST_COMMAND` 设定开机占空比。

`?` 的回复形如：

```text
L:50 R:50 ENC:38,-12 SPD:3800,-1200
```

> 文本回复和 JustFloat 二进制流共用 USART2。发命令时 VOFA+ 会看到一帧乱码，
> 下一帧自动对齐；要彻底干净，把文本回复改到 USART3（已预留）。

---

## 当前进度

- [x] TIM1 四路 PWM（20 kHz）+ TB6612 四通道驱动
- [x] 电机分层抽象（BSP / Device / Service）
- [x] TIM3 / TIM4 正交编码器采样，10 ms 差分测速
- [x] USART2 调试输出，VOFA+ 8 通道
- [x] 串口接收中断（环形缓冲）+ 行命令解析
- [ ] 速度闭环（PID）
- [ ] 八路灰度循迹与状态机

---

## 已知限制

- `Device_Motor_Info_Typedef::Stop()` 转调 `Bsp_tb6612_StopAll()`，停一个轮子会停掉四个。
- `Set_Target_Speed()` / `Set_Actual_Speed()` 目前只有声明，尚未实现。
- 只有两路编码器，前左 / 前右轮速度是**推算值**，不是实测值。
- 上电默认停机，需通过串口命令或 `CAR_STARTUP_MOTOR_TEST` 才输出；首次上电请**把车架起来**。
- 命令解析用 `atoi`，非法输入按 0 处理，不做范围校验（超出 `±100` 由 Device 层限幅兜底）。
