#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>
#include <list>
#include <numeric>
#include <vector>

namespace os::algo {

template <typename T, typename TIter = decltype(std::begin(std::declval<T>())),
          typename = decltype(std::end(std::declval<T>()))>
constexpr auto enumerate(T&& iterable) {
  struct iterator {
    std::size_t i;
    TIter       iter;
    bool        operator!=(const iterator& other) const { return iter != other.iter; }
    void        operator++() {
      ++i;
      ++iter;
    }
    auto operator*() const { return std::tie(i, *iter); }
  };
  struct iterable_wrapper {
    T    iterable;
    auto begin() { return iterator{0, std::begin(iterable)}; }
    auto end() { return iterator{0, std::end(iterable)}; }
  };
  return iterable_wrapper{std::forward<T>(iterable)};
}

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

// adapted from https://en.cppreference.com/w/cpp/algorithm/set_intersection
// assumes sorted data
template <class InputIt1, class InputIt2>
std::size_t count_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {
  std::size_t count = 0;
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

template <class ContainerA, class ContainerB>
std::size_t count_intersection(ContainerA a, ContainerB b) {
  return count_intersection(a.begin(), a.end(), b.begin(), b.end());
}

template <class Container>
Container intersection(Container a, Container b) {
  auto c = Container{};
  std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_insert_iterator<Container>(c));
  return c;
}

// sorting parallel vectors: https://codereview.stackexchange.com/questions/235764

template <typename T>
void swap(std::size_t i, std::size_t j, std::vector<T>& v) {
  std::swap(v[i], v[j]);
}

template <typename Comp, typename Vec, typename... Vecs>
void parallel_sort(const Comp& comp, Vec& keyvec, Vecs&... vecs) {
#ifndef NDEBUG
  (assert(keyvec.size() == vecs.size()), ...);
#endif
  std::vector<std::size_t> index(keyvec.size());
  std::iota(index.begin(), index.end(), 0);
  std::sort(index.begin(), index.end(),
            [&](std::size_t a, std::size_t b) { return comp(keyvec[a], keyvec[b]); });

  for (auto&& [i, idx]: enumerate(index)) {
    if (idx != i) {
      (swap(idx, i, keyvec), ..., swap(idx, i, vecs));
      std::swap(index[idx], idx);
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
