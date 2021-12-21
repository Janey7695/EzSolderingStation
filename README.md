# EzSolderingStation

#  ![license: GPLv3 (shields.io)](https://img.shields.io/badge/license-GPLv3-brightgreen)![version: 1.0 (shields.io)](https://img.shields.io/badge/version-1.0-brightgreen)

一个简易的T12控制台

学习自https://github.com/wagiminator/ATmega-Soldering-Station

移植到esp32上

修改了自己设计的UI

# UI说明



![Screenshot1](/home/janey/Documents/PlatformIO/Projects/EzSolderingStation/Screenshot1.png)

1. 状态指示标志

   - `SLP`:休眠状态
   - `WK`:可正常工作状态
   - `HT：加热状态`
   - `!：发生错误，可能是没有插烙铁`

2. pwm指示条

   用来看当前输出占空比是多少

3. 烙铁类型（烙铁号）

   当前选取的烙铁型号

4. 预设的温度值

   这里没有直接的用数字表示预设的温度值，而是通过进度条的形式

   - 最小值跟最大值都可以在代码中设定
   - 一共20个步进

5. 当前测量的烙铁头热电偶温度

# 实现的功能

- 自认为好看的UI

- 最高最低温可设定（固件固化）
- 温度固定最多步进20次（为了跟UI的步进条匹配）
  - 设置最高最低值，会自动计算步进值
- 烙铁热插拔检测



# 待实现

- [ ] 添加EEROM，实现配置掉电保存
- [ ] 利用ESP32进行联网（？好像没什么用）
- [ ] 能进行人机交互，直接通过按键修改某些常参数，避免重复烧写固件
- [ ] 设计一个外壳
- [ ] 硬件集成（现在是散件）



# BUG

- 无法正常进入休眠或关机模式