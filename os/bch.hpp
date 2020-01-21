#pragma once

#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>

namespace os::bch {
// profiling

using clk        = std::chrono::high_resolution_clock;
using time_point = std::chrono::time_point<clk>;
using std::chrono::duration;
using std::chrono::duration_cast;

class Timer {
public:
  explicit Timer(std::string label_) : start_{clk::now()}, label{std::move(label_)} {}

  Timer(const Timer& other) = default;
  Timer& operator=(const Timer& other) = default;
  Timer(Timer&& other)                 = default;
  Timer& operator=(Timer&& other) = default;

  ~Timer() { print(); }

  void print() {
    finish_         = clk::now();
    auto elapsed    = finish_ - start_;
    auto elapsed_s  = duration_cast<duration<double>>(elapsed).count();
    auto elapsed_ms = elapsed_s * 1000;
    char buf[50]{0};                                                // NOLINT
    std::sprintf(buf, "%-20s %10.4f ms", label.data(), elapsed_ms); // NOLINT
    std::cerr << buf << '\n';                                       // NOLINT
  }

private:
  time_point  start_;
  time_point  finish_;
  std::string label;
};

} // namespace os::bch
