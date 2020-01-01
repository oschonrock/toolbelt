#ifndef OS_STR_HPP
#define OS_STR_HPP

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <functional>
#include <list>
#include <set>
#include <vector>

namespace os::str {

namespace ascii {

inline constexpr bool isalpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

inline constexpr bool isnumeric(char c) {
  return (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.' || c == ',' || c == '^' ||
         c == '*' || c == 'e' || c == 'E';
}

inline constexpr bool isspace(char c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f';
}

inline constexpr std::string_view spacechars() { return " \t\n\r\v\f"; }

inline constexpr bool isalphanum(char c) { return isalpha(c) || isnumeric(c) || isspace(c); }

inline constexpr char tolower(char c) {
  constexpr auto offset = 'a' - 'A';
  static_assert(offset % 2 == 0); // it's 32 == 0x20 == 1 << 5  in ASCII
  return c | offset;              //  NOLINT
}

} // namespace ascii

inline void ltrim(std::string& s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

inline void rtrim(std::string& s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(),
          s.end());
}

inline std::string pad_left(std::string const& str, size_t s) {
  return (str.size() < s) ? std::string(s - str.size(), ' ') + str : str;
}

inline std::string pad_right(std::string const& str, size_t s) {
  return (str.size() < s) ? str + std::string(s - str.size(), ' ') : str;
}

inline void strtolower(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

inline void strtoupper(std::string& s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
}

// clang-format off
inline void trim(std::string& s) { ltrim(s); rtrim(s); }

inline std::string ltrim_copy(std::string s) { ltrim(s); return s; }
inline std::string rtrim_copy(std::string s) { rtrim(s); return s; }
inline std::string trim_copy(std::string s) { trim(s); return s; }

inline std::string strtolower_copy(std::string s) { strtolower(s); return s; }
inline std::string strtoupper_copy(std::string s) { strtoupper(s); return s; }
// clang-format on

// std::string_view equivalents. different implementation, and "_copy" only because cheap
inline std::string_view ltrim(std::string_view sv, std::string_view ignore_chars = ascii::spacechars()) {
  sv.remove_prefix(std::min(sv.find_first_not_of(ignore_chars), sv.size()));
  return sv;
}

inline std::string_view rtrim(std::string_view sv, std::string_view ignore_chars = ascii::spacechars()) {
  auto last = sv.find_last_not_of(ignore_chars);
  if (last != std::string_view::npos) sv.remove_suffix(sv.size() - last - 1);
  return sv;
}

inline std::string_view trim(std::string_view sv, std::string_view ignore_chars = ascii::spacechars()) {
  return ltrim(rtrim(sv, ignore_chars), ignore_chars);
}

// std::string_view equivalents. different implementation, and "_copy" only because cheap
template <typename UnaryPredicate>
std::string_view ltrim_if(std::string_view sv, UnaryPredicate ischar) {
  auto first = std::find_if(sv.begin(), sv.end(), ischar);
  if (first != sv.end()) sv.remove_prefix(first - sv.begin());
  return sv;
}

template <typename UnaryPredicate>
std::string_view rtrim_if(std::string_view sv, UnaryPredicate ischar) {
  auto last = find_if(sv.rbegin(), sv.rend(), ischar);
  if (last != sv.rend()) sv.remove_suffix(sv.end() - last.base());
  return sv;
}

template <typename UnaryPredicate>
std::string_view trim_if(std::string_view sv,UnaryPredicate ischar) {
  return ltrim_if(rtrim_if(sv, ischar), ischar);
}

inline std::optional<std::string> trim_lower(std::string_view word) {
  word = trim_if(word, ascii::isalpha);
  if (!word.empty()) {
    std::string output{word};
    std::transform(output.begin(), output.end(), output.begin(),
                   [](auto c) { return ascii::tolower(c); });
    return std::optional<std::string>{output};
  }
  return std::nullopt;
}

template <typename ActionFunction>
void proc_words(std::string_view buffer, ActionFunction&& action) {

  const char*       begin = buffer.begin();
  const char*       curr  = begin;
  const char* const end   = buffer.end();

  while (curr != end) {
    if (!ascii::isalpha(*curr)) {
      auto maybe_word =
          trim_lower(std::string_view{&*begin, static_cast<std::size_t>(curr - begin)});
      if (maybe_word) action(*maybe_word);
      begin = std::next(curr);
    }
    std::advance(curr, 1);
  }
}

std::vector<std::string> split(const std::string& str, const std::string& delim) {
  std::vector<std::string> result;
  size_t pos   = str.find(delim);
  size_t start = 0;
  while (pos != std::string::npos) {
    result.push_back(std::string(str.begin() + start, str.begin() + pos));
    start = pos + delim.size();
    pos   = str.find(delim, start);
  }
  if (start != str.size()) result.push_back(std::string(str.begin() + start, str.end()));
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

#endif // OS_STR_HPP
