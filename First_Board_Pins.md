# STM32F103C8T6 四驱巡线车引脚分配表

> 更新日期：2026-10-23
> 主控：STM32F103C8T6
> 驱动：两块 TB6612FNG，分别驱动前组和后组电机
> 控制：左右差速，四路独立 PWM
> 编码器：当前推荐配置左右各一路硬件正交编码器

## 一、整体资源规划

| 功能 | 外设 | MCU 资源 |
|---|---|---|
| 四路电机 PWM | TIM1 CH1-CH4 | PA8、PA9、PA10、PA11 |
| 左侧编码器 | TIM3 Encoder | PA6、PA7 |
| 右侧编码器 | TIM4 Encoder | PB6、PB7 |
| 硬件 I2C | I2C1 Remap | PB8、PB9 |
| 调试串口 | USART2 | PA2、PA3 |
| 预留通信串口 | USART3 | PB10、PB11 |
| 八路灰度模块 | GPIO | PA0、PA1、PA4、PA5 |
| 调试下载 | SWD | PA13、PA14 |

### 编码器配置说明

当前采用“一侧一个编码器”的方案：

- 左侧选择左后轮 `RL` 编码器；
- 右侧选择右后轮 `RR` 编码器；
- 前后同侧电机使用相同的差速控制命令；
- 前左、前右编码器接口暂不接入硬件闭环。

如果后续要求四个电机分别闭环，当前 F103C8T6 的硬件定时器资源不够，需要增加软件解码或更换资源更多的 MCU。

> `PWMA/PWMB`、`AIN1/AIN2`、`BIN1/BIN2` 是 TB6612 驱动板输入接口信号。
> `E1A/E1B`、`E2A/E2B` 是电机编码器接口信号，不是 TB6612FNG 芯片本体引脚。
> 实际接线时还要以驱动板丝印和原理图为准。

## 二、按车轮位置分配

### 2.1 前组 TB6612FNG

前组驱动板负责前左 `FL` 和前右 `FR`。

| 车轮位置 | 驱动板接口 | MCU 引脚 | MCU 外设 | CubeMX 配置 | 说明 |
|---|---|---|---|---|---|
| 前左 FL | `PWMA` | PA8 | TIM1_CH1 | PWM Generation CH1 | 前左电机速度 |
| 前左 FL | `AIN1` | PB12 | GPIO_Output | Output Push-Pull，初始低 | 前左方向控制 |
| 前左 FL | `AIN2` | PB13 | GPIO_Output | Output Push-Pull，初始低 | 前左方向控制 |
| 前左 FL | `E1A` | 暂不分配 | — | — | 前左编码器 A 相，暂不接入 |
| 前左 FL | `E1B` | 暂不分配 | — | — | 前左编码器 B 相，暂不接入 |
| 前右 FR | `PWMB` | PA9 | TIM1_CH2 | PWM Generation CH2 | 前右电机速度 |
| 前右 FR | `BIN1` | PB14 | GPIO_Output | Output Push-Pull，初始低 | 前右方向控制 |
| 前右 FR | `BIN2` | PB15 | GPIO_Output | Output Push-Pull，初始低 | 前右方向控制 |
| 前右 FR | `E2A` | 暂不分配 | — | — | 前右编码器 A 相，暂不接入 |
| 前右 FR | `E2B` | 暂不分配 | — | — | 前右编码器 B 相，暂不接入 |

### 2.2 后组 TB6612FNG

后组驱动板负责左后 `RL` 和右后 `RR`，并作为当前左右两侧速度反馈的编码器采样轮。

| 车轮位置 | 驱动板接口 | MCU 引脚 | MCU 外设 | CubeMX 配置 | 说明 |
|---|---|---|---|---|---|
| 左后 RL | `PWMA` | PA10 | TIM1_CH3 | PWM Generation CH3 | 左侧后电机速度 |
| 左后 RL | `AIN1` | PB0 | GPIO_Output | Output Push-Pull，初始低 | 左侧后电机方向 |
| 左后 RL | `AIN2` | PB1 | GPIO_Output | Output Push-Pull，初始低 | 左侧后电机方向 |
| 左后 RL | `E1A` | PA6 | TIM3_CH1 | TIM3 Encoder CH1 | 左侧编码器 A 相 |
| 左后 RL | `E1B` | PA7 | TIM3_CH2 | TIM3 Encoder CH2 | 左侧编码器 B 相 |
| 右后 RR | `PWMB` | PA11 | TIM1_CH4 | PWM Generation CH4 | 右侧后电机速度 |
| 右后 RR | `BIN1` | PB5 | GPIO_Output | Output Push-Pull，初始低 | 右侧后电机方向 |
| 右后 RR | `BIN2` | PA12 | GPIO_Output | Output Push-Pull，初始低 | 右侧后电机方向 |
| 右后 RR | `E2A` | PB6 | TIM4_CH1 | TIM4 Encoder CH1 | 右侧编码器 A 相 |
| 右后 RR | `E2B` | PB7 | TIM4_CH2 | TIM4 Encoder CH2 | 右侧编码器 B 相 |

## 三、TB6612FNG 公共控制信号

| 信号 | 推荐连接 | 配置说明 |
|---|---|---|
| `VM` | 电机电源 | 根据 MG310 额定电压连接 |
| `VCC` | 3.3 V | TB6612 逻辑电源 |
| `GND` | 主控与电机电源共地 | 必须共地 |
| `STBY` | 接 3.3 V，或两块板共用一个 GPIO | 必须为有效高电平才能输出 |
| `PWMA/PWMB` | 接 TIM1 四路 PWM | 控制电机有效电压 |
| `AIN1/AIN2`、`BIN1/BIN2` | 接对应 GPIO | 控制正反转 |

`STBY` 如果直接接 3.3 V，软件无法单独关闭驱动器；如果希望实现故障保护，建议两块驱动板的 `STBY` 共用一个 GPIO。

## 四、CubeMX 配置

### 4.1 TIM1：四路 PWM

| 参数 | 设置 |
|---|---|
| Clock Source | Internal Clock |
| Channel 1-4 | PWM Generation CH1-CH4 |
| Prescaler | 0 |
| Counter Period | 3599 |
| Counter Mode | Up |
| Clock Division | No Division |
| Auto-reload preload | Enable |
| PWM Mode | PWM mode 1 |
| Pulse 初值 | 0 |
| Polarity | High |
| One Pulse Mode | Disable |

在 72 MHz TIM1 时钟下，`PSC=0`、`ARR=3599` 对应约 20 kHz PWM。

### 4.2 TIM3：左后轮编码器

在 Pinout 中选择：

```text
TIM3 -> Encoder Mode
PA6 -> TIM3_CH1
PA7 -> TIM3_CH2
```

参数建议：

| 参数 | 设置 |
|---|---|
| Encoder Mode | TI1 and TI2 |
| Prescaler | 0 |
| Counter Period | 65535 |
| Counter Mode | Up |
| Clock Division | DIV1 |
| IC1/IC2 Selection | Direct TI |
| IC1/IC2 Prescaler | DIV1 |
| IC1/IC2 Polarity | Rising |
| IC1/IC2 Filter | 4，若信号很干净可改为 0 |
| NVIC | 暂不启用 |

### 4.3 TIM4：右后轮编码器

在 Pinout 中选择：

```text
TIM4 -> Encoder Mode
PB6 -> TIM4_CH1
PB7 -> TIM4_CH2
```

TIM4 参数与 TIM3 完全相同。

### 4.4 I2C1：重映射到 PB8/PB9

由于 PB6/PB7 已经给 TIM4 编码器使用，I2C1 改为重映射：

```text
I2C1 -> Remap
SCL -> PB8
SDA -> PB9
```

GPIO 配置为：

```text
Alternate Function Open-Drain
```

外接 I2C 模块前确认模块是否自带上拉电阻，并确认上拉电压为 3.3 V。

### 4.5 编码器启动

CubeMX 重新生成代码后，在初始化阶段启动编码器：

```c
HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
```

编码器速度环可以每 10 ms 读取一次计数器增量：

```c
int16_t left_delta;
int16_t right_delta;

uint16_t left_now = __HAL_TIM_GET_COUNTER(&htim3);
uint16_t right_now = __HAL_TIM_GET_COUNTER(&htim4);

left_delta = (int16_t)(left_now - left_last);
right_delta = (int16_t)(right_now - right_last);

left_last = left_now;
right_last = right_now;
```

## 五、串口

### USART2：调试串口

| 信号 | MCU 引脚 | CubeMX 配置 |
|---|---|---|
| `USART2_TX` | PA2 | Asynchronous，115200 8-N-1 |
| `USART2_RX` | PA3 | Asynchronous，115200 8-N-1 |

### USART3：预留通信串口

| 信号 | MCU 引脚 | CubeMX 配置 |
|---|---|---|
| `USART3_TX` | PB10 | Asynchronous，115200 8-N-1 |
| `USART3_RX` | PB11 | Asynchronous，115200 8-N-1 |

## 六、八路灰度循迹模块

灰度模块保持原引脚分配：

| 功能 | 驱动板接口 | MCU 引脚 | CubeMX 配置 |
|---|---|---|---|
| OUT | `OUT` | PA5 | GPIO_Input |
| AD0 | `AD0` | PA4 | GPIO_Output，初始低 |
| AD1 | `AD1` | PA0 | GPIO_Output，初始低 |
| AD2 | `AD2` | PA1 | GPIO_Output，初始低 |

## 七、未分配资源

以下资源暂不使用：

- TIM2；
- 前左 `FL` 编码器 `E1A/E1B`；
- 前右 `FR` 编码器 `E2A/E2B`；
- MPU6050、OLED、按键、蜂鸣器；
- PC13-PC15、PB2-PB4 等空闲 GPIO。

PA13/PA14 保留给 SWD 调试，CubeMX 中选择 `Serial Wire` 并关闭 JTAG。
