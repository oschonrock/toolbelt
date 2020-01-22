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

// some debug swiss army knives
// example usage

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

#ifndef DEBUG
#define DEBUG 1 // not working conveniently from cmd line yet
#endif

#define DB(x)                                                                                      \
  do {                                                                                             \
    if (DEBUG) os::db_impl(__FILE__, __LINE__, #x, x);                                             \
  } while (0)

template <typename Arg>
void db_impl(const char* file, int line, const char* varname, Arg value) {
  std::cout << file << ":" << line << ": warning: ";
  std::cout << varname << " = " << value << '\n';
}

#define DBP(...)                                                                                   \
  do {                                                                                             \
    if (DEBUG) os::dbp_impl(__FILE__, __LINE__, __VA_ARGS__);                                      \
  } while (0)

template <typename... Args>
void dbp_impl(const char* file, int line, Args&&... args) {
  std::cout << file << ":" << line << ": warning: ";
  (std::cout << ... << std::forward<Args>(args)) << '\n';
}

// utility: keeps states of an ostream, restores on destruction
template <typename T>
struct ostream_state {
  explicit ostream_state(std::basic_ostream<T>& stream)
      : stream_{stream}, flags_{stream.flags()}, fill_{stream.fill()}, width_{stream.width()} {}

  ostream_state(const ostream_state& other) = delete;
  ostream_state& operator=(const ostream_state& other) = delete;

  ostream_state(ostream_state&& other) = delete;
  ostream_state& operator=(ostream_state&& other) = delete;

  ~ostream_state() {
    stream_.flags(flags_);
    stream_.fill(fill_);
    stream_.width(width_);
  }

private:
  std::basic_ostream<T>&                    stream_;
  std::ios_base::fmtflags                   flags_;
  typename std::basic_ostream<T>::char_type fill_;
  std::streamsize                           width_;
};

inline std::ostream& print_adr(std::ostream& os, const void* adr) {
  auto state = ostream_state(os);
  return os << std::setw(19) << std::setfill(' ') << adr; // NOLINT
}

inline std::ostream& print_byte(std::ostream& os, const unsigned char* ptr) {
  auto state = ostream_state(os);
  return os << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned>(*ptr) << ' ';
}

inline std::ostream& print_ascii(std::ostream& os, const unsigned char* ptr) {
  auto state = ostream_state(os);
  return os << std::setw(2) << std::setfill('0') << std::hex << static_cast<unsigned>(*ptr) << ' ';
}

inline std::ostream& hex_dump(std::ostream& os, const void* buffer, std::size_t bufsize) {
  if (buffer == nullptr || bufsize == 0) return os;

  constexpr std::size_t maxline{16};

  // buffer for printable characters
  unsigned char  pbuf[maxline + 1]; // NOLINT
  unsigned char* pbuf_curr{pbuf};   // NOLINT

  const unsigned char* buf{reinterpret_cast<const unsigned char*>(buffer)}; // NOLINT

  // pre-buffer area: floor(nearest maxline)
  size_t      offset    = reinterpret_cast<size_t>(buffer) % maxline; // NOLINT
  std::size_t linecount = maxline;
  if (offset > 0) {
    const void* prebuf = buf - offset; // NOLINT
    print_adr(os, prebuf) << ": ";
    while (offset--) { // underflow OK NOLINT
      os << "-- ";
      *pbuf_curr++ = '.'; // NOLINT
      --linecount;
    }
  }

  // main buffer area
  while (bufsize) {                                    // NOLINT
    if (pbuf_curr == pbuf) print_adr(os, buf) << ": "; // NOLINT
    print_byte(os, buf);
    *pbuf_curr++ = std::isprint(*buf) ? *buf : '.'; // NOLINT
    if (--linecount == 0) {
      *pbuf_curr++ = '\0';         // NOLINT
      os << " | " << pbuf << '\n'; // NOLINT
      pbuf_curr = pbuf;            // NOLINT
      linecount = std::min(maxline, bufsize);
    }
    --bufsize;
    ++buf; // NOLINT
  }

  // post buffer area: finish incomplete line
  if (pbuf_curr != pbuf) {                                                               // NOLINT
    for (*pbuf_curr++ = '\0'; pbuf_curr != &pbuf[maxline + 1]; ++pbuf_curr) os << "-- "; // NOLINT
    os << " | " << pbuf << '\n';                                                         // NOLINT
  }
  return os;
}

struct hd {
  const void* buffer;
  std::size_t bufsize;

  hd(const void* buf, std::size_t bufsz) : buffer{buf}, bufsize{bufsz} {}

  // doesn't really work. can't detect the appropriate size
  // template <typename T>
  // explicit hd(const T& buf, std::size_t bufsz = sizeof(T)) : buffer{buf}, bufsize{bufsz} {}

  friend std::ostream& operator<<(std::ostream& out, const hd& hd) {
    return hex_dump(out, hd.buffer, hd.bufsize);
  }
};

} // namespace os

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
