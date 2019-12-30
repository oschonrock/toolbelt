#ifndef OS_STR_HPP
#define OS_STR_HPP

#include <algorithm>
#include <iostream>

namespace os::debug {

// some debug swiss army knives

#ifndef DEBUG
#define DEBUG 0
#endif

#define DB(x) do { if (DEBUG) db_impl(__FILE__, __LINE__, #x, x); } while (0)

template <typename Arg>
void db_impl(const char* file, int line, const char* varname, Arg value)
{
  std::cerr << file << ":" << line << ": warning: ";
  std::cerr << varname << " = " << value << '\n';
}


#define DBP(...) do { if (DEBUG) dbp_impl(__FILE__, __LINE__, __VA_ARGS__); } while (0)

template <typename... Args>
void dbp_impl(const char* file, int line, Args&&... args)
{
  std::cerr << file << ":" << line << ": warning: ";
  (std::cerr << ... << std::forward<Args>(args)) << '\n';
}
} // namespace os::debug

//example usage

// void f(int num, std::string msg) {
//   // do sth
//   DBP("in f() with: ", num, " ", msg, " in: ", __PRETTY_FUNCTION__);
// }

// int g(int n) {
//   return n + 2;
// }

// int main() {

//   int x = 6;
//   DB(x + 1);

//   DB(g(3));

//   // use like:
//   DBP("hello world! ", 10, ", ", 42);

//   f(3, "hello");
//   return 0;
// }


// needs to be last, causes problems
// debug printing of containers

template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& container) {
  stream << '{';
  char comma[3] = {'\0', ' ', '\0'};
  for (const T& elem: container) {
    stream << comma << elem;
    comma[0] = ',';
  }
  return stream << '}';
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::set<T>& container) {
  stream << '[';
  char comma[3] = {'\0', ' ', '\0'};
  for (const T& elem: container) {
    stream << comma << elem;
    comma[0] = ',';
  }
  return stream << ']';
}


#endif // OS_STR_HPP
