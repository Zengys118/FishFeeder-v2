# FishFeeder-v2

一个基于 **STM32F103C8T6 + DS3231 + 28BYJ-48 + ULN2003** 的开源自动鱼食投喂器。

项目使用 3D 打印螺杆送料结构，通过步进电机控制每次投喂量，并使用 DS3231 实时时钟实现精确到秒的定时投喂。

目前主要针对 **粉末状 / 细颗粒鱼粮** 进行设计和测试。

---

## 功能

目前已实现：

- STM32F103C8T6 主控
- DS3231 实时时钟
- I2C 读取年月日时分秒
- RTC 手动校时
- 精确到秒的定时投喂
- 每日多次投喂
- 28BYJ-48 步进电机控制
- ULN2003 电机驱动
- 八拍半步驱动
- 投喂步数可调
- 步进速度可调
- 电机方向可调
- 投喂完成后自动释放线圈
- USART1 串口调试
- 串口调试总开关
- 每秒打印当前 RTC 时间
- Reset 后单次步进测试模式
- 3D 打印螺杆送料结构
- 可拆卸螺纹料仓

---

# 系统结构

```text
             +----------------+
             |    DS3231      |
             | Real Time Clock|
             +-------+--------+
                     |
                   I2C1
                     |
                     ▼
            +-----------------+
            | STM32F103C8T6   |
            +--------+--------+
                     |
                 GPIO IN1~IN4
                     |
                     ▼
              +------------+
              |  ULN2003   |
              +-----+------+
                    |
                    ▼
              +------------+
              | 28BYJ-48   |
              +-----+------+
                    |
                    ▼
             3D打印螺杆送料
                    |
                    ▼
                  鱼粮
```

---

# 硬件

## 主控

- STM32F103C8T6

## RTC

- DS3231

DS3231 负责保存：

- 年
- 月
- 日
- 星期
- 时
- 分
- 秒

断开主电源后，可依靠后备电池继续计时。

## 步进电机

- 28BYJ-48

当前使用八拍半步驱动。

常见情况下可近似认为：

```text
约 4096 个半步 ≈ 输出轴旋转一圈
```

实际减速比并不是严格整数，如果需要精确送料，应以实际称重标定结果为准。

## 电机驱动

- ULN2003

STM32 GPIO 只负责控制 ULN2003 输入端。

请勿直接使用 STM32 GPIO 驱动 28BYJ-48。

---

# 接线

## 28BYJ-48 / ULN2003

| STM32 | ULN2003 |
|---|---|
| PB0 | IN1 |
| PB1 | IN2 |
| PB10 | IN3 |
| PB11 | IN4 |
| GND | GND |

28BYJ-48 直接连接 ULN2003 电机接口。

---

## DS3231

| STM32 | DS3231 |
|---|---|
| PB6 | SCL |
| PB7 | SDA |
| 3.3V | VCC |
| GND | GND |

I2C 参数：

```text
I2C1
100 kHz
7-bit address
```

DS3231 地址：

```text
0x68
```

STM32 HAL 中使用：

```c
(0x68 << 1)
```

---

## USART1

| STM32 | USB-TTL |
|---|---|
| PA9 / TX | RX |
| PA10 / RX | TX |
| GND | GND |

串口参数：

```text
115200
8N1
```

即：

```text
115200 baud
8 data bits
No parity
1 stop bit
No flow control
```

---

# 软件环境

当前项目使用：

- STM32CubeMX
- STM32 HAL
- Keil MDK 5
- ARM Compiler 5
- VS Code
- Keil Assistant（可选）

当前 MCU 时钟配置：

```text
HSE = 8 MHz

8 MHz
  ↓
PLL ×9
  ↓
SYSCLK = 72 MHz
```

最终：

```text
SYSCLK = 72 MHz
HCLK   = 72 MHz
APB1   = 36 MHz
APB2   = 72 MHz
```

RTC 长期时间精度由 DS3231 提供。

---

# 项目目录

```text
FishFeeder-v2/
│
├─ Core/
│  ├─ Inc/
│  │  ├─ main.h
│  │  ├─ stepper.h
│  │  └─ ds3231.h
│  │
│  └─ Src/
│     ├─ main.c
│     ├─ stepper.c
│     └─ ds3231.c
│
├─ Drivers/
│
├─ MDK-ARM/
│
├─ sw/
│  └─ 装配体.STEP
│
├─ FishFeeder-v2.ioc
├─ README.md
└─ LICENSE
```

---

# 主要参数

项目中大部分需要调整的参数都集中在 `main.c` 顶部。

---

## 调试输出

```c
#define DEBUG_PRINT_ENABLE 1
```

含义：

```text
1 = 开启串口调试
0 = 关闭串口调试
```

正式长期运行时可以设置：

```c
#define DEBUG_PRINT_ENABLE 0
```

关闭串口不会影响：

- RTC
- 定时判断
- 步进电机
- 自动投喂

---

# 步进电机参数

## 投喂步数

```c
#define FEED_STEPS 15000
```

控制每次投喂时步进电机运行多少半步。

例如：

```text
约 4000 步  ≈ 1 圈
约 8000 步  ≈ 2 圈
约 12000 步 ≈ 3 圈
```

这里只能作为近似参考。

实际投喂量应根据鱼粮和机械结构进行标定。

---

## 步进速度

```c
#define STEPPER_DELAY_MS 2
```

表示每个半步之间延时：

```text
2 ms
```

数值越小：

```text
电机越快
```

数值越大：

```text
电机越慢
```

设置过小可能导致：

- 丢步
- 抖动
- 堵转

---

## 电机方向

```c
#define FEED_DIRECTION 1
```

当前逻辑：

```text
1 = Forward
0 = Reverse
```

不同机械安装方向可能需要调整。

---

# 步进电机调试模式

为了方便调试机械结构，可以开启：

```c
#define STEPPER_DEBUG_ENABLE 1
```

开启后：

```text
按下 Reset
    ↓
STM32重新启动
    ↓
步进电机运行 FEED_STEPS
    ↓
停止
    ↓
继续执行正常程序
```

这可以用于快速测试：

- 螺杆旋转量
- 出粮量
- 电机方向
- 是否堵转
- 机械结构是否卡住

正式使用时建议关闭：

```c
#define STEPPER_DEBUG_ENABLE 0
```

---

# DS3231 校时

第一次使用 DS3231 时，需要设置正确时间。

将：

```c
#define RTC_SET_TIME_ON_BOOT 1
```

然后设置：

```c
#define RTC_SET_YEAR    2026
#define RTC_SET_MONTH   8
#define RTC_SET_DATE    21

#define RTC_SET_HOUR    12
#define RTC_SET_MINUTE  30
#define RTC_SET_SECOND  0
```

烧录后 STM32 会将该时间写入 DS3231。

---

## 重要

校时完成以后必须改回：

```c
#define RTC_SET_TIME_ON_BOOT 0
```

然后重新烧录。

否则每次 Reset 都会重新写入固定时间。

---

# 精确到秒校准

如果需要较准确地对齐秒数，可以提前设置一个未来时间。

例如：

```text
13:00:00
```

代码：

```c
#define RTC_SET_HOUR    13
#define RTC_SET_MINUTE  0
#define RTC_SET_SECOND  0
```

程序提前烧录完成。

现实时间到：

```text
13:00:00
```

时按一下 Reset，即可较方便地完成秒级校准。

---

# C 语言时间参数注意事项

不要写：

```c
#define RTC_SET_MINUTE 08
```

因为 C 语言中以 `0` 开头的整数可能被解释为八进制。

八进制不存在 `8` 和 `9`。

错误：

```c
#define RTC_SET_HOUR   08
#define RTC_SET_MINUTE 09
```

正确：

```c
#define RTC_SET_HOUR   8
#define RTC_SET_MINUTE 9
```

打印时仍然可以显示为：

```text
08:09:00
```

---

# 定时投喂

目前支持每日两组定时投喂。

---

## 第一顿

```c
#define FEED1_ENABLE 1

#define FEED1_HOUR   6
#define FEED1_MINUTE 0
#define FEED1_SECOND 0
```

表示：

```text
每天 06:00:00
```

自动投喂一次。

---

## 第二顿

```c
#define FEED2_ENABLE 0
```

设置：

```text
0 = 禁用
1 = 启用
```

如果启用：

```c
#define FEED2_HOUR   18
#define FEED2_MINUTE 0
#define FEED2_SECOND 0
```

表示：

```text
每天 18:00:00
```

投喂一次。

---

# 定时精度

当前：

```c
#define RTC_CHECK_INTERVAL_MS 100
```

STM32 每：

```text
100 ms
```

读取一次 DS3231。

因此触发时间通常在目标秒后的：

```text
0 ~ 100 ms
```

范围内。

对于自动投喂场景已经足够。

---

# 防止同一秒重复投喂

由于：

```text
18:00:00
```

会持续完整一秒，而 MCU 每 100 ms 检查一次，因此如果不做处理，有可能同一秒触发多次。

程序使用：

```c
feed1_triggered
feed2_triggered
```

进行状态锁定。

因此每个设定时间只会执行一次投喂。

---

# 投喂量调节

项目使用：

```c
FEED_STEPS
```

控制投喂量，而不是直接使用克数。

实际出粮量会受到以下因素影响：

- 鱼粮颗粒大小
- 粉末流动性
- 鱼粮密度
- 湿度
- 含油量
- 螺杆直径
- 螺距
- 料仓出口
- 进料窗口
- 机械间隙

因此建议进行实际标定。

---

# 推荐标定方式

例如：

```c
#define FEED_STEPS 10000
```

连续投喂 10 次。

称量总重量。

假设：

```text
10 次 = 1.20 g
```

则单次平均：

```text
1.20 / 10 = 0.12 g
```

之后根据目标重量调整 `FEED_STEPS`。

---

# 串口调试

开启：

```c
#define DEBUG_PRINT_ENABLE 1
```

串口可以看到类似：

```text
PROGRAM START
DS3231 OK
2026-08-21 12:00:00
2026-08-21 12:00:01
2026-08-21 12:00:02
```

触发投喂时：

```text
FEED1 TRIGGER 06:00:00
FEED START
FEED END
```

---

# 机械结构

送料结构主要包括：

```text
料仓
 ↓
料仓出口
 ↓
螺杆进料区域
 ↓
螺杆
 ↓
出粮口
```

步进电机驱动螺杆旋转，将鱼粮沿轴向推出。

---

# 可拆卸螺纹料仓

料仓与底座之间采用 3D 打印螺纹连接。

优点：

- 方便加粮
- 方便拆卸
- 方便清理
- 方便更换料仓
- 方便修改不同容量

由于 FDM 打印存在误差，螺纹需要根据实际打印机适当预留间隙。

---

# 粉末鱼粮注意事项

粉末状鱼粮并不一定具有理想流动性。

实际可能出现：

```text
架桥
挂壁
结块
下料不畅
螺杆吃不到料
```

典型现象：

```text
电机正常转
螺杆正常转
但是没有鱼粮进入螺杆
```

这时继续增加：

```c
FEED_STEPS
```

通常没有意义。

问题主要发生在：

```text
料仓 → 螺杆
```

而不是：

```text
螺杆 → 出粮口
```

---

# 机械结构优化方向

可以尝试：

- 增大料仓出口
- 增大螺杆进料窗口
- 增大漏斗坡度
- 减少料仓底部水平面
- 增加搅料杆
- 增加随螺杆转动的小拨片
- 优化粉末防架桥结构

---

# 编译

使用 Keil 打开：

```text
MDK-ARM/FishFeeder-v2.uvprojx
```

即可编译。

如果手动创建：

```text
stepper.c
ds3231.c
```

必须手动加入 Keil Project Group。

仅仅放入：

```text
Core/Src/
```

不会自动加入编译。

否则可能出现：

```text
Undefined symbol Stepper_Init
Undefined symbol Stepper_Forward
Undefined symbol DS3231_ReadTime
```

---

# 串口烧录

STM32F103 可通过内部 ROM Bootloader 使用 USART1 烧录。

例如使用：

```text
FlyMCU
```

进入 Bootloader：

```text
BOOT0 = 1
Reset
```

烧录完成后：

```text
BOOT0 = 0
Reset
```

即可运行用户程序。

---

# 常见问题

## 电机只抖不转

检查：

```text
PB0  → IN1
PB1  → IN2
PB10 → IN3
PB11 → IN4
```

以及电机供电。

---

## 电机方向反了

修改：

```c
#define FEED_DIRECTION 0
```

或：

```c
#define FEED_DIRECTION 1
```

不需要重新调整接线。

---

## DS3231 找不到

检查：

```text
PB6 → SCL
PB7 → SDA
```

同时确认：

```text
VCC
GND
```

正常空闲时：

```text
SCL ≈ 3.3V
SDA ≈ 3.3V
```

如果 SDA 与 SCL 接反，DS3231 无法正常通信。

---

## RTC 显示 2000-01-01

例如：

```text
2000-01-01 07:43:11
```

说明 DS3231 已经能够正常走时，但尚未校准。

将：

```c
#define RTC_SET_TIME_ON_BOOT 1
```

写入正确时间即可。

---

## 每次 Reset 时间都会恢复

检查：

```c
#define RTC_SET_TIME_ON_BOOT
```

校准完成后必须设置：

```c
#define RTC_SET_TIME_ON_BOOT 0
```

---

## 编译出现 invalid octal digit

例如写了：

```c
#define RTC_SET_MINUTE 08
```

改成：

```c
#define RTC_SET_MINUTE 8
```

即可。

---

## 螺杆转但不出粮

优先检查：

- 料仓是否架桥
- 出口是否太小
- 漏斗角度是否太缓
- 螺杆进料窗口是否太小
- 鱼粮是否受潮结块

这通常不是步数问题。

---

# 推荐测试流程

第一次使用建议：

```text
1. 不放鱼粮测试步进电机
2. 检查旋转方向
3. 检查螺杆是否卡顿
4. 测试 DS3231
5. 校准 RTC
6. 设置几分钟后的测试投喂时间
7. 验证自动触发
8. 少量加入鱼粮
9. 标定 FEED_STEPS
10. 再进行长期测试
```

---

# 后续计划

- 优化粉末鱼粮料仓
- 优化螺杆进料窗口
- 增加防架桥结构
- 增加更多每日投喂时间
- 串口动态校准 RTC
- 串口动态修改投喂时间
- Flash 保存配置
- 非阻塞步进电机驱动
- 堵转检测
- 低粮量检测
- OLED 显示
- 按键设置
- Wi-Fi 联网
- 手机端配置

---

# License

项目计划使用：

```text
MIT License
```

本仓库中的 STM32 HAL / CMSIS 等第三方代码仍遵循其各自原始许可证。

---

# Disclaimer

本项目主要用于：

- DIY
- 学习
- 实验
- 自动投喂研究

自动投喂设备可能因为：

- 鱼粮堵塞
- 机械卡死
- 电源异常
- 软件配置错误
- 料仓架桥

导致少喂或过量投喂。

正式长期无人值守运行前，请进行充分测试。

---

# Contribution

欢迎提交：

- Issue
- Pull Request
- 固件优化
- 新的料仓设计
- 螺杆优化
- 不同鱼粮测试数据
- Bug 修复
- 新功能建议