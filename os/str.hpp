#pragma once

#include "ryu/d2s.c"
#include "ryu/s2d.c"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <limits>
#include <functional>
#include <iostream>
#include <list>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace os::str {

namespace ascii {

inline constexpr bool isalpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

inline constexpr bool isdigit(char c) { return (c >= '0' && c <= '9'); }

inline constexpr bool isintegral(char c) { return isdigit(c) || (c == '-') || c == '+'; }

inline constexpr bool isnumeric(char c) {
  return isintegral(c) || c == '.' || c == ',' || c == '^' || c == '*' || c == 'e' || c == 'E';
}

inline constexpr bool isspace(char c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f';
}

inline constexpr std::string_view spacechars() { return " \t\n\r\v\f"; }

inline constexpr bool isalphanum(char c) { return isalpha(c) || isnumeric(c) || isspace(c); }

inline constexpr char tolower(char c) {
  constexpr auto offset = 'a' - 'A';
  static_assert(offset % 2 == 0); // it's 32 == 0x20 == 1 << 5  in ASCII
  return c | offset;              //  NOLINT  - ignore warnings because < 128
}

inline constexpr char toupper(char c) { return c & ~('a' - 'A'); } //  NOLINT  - ignore warnings

} // namespace ascii

inline void ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char c) { return std::isspace(c) == 0; }));
}

inline void rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](char c) { return std::isspace(c) == 0; }).base(),
          s.end());
}

inline std::string lpad(const std::string& s, size_t size) {
  return (s.size() < size) ? std::string(size - s.size(), ' ') + s : s;
}

inline std::string rpad(const std::string& s, size_t size) {
  return (s.size() < size) ? s + std::string(size - s.size(), ' ') : s;
}

inline void tolower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

inline void toupper(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
}

// clang-format off
inline void trim(std::string& s) { ltrim(s); rtrim(s); }

inline std::string ltrim_copy(std::string s) { ltrim(s); return s; }
inline std::string rtrim_copy(std::string s) { rtrim(s); return s; }
inline std::string trim_copy(std::string s) { trim(s); return s; }

inline std::string tolower_copy(std::string s) { tolower(s); return s; }
inline std::string toupper_copy(std::string s) { toupper(s); return s; }
// clang-format on

// std::string_view equivalents. different implementation, and "_copy" only because cheap
inline std::string_view ltrim(std::string_view sv,
                              std::string_view ignore_chars = ascii::spacechars()) {
  sv.remove_prefix(std::min(sv.find_first_not_of(ignore_chars), sv.size()));
  return sv;
}

inline std::string_view rtrim(std::string_view sv,
                              std::string_view ignore_chars = ascii::spacechars()) {
  auto last = sv.find_last_not_of(ignore_chars);
  if (last != std::string_view::npos) sv.remove_suffix(sv.size() - last - 1);
  return sv;
}

inline std::string_view trim(std::string_view sv,
                             std::string_view ignore_chars = ascii::spacechars()) {
  return ltrim(rtrim(sv, ignore_chars), ignore_chars);
}

// std::string_view equivalents. different implementation, and "_copy" only because cheap
template <typename UnaryPredicate>
std::string_view ltrim_if(std::string_view sv, const UnaryPredicate& ischar) {
  auto first = std::find_if(sv.begin(), sv.end(), ischar);
  if (first != sv.end()) sv.remove_prefix(first - sv.begin());
  return sv;
}

template <typename UnaryPredicate>
std::string_view rtrim_if(std::string_view sv, const UnaryPredicate& ischar) {
  auto last = find_if(sv.rbegin(), sv.rend(), ischar);
  if (last != sv.rend()) sv.remove_suffix(sv.end() - last.base());
  return sv;
}

template <typename UnaryPredicate>
std::string_view trim_if(std::string_view sv, const UnaryPredicate& ischar) {
  return ltrim_if(rtrim_if(sv, ischar), ischar);
}

inline std::optional<std::string> trim_lower(std::string_view word) {
  word = trim_if(word, ascii::isalpha);
  if (!word.empty()) {
    std::string output{word};
    std::transform(output.begin(), output.end(), output.begin(),
                   [](auto c) { return ascii::tolower(c); });
    return output; // auto wrapped in std::optional
  }
  return std::nullopt;
}

template <typename ActionCallback, typename TokenPredicate = decltype(ascii::isalpha)>
void for_each_token(std::string_view buffer, const ActionCallback& action,
                    const TokenPredicate& token_pred = ascii::isalpha) {

  const char*       begin = buffer.begin();
  const char*       curr  = begin;
  const char* const end   = buffer.end();

  while (curr != end) {
    if (!token_pred(*curr)) {
      action(std::string_view{&*begin, static_cast<std::size_t>(curr - begin)});
      begin = std::next(curr);
    }
    std::advance(curr, 1);
  }
}

template <typename T>
std::optional<T> from_chars(std::string_view sv) {
  T val;
  if (auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val); ec == std::errc()) {
    return val;
  }
  return std::nullopt;
}

#if !defined(_MSC_VER)
// full specialisation for double as a work around for clang/gcc
// not supporting the float type -> use ryu
template <>
inline std::optional<double> from_chars<double>(std::string_view sv) {
  double val;
  if (s2d_n(sv.data(), sv.length(), &val) == SUCCESS) {
    return val;
  }
  return std::nullopt;
}
#endif

template <typename T>
std::string to_chars(T val) {
  int bufsize = std::numeric_limits<T>::digits10 + 7;
  char chars[bufsize]; // NOLINT
  if(auto [p, ec] =std::to_chars(chars, chars + bufsize, val); ec == std::errc()) {
    if (p >= chars + bufsize) {
      throw std::logic_error("not enough space for null terminator");
    }
    *p = '\0';
    return std::string{chars};
  }
  throw std::logic_error("to_chars failed: not enough space");
}

#if !defined(_MSC_VER)
// full specialisation for double as a work around for clang/gcc
// not supporting the float type -> use ryu
template <>
std::string to_chars<double>(double val) {
  int bufsize = std::numeric_limits<double>::max_digits10 + 7;
  char chars[bufsize]; // NOLINT
  d2s_buffered(val, chars);
  return std::string{chars};
}
#endif

inline std::vector<std::string> split(const std::string& str, const std::string& delim) {
  std::vector<std::string> result;
  size_t                   pos   = str.find(delim);
  size_t                   start = 0;
  while (pos != std::string::npos) {
    result.emplace_back(str.begin() + start, str.begin() + pos);
    start = pos + delim.size();
    pos   = str.find(delim, start);
  }
  if (start != str.size()) result.emplace_back(str.begin() + start, str.end());
  return result;
}

template <typename InputIt>
std::ostream& join(std::ostream& stream, InputIt begin, InputIt end, const std::string& glue = ", ",
                   const std::string& term = "") {

  if (begin == end) return stream << term;

  stream << *begin;
  ++begin;

  while (begin != end) {
    stream << glue;
    stream << *begin;
    ++begin;
  }
  return stream << term;
}

template <typename InputIt>
std::string join(InputIt begin, InputIt end, const std::string& glue = ", ",
                 const std::string& term = "") {
  std::ostringstream ss;
  join(ss, begin, end, glue, term);
  return ss.str();
}

template <typename Container>
std::ostream& join(std::ostream& stream, Container cont, const std::string& glue = ", ",
                   const std::string& term = "") {
  return join(stream, std::begin(cont), std::end(cont), glue, term);
}

template <typename Container>
std::string join(Container cont, const std::string& glue = ", ", const std::string& term = "") {
  std::ostringstream ss;
  join(ss, std::begin(cont), std::end(cont), glue, term);
  return ss.str(); // can't call this on the rvalue above LWG#1203
}

} // namespace os::str
