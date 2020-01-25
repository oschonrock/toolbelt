#pragma once

#include "flat_hash_map/bytell_hash_map.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <iomanip>
#include <limits>
#include <list>
#include <numeric>
#include <vector>

using std::size_t;

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

// basic summary statistics

template <typename T>
struct stats {
  size_t n = 0;

  T min = std::numeric_limits<T>::max();
  T max = std::numeric_limits<T>::min();
  T sum = 0;

  ska::bytell_hash_map<T, size_t> dist{};

  [[nodiscard]] size_t uniq_n() const noexcept { return dist.size(); }
  [[nodiscard]] auto   mean() const noexcept { return sum * 1.0 / n; }

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

// sorting parallel vectors: https://codereview.stackexchange.com/questions/235764

template <typename T>
void swap(size_t i, size_t j, std::vector<T>& v) {
  std::swap(v[i], v[j]);
}

// adapted from https://en.cppreference.com/w/cpp/algorithm/set_intersection
// assumes sorted data
template <class InputIt1, class InputIt2>
size_t count_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
  size_t count = 0;
  while (first1 != last1 && first2 != last2) {
    if (*first1 < *first2) {
      ++first1;
    } else {
      if (!(*first2 < *first1)) {
        ++count;
        ++first1;
      }
      ++first2;
    }
  }
  return count;
}

template <typename Comp, typename Vec, typename... Vecs>
void parallel_sort(const Comp& comp, Vec& keyvec, Vecs&... vecs) {
  #ifndef NDEBUG
  (assert(keyvec.size() == vecs.size()), ...);
  #endif
  std::vector<size_t> index(keyvec.size());
  std::iota(index.begin(), index.end(), 0);
  std::sort(index.begin(), index.end(),
            [&](size_t a, size_t b) { return comp(keyvec[a], keyvec[b]); });

  for (size_t i = 0; i < index.size(); i++) {
    if (index[i] != i) {
      (swap(index[i], i, keyvec), ..., swap(index[i], i, vecs));
      std::swap(index[index[i]], index[i]);
    }
  }
}

// template <typename T>
// void test(const std::vector<T>& vec, const std::vector<T>& res) {
//   assert(vec == res);
// }

// int main() {
//   using value_t = int;
//   using vec_t   = std::vector<value_t>;

//   vec_t order{1, 0, 3, 2};
//   vec_t v1{100, 200, 300, 400};
//   vec_t v2{100, 200, 300, 400};
//   vec_t v3{400, 200, 3000, 4000};
//   vec_t v4{500, 200, 360, 400};

//   parallel_sort(std::less<>(), order, v1, v2, v3, v4);

//   test(v1, vec_t{200, 100, 400, 300});
//   test(v2, vec_t{200, 100, 400, 300});
//   test(v3, vec_t{200, 400, 4000, 3000});
//   test(v4, vec_t{200, 500, 400, 360});
// }

} // namespace os::algo
