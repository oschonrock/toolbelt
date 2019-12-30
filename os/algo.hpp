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
} // namespace os
