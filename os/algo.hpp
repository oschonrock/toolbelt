#pragma once

#include "flat_hash_map/bytell_hash_map.hpp"
#include <algorithm>
#include <functional>
#include <iomanip>
#include <limits>
#include <list>

// not really generic templates, but for now

namespace os::algo {

template <template <typename...> class Container, typename T, typename UnaryPredicate>
void move_append_if(Container<T>& origin, Container<T>& destination, UnaryPredicate&& predicate) {

  auto part_it = std::stable_partition(origin.begin(), origin.end(),
                                       [&](auto&& elem) { return !predicate(elem); });
  std::move(part_it, origin.end(), std::back_inserter(destination));
  origin.erase(part_it, origin.end());
}

template <typename T, typename UnaryPredicate>
void move_append_if(std::list<T>& origin, std::list<T>& destination, UnaryPredicate&& predicate,
                    std::optional<int> move_max = {}) {

  for (auto iter = std::find_if(origin.begin(), origin.end(), predicate);
       iter != origin.end() && (!move_max.has_value() || *move_max > 0);
       iter = std::find_if(iter, origin.end(), predicate)) {
    auto iter_next = std::next(iter);
    destination.splice(destination.end(), origin, iter);
    iter = iter_next;
    --(*move_max);
  }
}

template <typename T>
struct stats {
  std::size_t n = 0;

  T min = std::numeric_limits<T>::max();
  T max = std::numeric_limits<T>::min();
  T sum = 0;

  ska::bytell_hash_map<T, std::size_t> dist{};

  [[nodiscard]] std::size_t uniq_n() const noexcept { return dist.size(); }
  [[nodiscard]] auto        mean() const noexcept { return sum * 1.0 / n; }

  void record(T a) {
    ++n;
    sum += a;
    if (a < min) min = a;
    if (a > max) max = a;
    ++dist[a];
  }

  friend std::ostream& operator<<(std::ostream& stream, const stats& st) {
    return stream << std::setprecision(std::numeric_limits<T>::max_digits10)
                  << "n       = " << st.n << '\n'
                  << "uniq_n  = " << st.uniq_n() << '\n'
                  << "min     = " << st.min << '\n'
                  << "max     = " << st.max << '\n'
                  << "sum     = " << st.sum << '\n'
                  << "mean    = " << st.mean() << '\n';
  }
};
} // namespace os::algo
