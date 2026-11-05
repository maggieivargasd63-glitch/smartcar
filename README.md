# 智能车 · STM32F103 + TC264 双路雷达

全国大学生智能车竞赛参赛代码。

## 组成

- `cheju/`：STM32F103 主控工程（STM32CubeMX 生成），含 GPIO、USART、OLED(I2C) 驱动
- `cheju.c` / `cheju.h`：TC264 侧双路雷达模块（字节流状态机解析）

## 双路雷达解析

- 帧格式：`帧头(0xBB/0xCC) | dL | dH | conf | xor | 0x0D`，共 6 字节
- 两路雷达共用一条 UART，靠帧头区分实例（`radar1` / `radar2`）
- 逐字节喂入状态机 `Radar_Feed_Byte()`，不阻塞，可放在串口中断里调用
- XOR 校验 + 帧尾 0x0D 双重校验；连续 N 帧一致才更新距离，防止野值跳变

## 编译

- 用 Keil MDK 打开 `cheju/MDK-ARM/cheju.uvprojx` 编译
- 外设与时钟配置见 `cheju/cheju.ioc`（CubeMX）

## 成绩

- 待补充：XXXX 年全国大学生智能车竞赛 XX 省 三等奖
