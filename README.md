# fins_led_analyzer — LED 分析

LEDAnalyzer 节点：裁好 patch → 颜色 + 频率 + 亮灭。

## 端口

| 端口 | 方向 | 类型 | 内容 |
|------|------|------|------|
| patch | in | cv::Mat | LED 区域裁剪图 |
| color | out | string | red/green/blue/yellow/white/none |
| freq | out | float | 闪烁频率 Hz |
| active | out | bool | 当前亮灭 |

## 管线

```
RegionClassifier.light → LEDAnalyzer → color + freq + active
```
