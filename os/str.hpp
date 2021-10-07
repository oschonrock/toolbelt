#pragma once

#include "date/date.h"
#include "fast_float/fast_float.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
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

template <typename...>
constexpr std::false_type always_false{};

// faster and stricter alternative to std::stoi/stol/stof etc if you have a char*.
// float and double specialisations benefit if len is passsed (saves strlen)
template <typename NumericType>
inline NumericType parse(const char* str, std::size_t len = ~0UL) { // NOLINT complexity
  if constexpr (std::is_same_v<NumericType, int> || std::is_same_v<NumericType, unsigned>) {
      long long_val = parse<long>(str, len);
    if (long_val < std::numeric_limits<NumericType>::min() ||
        long_val > std::numeric_limits<NumericType>::max())
      throw std::range_error("out of range");
    return static_cast<NumericType>(long_val);
  } else {
    NumericType val;
    if constexpr (std::is_same_v<NumericType, double> || std::is_same_v<NumericType, float>) {
      if (len == ~0UL) len = strlen(str); // prefer to have this passed in for performance
      const auto* str_end = str + len;
      auto [ptr, ec]      = fast_float::from_chars(str, str_end, val);
      if (ec != std::errc() || ptr != str_end)
        throw std::domain_error("fast_float::from_chars couldn't parse double: " +
                                std::string(str));
      return val;
    } else {
      char* end; // NOLINT
      errno = 0;

      if constexpr (std::is_same_v<NumericType, long>)
        val = std::strtol(str, &end, 0);
      else if constexpr (std::is_same_v<NumericType, long long>)
        val = std::strtoll(str, &end, 0);
      else if constexpr (std::is_same_v<NumericType, long double>)
        val = std::strtold(str, &end);
      else
        static_assert(always_false<NumericType>, "don't know how to parse this type");

      if (end == str || *end != '\0' || errno == ERANGE) throw std::domain_error("failed to parse");
      return val;
    }
  }
}

// very high performance minimalistic integer parsing
// adapted from fmt::detail
template <typename ReturnType>
constexpr ReturnType parse_nonnegative_int(const char* begin, const char* end,
                                           ReturnType error_value) noexcept {

  assert(begin != end && '0' <= *begin && *begin <= '9');
  std::uint64_t value = 0;
  std::uint64_t prev  = 0;
  const char*   p     = begin;
  do {
    prev  = value;
    value = value * 10 + std::uint64_t(*p - '0');
    ++p;
  } while (p != end && '0' <= *p && *p <= '9');
  auto num_digits = p - begin;
  if (num_digits <= std::numeric_limits<ReturnType>::digits10)
    return static_cast<ReturnType>(value);
  // Check for overflow. Will never happen here
  const auto max = static_cast<std::uint64_t>(std::numeric_limits<ReturnType>::max());
  return num_digits == std::numeric_limits<ReturnType>::digits10 + 1 &&
                 prev * 10ULL + std::uint64_t(p[-1] - '0') <= max
             ? static_cast<ReturnType>(value)
             : error_value;
}


// must set a utf8 locale before calling this
inline std::size_t mb_strlen(const std::string_view s) {
  std::size_t result = 0;
  const char* ptr    = s.data();
  const char* end    = ptr + s.size();
  std::mblen(nullptr, 0); // reset the conversion state. NOLINT not thread safe
  while (ptr < end) {
    int next = static_cast<int>(
        std::mblen(ptr, static_cast<std::size_t>(end - ptr))); // NOLINT not thread safe
    if (next == -1) throw std::runtime_error("mb_strlen(): conversion error");
    ptr += next;
    ++result;
  }
  return result;
}

inline void ltrim(std::string& s, std::string_view delims = " \v\t\n\r") {
  s.erase(0, s.find_first_not_of(delims));
}

inline void rtrim(std::string& s, std::string_view delims = " \v\t\n\r") {
  s.erase(s.find_last_not_of(delims) + 1); // npos wraps to zero
}

// clang-format off
inline void trim(std::string& s, std::string_view delims = " \v\t\n\r") { ltrim(s, delims); rtrim(s, delims); }

inline std::string ltrim_copy(std::string s, std::string_view delims = " \v\t\n\r") { ltrim(s, delims); return s; }
inline std::string rtrim_copy(std::string s, std::string_view delims = " \v\t\n\r") { rtrim(s, delims); return s; }
inline std::string trim_copy(std::string s, std::string_view delims = " \v\t\n\r") { trim(s, delims); return s; }

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
  if (first != sv.end()) sv.remove_prefix(static_cast<std::size_t>(first - sv.begin()));
  return sv;
}

template <typename UnaryPredicate>
std::string_view rtrim_if(std::string_view sv, const UnaryPredicate& ischar) {
  auto last = find_if(sv.rbegin(), sv.rend(), ischar);
  if (last != sv.rend()) sv.remove_suffix(static_cast<std::size_t>(sv.end() - last.base()));
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

inline std::vector<std::string> explode(const std::string& delims, const std::string& s) {
  std::vector<std::string> pieces;
  auto                     start = 0UL;
  auto                     end   = s.find_first_of(delims);
  while (end != std::string::npos) {
    pieces.emplace_back(s.substr(start, end - start));
    start = end + delims.size();
    end   = s.find_first_of(delims, start);
  }
  pieces.emplace_back(s.substr(start));
  return pieces;
}

inline std::vector<std::string_view> explode_sv(const std::string_view& delims,
                                                const std::string_view& sv) {
  std::vector<std::string_view> pieces;
  auto                          start = 0UL;
  auto                          end   = sv.find_first_of(delims);
  while (end != std::string::npos) {
    pieces.emplace_back(sv.substr(start, end - start));
    start = end + delims.size();
    end   = sv.find_first_of(delims, start);
  }
  pieces.emplace_back(sv.substr(start));
  return pieces;
}

inline void replace_all(std::string& subject, const std::string_view& search,
                        const std::string_view& replace) {
  size_t pos = subject.find(search);
  while (pos != std::string::npos) {
    subject.replace(pos, search.size(), replace);
    pos = subject.find(search, pos + replace.size());
  }
}

inline std::string replace_all_copy(std::string subject, const std::string_view& search,
                                    const std::string_view& replace) {
  replace_all(subject, search, replace);
  return subject;
}

inline bool contains(std::string_view needle, std::string_view s) {
  return s.find(needle) != std::string::npos;
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

template <typename T>
std::string stringify(T t) {
  std::stringstream s;
  s << t;
  return s.str();
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
