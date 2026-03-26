module;

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>

export module mcpplibs.primitives.algorithms.hash;

import mcpplibs.primitives.underlying;

namespace mcpplibs::primitives::algorithms::details {

template <typename T>
concept std_hashable = requires(std::remove_cv_t<T> const &value) {
  {
    std::hash<std::remove_cv_t<T>>{}(value)
  } -> std::convertible_to<std::size_t>;
};

} // namespace mcpplibs::primitives::algorithms::details

export namespace mcpplibs::primitives::algorithms {

template <typename T> struct hash {
  using value_type = std::remove_cv_t<T>;
  using result_type = std::size_t;

  static constexpr bool enabled = false;

  auto operator()(value_type const &) const noexcept -> result_type {
    return result_type{};
  }
};

template <typename T>
using hash_result_t = hash<std::remove_cvref_t<T>>::result_type;

template <typename T>
concept hashable = hash<std::remove_cvref_t<T>>::enabled;

template <typename T>
  requires details::std_hashable<T>
struct hash<T> {
  using value_type = std::remove_cv_t<T>;
  using result_type = std::size_t;

  static constexpr bool enabled = true;

  auto operator()(value_type const &value) const noexcept(
      noexcept(std::hash<value_type>{}(value))) -> result_type {
    return std::hash<value_type>{}(value);
  }
};

template <underlying_type T>
  requires (!details::std_hashable<T> &&
            hash<typename underlying::traits<std::remove_cv_t<T>>::rep_type>::enabled)
struct hash<T> {
  using value_type = std::remove_cv_t<T>;
  using rep_type = underlying::traits<value_type>::rep_type;
  using result_type = hash_result_t<rep_type>;

  static constexpr bool enabled = true;

  auto operator()(value_type const &value) const noexcept(
      noexcept(hash<rep_type>{}(underlying::traits<value_type>::to_rep(value))))
      -> result_type {
    return hash<rep_type>{}(underlying::traits<value_type>::to_rep(value));
  }
};

template <hashable T>
auto hash_value(T const &value) noexcept(
    noexcept(hash<std::remove_cvref_t<T>>{}(value))) -> hash_result_t<T> {
  return hash<std::remove_cvref_t<T>>{}(value);
}

} // namespace mcpplibs::primitives::algorithms
