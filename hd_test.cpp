#include <iostream>
#include <memory>

#include "os/debug.hpp"

struct Dummy {
  int16_t a      = 0x1111;
  int32_t b      = 0x22222222;
  int32_t c      = 0x33333333;
  void*   end    = (void*)0xffffffffffffffff; // NOLINT end of earth
};

int main() {
  auto v1 = std::vector<int>{};
  v1.push_back(1);
  v1.push_back(2);
  v1.push_back(3);
  v1.push_back(4);
  v1.push_back(5);
  DBH(v1);
  std::cout << '\n';

  auto i1 = int{0x12345678}; // 4 bytes int, 4-byte aligned
  std::cout << os::hd(i1) << '\n';

  auto pi = 22/7.0; // in .text (ie code) segment => non-aligned
  std::cout << os::hd(pi) << '\n';

  auto sv1 = std::string_view{"1234567890"}; // in .text (ie code) segment => non-aligned
  std::cout << os::hd(sv1) << '\n';

  auto sv2 = std::string_view{"This is a much longer string view onto a string literal"};
  std::cout << os::hd(sv2) << '\n'; // starts after sv1 with '\0' gap

  auto i2 = short{0x1234};  // 2 bytes int, 4-byte aligned
  std::cout << os::hd(i2) << '\n';

  auto str1 = std::string{"123456789012345"}; // SBO on stack => 16-byte aligned??
  std::cout << os::hd(str1) << '\n';

  auto str2 = std::string{"1234567890123456"}; // too big for SBO => on heap => 16Byte aligned
  std::cout << os::hd(str2) << '\n';

  auto str3 = std::string{"short big cap"}; // short string with enforce big capcity
  str3.reserve(30);
  DBH(str3);
  std::cout << '\n';


  auto d1 = Dummy{}; // on stack 8-byte aligned with padding gaps
  std::cout << os::hd(d1) << '\n';

  auto d2 = std::make_unique<Dummy>(); // on heap 16byte aligned with padding gaps
  std::cout << os::hd(d2.get(), sizeof(*d2)) << '\n';

  auto d3 = std::make_unique<Dummy[]>(4); // array on heap 8-byte aligned: odd/even NOLINT
  std::cout << os::hd(d3.get(), 4 * sizeof(d3[0])) << '\n';
}
