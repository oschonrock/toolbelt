#include <type_traits>

namespace os::tmp {

// for "printing" typenames during debugging
template <typename... Args>
void whatis();

// detect vector vs other containers
template <typename C, typename = void>
struct has_push_back : std::false_type {};

template <typename C>
struct has_push_back<
    C, std::void_t<decltype(std::declval<C>().push_back(std::declval<typename C::value_type>()))>>
    : std::true_type {};

// is type wrapped in std::optional?
template <typename T, typename = void>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<T, std::void_t<decltype(std::declval<T>().value())>> : std::true_type {};

} // namespace os::tmp
