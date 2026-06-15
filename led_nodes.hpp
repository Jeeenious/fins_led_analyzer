/*******************************************************************************
 * led_nodes.hpp — LEDAnalyzer (cv::Mat patch → color + freq + active)
 ******************************************************************************/

#pragma once

#include <chrono>
#include <cmath>
#include <deque>
#include <string>

#include <fins/node.hpp>
#include <fins/server/parameter_server.hpp>
#include <opencv2/opencv.hpp>

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

  void initialize() override {
    auto &ps = fins::param_server();

    // 尝试加载配置文件
    if (!ps.load_file(config_path_)) {
      logger->warn("LEDAnalyzer: 未找到 config/config.yaml，使用默认参数");
    }

    cfg_.saturation_thresh = ps.get<int>("color.saturation_thresh", 15);
    cfg_.bright_min        = ps.get<int>("color.bright_min", 50);

    cfg_.h_red_end     = ps.get<int>("color.h_red_end", 15);
    cfg_.h_red2_start  = ps.get<int>("color.h_red2_start", 345);
    cfg_.h_orange_end  = ps.get<int>("color.h_orange_end", 40);
    cfg_.h_yellow_end  = ps.get<int>("color.h_yellow_end", 65);
    cfg_.h_green_end   = ps.get<int>("color.h_green_end", 170);
    cfg_.h_cyan_end    = ps.get<int>("color.h_cyan_end", 195);
    cfg_.h_blue_end    = ps.get<int>("color.h_blue_end", 260);
    cfg_.h_purple_end  = ps.get<int>("color.h_purple_end", 310);
    cfg_.h_pink_end    = ps.get<int>("color.h_pink_end", 345);

    cfg_.freq_window_sec = ps.get<double>("frequency.window_sec", 5.0);

    logger->info("LEDAnalyzer 参数已加载");
  }

  void on_patch(const cv::Mat &patch, fins::AcqTime ts) {
    if (patch.empty()) return;

    cv::Scalar mean = cv::mean(patch);
    double bright = mean[0] + mean[1] + mean[2];

    double b = mean[0], g = mean[1], r = mean[2];

    // HSV 色相映射
    double mx = std::max({r, g, b});
    double mn = std::min({r, g, b});
    double delta = mx - mn;
    std::string color = "none";
    if (bright > 0) {
      if (mx < cfg_.bright_min)              color = "none";
      else if (delta < cfg_.saturation_thresh) color = "white";
      else {
        double h = 0;
        if (mx == r)      h = 60.0 * std::fmod((g - b) / delta, 6.0);
        else if (mx == g) h = 60.0 * ((b - r) / delta + 2.0);
        else if (mx == b) h = 60.0 * ((r - g) / delta + 4.0);
        if (h < 0) h += 360.0;

        if (h < cfg_.h_red_end || h >= cfg_.h_red2_start)  color = "red";
        else if (h < cfg_.h_orange_end)                     color = "orange";
        else if (h < cfg_.h_yellow_end)                     color = "yellow";
        else if (h < cfg_.h_green_end)                      color = "green";
        else if (h < cfg_.h_cyan_end)                       color = "cyan";
        else if (h < cfg_.h_blue_end)                       color = "blue";
        else if (h < cfg_.h_purple_end)                     color = "purple";
        else if (h < cfg_.h_pink_end)                       color = "pink";
        else                                                color = "red";
      }
    }

    // 自适应阈值
    bright_max_ = std::max(bright_max_, bright);
    bright_min_ = std::min(bright_min_, bright);
    double threshold = (bright_max_ + bright_min_) / 2.0;

    if (!active_ && bright > threshold)  active_ = true;
    if (active_  && bright < threshold)  active_ = false;

    double now = std::chrono::duration<double>(ts.time_since_epoch()).count();
    history_.emplace_back(now, active_);
    while (!history_.empty() && now - history_.front().t > cfg_.freq_window_sec)
      history_.pop_front();

    float freq = 0;
    int edges = 0;
    for (size_t i = 1; i < history_.size(); ++i)
      if (history_[i].active != history_[i - 1].active) ++edges;
    if (!history_.empty()) {
      double span = history_.back().t - history_.front().t;
      if (span > 0.1) freq = static_cast<float>(edges / (2.0 * span));
    }

    logger->info("LED: B={:.0f} G={:.0f} R={:.0f} bright={:.0f} color={} freq={:.1f}Hz active={}",
                 b, g, r, bright, color, freq, active_);
    send("color", color);
    send("freq", freq);
    send("active", active_);
  }

private:
  struct Config {
    int saturation_thresh = 15;
    int bright_min = 50;
    int h_red_end = 15, h_red2_start = 345;
    int h_orange_end = 40, h_yellow_end = 65;
    int h_green_end = 170, h_cyan_end = 195;
    int h_blue_end = 260, h_purple_end = 310;
    int h_pink_end = 345;
    double freq_window_sec = 5.0;
  } cfg_;

  const char *config_path_ = "config/config.yaml";

  bool active_ = false;
  double bright_min_ = 9999;
  double bright_max_ = 0;
  struct Entry { double t; bool active; Entry(double t_, bool a) : t(t_), active(a) {} };
  std::deque<Entry> history_;
};
EXPORT_NODE(LEDAnalyzer)
