#ifndef OS_DEBUG_HPP
#define OS_DEBUG_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <map>

// hacky. needs to be before str.hpp
template <typename T, typename U>
std::ostream& operator<<(std::ostream& stream, const std::pair<T,U>& pair) {
  return stream << '(' << pair.first << ", " << pair.second << ")";
}

#include "os/str.hpp"

namespace os::debug {

// some debug swiss army knives

#ifndef DEBUG
#define DEBUG 1  // not working conveniently from cmd line yet
#endif

#define DB(x) do { if (DEBUG) os::debug::db_impl(__FILE__, __LINE__, #x, x); } while (0)

template <typename Arg>
void db_impl(const char* file, int line, const char* varname, Arg value)
{
  std::cout << file << ":" << line << ": warning: ";
  std::cout << varname << " = " << value << '\n';
}


#define DBP(...) do { if (DEBUG)  os::debug::dbp_impl(__FILE__, __LINE__, __VA_ARGS__); } while (0)

template <typename... Args>
void dbp_impl(const char* file, int line, Args&&... args)
{
  std::cout << file << ":" << line << ": warning: ";
  (std::cout << ... << std::forward<Args>(args)) << '\n';
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


// debug printing of containers


template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::vector<T>& container) {
  return os::str::join(stream << '[', container, ", ", "]\n");
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::set<T>& container) {
  return os::str::join(stream << '[', container, ", ", "]\n");
}

template <typename T>
std::ostream& operator<<(std::ostream& stream, const std::list<T>& container) {
  return os::str::join(stream << '[', container, ", ", "]\n");
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& stream, const std::map<T,U>& container) {
  return os::str::join(stream << '[', container, ", ", "]\n");
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& stream, const std::unordered_map<T,U>& container) {
  return os::str::join(stream << '[', container, ", ", "]\n");
}




#endif // OS_DEBUG_HPP
