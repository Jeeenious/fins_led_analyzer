/*******************************************************************************
 * config.h — LED 检测参数
 ******************************************************************************/
#pragma once

#include <string>

namespace config {

// ROI: 左上角 (x,y) 和宽高，用于裁剪 LED 区域
inline constexpr int ROI_X      = 0;
inline constexpr int ROI_Y      = 0;
inline constexpr int ROI_W      = 64;
inline constexpr int ROI_H      = 64;

// 亮度阈值: mean > THRESH_ON 视为亮，mean < THRESH_OFF 视为灭
inline constexpr int THRESH_ON  = 128;
inline constexpr int THRESH_OFF = 64;

// 颜色判定: 饱和度 delta < SATURATION_THRESH 视为白色，亮度 mx < BRIGHT_MIN_COLOR 视为无光
inline constexpr int SATURATION_THRESH  = 15;
inline constexpr int BRIGHT_MIN_COLOR   = 50;

// 频率计算: 最近 N 秒内的亮灭周期
inline constexpr double FREQ_WINDOW_SEC = 5.0;

}  // namespace config
