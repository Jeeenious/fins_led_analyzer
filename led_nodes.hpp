/*******************************************************************************
 * led_nodes.hpp — LEDAnalyzer (cv::Mat patch → color + freq + active)
 ******************************************************************************/

#pragma once

#include <chrono>
#include <deque>
#include <string>

#include <fins/node.hpp>
#include <opencv2/opencv.hpp>

#include "config.h"

class LEDAnalyzer : public fins::Node {
public:
  void define() override {
    set_name("LEDAnalyzer");
    set_description("LED 指示灯分析: 颜色 + 闪烁频率 + 亮灭状态");
    set_category("Inspect");
    register_input<cv::Mat>("roi", &LEDAnalyzer::on_patch);
    register_output<std::string>("color");
    register_output<float>("freq");
    register_output<bool>("active");
  }

  void on_patch(const cv::Mat &patch, fins::AcqTime ts) {
    if (patch.empty()) return;

    cv::Scalar mean = cv::mean(patch);
    double bright = mean[0] + mean[1] + mean[2];

    double b = mean[0], g = mean[1], r = mean[2];
    std::string color = "none";
    if (bright > config::THRESH_ON) {
      if (r > g + b)          color = "red";
      else if (g > r + b)     color = "green";
      else if (b > r + g)     color = "blue";
      else if (r > 150 && g > 120) color = "yellow";
      else                    color = "white";
    }

    if (!active_ && bright > config::THRESH_ON)  active_ = true;
    if (active_  && bright < config::THRESH_OFF) active_ = false;

    double now = std::chrono::duration<double>(ts.time_since_epoch()).count();
    history_.emplace_back(now, active_);
    while (!history_.empty() && now - history_.front().t > config::FREQ_WINDOW_SEC)
      history_.pop_front();

    float freq = 0;
    int edges = 0;
    for (size_t i = 1; i < history_.size(); ++i)
      if (history_[i].active != history_[i - 1].active) ++edges;
    if (!history_.empty()) {
      double span = history_.back().t - history_.front().t;
      if (span > 0.1) freq = static_cast<float>(edges / (2.0 * span));
    }

    send("color", color);
    send("freq", freq);
    send("active", active_);
  }

private:
  bool active_ = false;
  struct Entry { double t; bool active; Entry(double t_, bool a) : t(t_), active(a) {} };
  std::deque<Entry> history_;
};
EXPORT_NODE(LEDAnalyzer)
