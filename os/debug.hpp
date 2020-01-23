#pragma once

#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <vector>

// hacky. needs to be before str.hpp
template <typename T, typename U>
std::ostream& operator<<(std::ostream& stream, const std::pair<T, U>& pair) {
  return stream << '(' << pair.first << ", " << pair.second << ")";
}

#include "os/str.hpp"

namespace os {

// utility: keeps states of an ostream, restores on destruction
template <typename T>
struct ostream_state {
  explicit ostream_state(std::basic_ostream<T>& stream)
      : stream_{stream}, flags_{stream.flags()}, fill_{stream.fill()} {}

  ostream_state(const ostream_state& other) = delete;
  ostream_state& operator=(const ostream_state& other) = delete;

  ostream_state(ostream_state&& other) = delete;
  ostream_state& operator=(ostream_state&& other) = delete;

  ~ostream_state() {
    stream_.flags(flags_);
    stream_.fill(fill_);
  }

private:
  std::basic_ostream<T>&                    stream_;
  std::ios_base::fmtflags                   flags_;
  typename std::basic_ostream<T>::char_type fill_;
};

namespace detail {
inline void print_adr(std::ostream& os, const void* adr) {
  os << std::setw(19) << std::setfill(' ') << adr; // NOLINT
}

inline void print_fill_advance(std::ostream& os, const std::byte*& buf, std::size_t cnt,
                               const std::string& str) {
  while (cnt-- != 0U) {
    ++buf; // NOLINT
    os << str;
  }
}

inline void print_hex(std::ostream& os, const std::byte* buf, std::size_t linesize, std::size_t pre,
                      std::size_t post) {
  print_fill_advance(os, buf, pre, "-- ");
  {
    os << std::setfill('0') << std::hex;
    auto cnt = linesize - pre - post;
    while (cnt-- != 0U) os << std::setw(2) << static_cast<unsigned>(*buf++) << ' '; // NOLINT
  }
  print_fill_advance(os, buf, post, "-- ");
}

inline void print_ascii(std::ostream& os, const std::byte* buf, std::size_t linesize,
                        std::size_t pre, std::size_t post) {
  print_fill_advance(os, buf, pre, ".");
  auto cnt = linesize - pre - post;
  while (cnt-- != 0U) {
    os << (std::isprint(static_cast<unsigned char>(*buf)) != 0 ? static_cast<char>(*buf) : '.');
    ++buf; // NOLINT
  }
  print_fill_advance(os, buf, post, ".");
}
} // namespace detail

inline std::ostream& hex_dump(std::ostream& os, const std::byte* buffer, std::size_t bufsize) {
  if (buffer == nullptr || bufsize == 0) return os;

  constexpr std::size_t linesize{16};
  const std::byte*      buf{buffer};
  std::size_t           pre =
      reinterpret_cast<std::size_t>(buffer) % linesize; // Size of pre-buffer area  NOLINT
  bufsize += pre;
  buf -= pre; // NOLINT

  auto state = ostream_state{os}; // save stream setting and restore at end of scope
  while (bufsize != 0U) {
    std::size_t post = bufsize < linesize ? linesize - bufsize : 0;

    detail::print_adr(os, buf);
    os << ": ";
    detail::print_hex(os, buf, linesize, pre, post);
    os << " | ";
    detail::print_ascii(os, buf, linesize, pre, post);
    os << "\n";

    buf += linesize; // NOLINT
    bufsize -= linesize - post;
    pre = 0;
  }
  return os;
}

struct hd {
  const void* buffer;
  std::size_t bufsize;

  hd(const void* buf, std::size_t bufsz) : buffer{buf}, bufsize{bufsz} {}

  template <typename T>
  explicit hd(const T& buf) : buffer{&buf}, bufsize{sizeof(T)} {}

  template <> // note that string_view DOES NOT access sv[size()]
  explicit hd(const std::string_view& buf) : buffer{buf.data()}, bufsize{buf.size()} {}

  template <> // note that string DOES access str[size()]
  explicit hd(const std::string& buf) : buffer{buf.data()}, bufsize{buf.size() + 1} {}

  friend std::ostream& operator<<(std::ostream& out, const hd& hd) {
    return hex_dump(out, reinterpret_cast<const std::byte*>(hd.buffer), hd.bufsize); // NOLINT
  }
};

#ifndef DEBUG
#define DEBUG 1 // not working conveniently from cmd line yet
#endif

#define DB(x)                                                                                      \
  do {                                                                                             \
    if (DEBUG) os::db_impl(__FILE__, __LINE__, #x, x);                                             \
  } while (0)

template <typename Arg>
void db_impl(const char* file, int line, const char* varname, Arg& value) {
  std::cerr << file << ":" << line << ": warning: ";
  std::cerr << varname << " = " << value << '\n';
}

#define DBH(x)                                                                                     \
  do {                                                                                             \
    if (DEBUG) os::db_impl(__FILE__, __LINE__, #x, x);                                             \
  } while (0)

template <typename Arg>
void dbh_impl(const char* file, int line, const char* varname, Arg& value) {
  std::cerr << file << ":" << line << ": warning: ";
  std::cerr << varname << " = " << value << "  hexdump:\n";
  std::cerr << hd(value);
}

#define DBP(...)                                                                                   \
  do {                                                                                             \
    if (DEBUG) os::dbp_impl(__FILE__, __LINE__, __VA_ARGS__);                                      \
  } while (0)

template <typename... Args>
void dbp_impl(const char* file, int line, Args&&... args) {
  std::cerr << file << ":" << line << ": warning: ";
  (std::cerr << ... << std::forward<Args>(args)) << '\n';
}

} // namespace os

// quick debug example usage

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
std::ostream& operator<<(std::ostream& stream, const std::map<T, U>& container) {
  return os::str::join(stream << '[', container, ", ", "]\n");
}

template <typename T, typename U>
std::ostream& operator<<(std::ostream& stream, const std::unordered_map<T, U>& container) {
  return os::str::join(stream << '[', container, ", ", "]\n");
}
