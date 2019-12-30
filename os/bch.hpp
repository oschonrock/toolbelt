#ifndef OS_BCH_HPP
#define OS_BCH_HPP

#include <chrono>
#include <iostream>
#include <string>

namespace os::bch {
// profiling

class Timer {
public:
  Timer(std::string label_) : start{std::chrono::high_resolution_clock::now()}, label{label_} {};

  ~Timer() { print(); }

  void print() {
    finish = std::chrono::high_resolution_clock::now();
    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::duration<double>>(finish - start).count() * 1000;
    std::cout << label << "=" << elapsed_ms << "ms\n";
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start, finish;
  std::string label;
};

} // namespace os::bch


#endif // OS_BCH_HPP
