module;
#include <type_traits>

export module mcpplibs.primitives.underlying.impl;

import mcpplibs.primitives.underlying.traits;

export
template <mcpplibs::primitives::std_underlying_type T>
struct mcpplibs::primitives::underlying::traits<T> {
  using value_type = std::remove_cv_t<T>;
  using rep_type = value_type;

  static constexpr bool enabled = true;

  static constexpr category kind = [] {
    if constexpr (std_bool<value_type>) {
      return category::boolean;
    } else if constexpr (std_char<value_type>) {
      return category::character;
    } else if constexpr (std_integer<value_type>) {
      return category::integer;
    } else {
      return category::floating;
    }
  }();

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};
