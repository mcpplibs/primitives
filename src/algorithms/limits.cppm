module;

#include <concepts>
#include <limits>
#include <type_traits>

export module mcpplibs.primitives.algorithms.limits;

import mcpplibs.primitives.underlying;

namespace mcpplibs::primitives::algorithms::details {

template <typename T>
concept has_numeric_limits =
    std::numeric_limits<std::remove_cv_t<T>>::is_specialized;

template <typename T>
consteval auto category_from_type() -> underlying::category {
  using value_type = std::remove_cv_t<T>;
  if constexpr (std_bool<value_type>) {
    return underlying::category::boolean;
  } else if constexpr (std_char<value_type>) {
    return underlying::category::character;
  } else if constexpr (std::numeric_limits<value_type>::is_integer) {
    return underlying::category::integer;
  } else {
    return underlying::category::floating;
  }
}

} // namespace mcpplibs::primitives::algorithms::details

export namespace mcpplibs::primitives::algorithms {

template <typename T> struct limits {
  using value_type = std::remove_cv_t<T>;
  using rep_type = value_type;

  static constexpr bool enabled = false;
  static constexpr bool is_specialized = false;
  static constexpr bool is_bounded = false;
  static constexpr bool is_exact = false;
  static constexpr bool is_signed = false;
  static constexpr bool is_integer = false;
  static constexpr bool is_iec559 = false;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_nan = false;
  static constexpr int digits = 0;
  static constexpr int digits10 = 0;
  static constexpr int radix = 0;
  static constexpr auto kind = static_cast<underlying::category>(-1);

  static constexpr auto min() noexcept -> value_type { return {}; }
  static constexpr auto lowest() noexcept -> value_type { return {}; }
  static constexpr auto max() noexcept -> value_type { return {}; }
  static constexpr auto epsilon() noexcept -> value_type { return {}; }
  static constexpr auto infinity() noexcept -> value_type { return {}; }
  static constexpr auto quiet_nan() noexcept -> value_type { return {}; }
};

template <typename T>
  requires details::has_numeric_limits<T>
struct limits<T> {
  using value_type = std::remove_cv_t<T>;
  using rep_type = value_type;

  static constexpr bool enabled = true;
  static constexpr bool is_specialized = true;
  static constexpr bool is_bounded = std::numeric_limits<value_type>::is_bounded;
  static constexpr bool is_exact = std::numeric_limits<value_type>::is_exact;
  static constexpr bool is_signed = std::numeric_limits<value_type>::is_signed;
  static constexpr bool is_integer = std::numeric_limits<value_type>::is_integer;
  static constexpr bool is_iec559 = std::numeric_limits<value_type>::is_iec559;
  static constexpr bool has_infinity =
      std::numeric_limits<value_type>::has_infinity;
  static constexpr bool has_quiet_nan =
      std::numeric_limits<value_type>::has_quiet_NaN;
  static constexpr int digits = std::numeric_limits<value_type>::digits;
  static constexpr int digits10 = std::numeric_limits<value_type>::digits10;
  static constexpr int radix = std::numeric_limits<value_type>::radix;
  static constexpr auto kind = details::category_from_type<value_type>();

  static constexpr auto min() noexcept -> value_type {
    return std::numeric_limits<value_type>::min();
  }

  static constexpr auto lowest() noexcept -> value_type {
    return std::numeric_limits<value_type>::lowest();
  }

  static constexpr auto max() noexcept -> value_type {
    return std::numeric_limits<value_type>::max();
  }

  static constexpr auto epsilon() noexcept -> value_type {
    return std::numeric_limits<value_type>::epsilon();
  }

  static constexpr auto infinity() noexcept -> value_type {
    return std::numeric_limits<value_type>::infinity();
  }

  static constexpr auto quiet_nan() noexcept -> value_type {
    return std::numeric_limits<value_type>::quiet_NaN();
  }
};

template <underlying_type T>
  requires (!details::has_numeric_limits<T> &&
            !std::same_as<std::remove_cv_t<T>,
                          typename underlying::traits<std::remove_cv_t<T>>::rep_type> &&
            limits<typename underlying::traits<std::remove_cv_t<T>>::rep_type>::enabled)
struct limits<T> {
  using value_type = std::remove_cv_t<T>;
  using rep_type = underlying::traits<value_type>::rep_type;
  using rep_limits = limits<rep_type>;

  static constexpr bool enabled = true;
  static constexpr bool is_specialized = rep_limits::is_specialized;
  static constexpr bool is_bounded = rep_limits::is_bounded;
  static constexpr bool is_exact = rep_limits::is_exact;
  static constexpr bool is_signed = rep_limits::is_signed;
  static constexpr bool is_integer = rep_limits::is_integer;
  static constexpr bool is_iec559 = rep_limits::is_iec559;
  static constexpr bool has_infinity = rep_limits::has_infinity;
  static constexpr bool has_quiet_nan = rep_limits::has_quiet_nan;
  static constexpr int digits = rep_limits::digits;
  static constexpr int digits10 = rep_limits::digits10;
  static constexpr int radix = rep_limits::radix;
  static constexpr auto kind = underlying::traits<value_type>::kind;

  static constexpr auto min() noexcept -> value_type {
    return underlying::traits<value_type>::from_rep(rep_limits::min());
  }

  static constexpr auto lowest() noexcept -> value_type {
    return underlying::traits<value_type>::from_rep(rep_limits::lowest());
  }

  static constexpr auto max() noexcept -> value_type {
    return underlying::traits<value_type>::from_rep(rep_limits::max());
  }

  static constexpr auto epsilon() noexcept -> value_type {
    return underlying::traits<value_type>::from_rep(rep_limits::epsilon());
  }

  static constexpr auto infinity() noexcept -> value_type {
    return underlying::traits<value_type>::from_rep(rep_limits::infinity());
  }

  static constexpr auto quiet_nan() noexcept -> value_type {
    return underlying::traits<value_type>::from_rep(rep_limits::quiet_nan());
  }
};

template <typename T>
concept limited_type = limits<std::remove_cvref_t<T>>::enabled;

template <typename T>
using limit_value_t = limits<std::remove_cvref_t<T>>::value_type;

template <limited_type T>
constexpr auto min_value() noexcept(
    noexcept(limits<std::remove_cvref_t<T>>::min())) -> limit_value_t<T> {
  return limits<std::remove_cvref_t<T>>::min();
}

template <limited_type T>
constexpr auto lowest_value() noexcept(
    noexcept(limits<std::remove_cvref_t<T>>::lowest())) -> limit_value_t<T> {
  return limits<std::remove_cvref_t<T>>::lowest();
}

template <limited_type T>
constexpr auto max_value() noexcept(
    noexcept(limits<std::remove_cvref_t<T>>::max())) -> limit_value_t<T> {
  return limits<std::remove_cvref_t<T>>::max();
}

template <limited_type T>
constexpr auto epsilon_value() noexcept(
    noexcept(limits<std::remove_cvref_t<T>>::epsilon())) -> limit_value_t<T> {
  return limits<std::remove_cvref_t<T>>::epsilon();
}

template <limited_type T>
constexpr auto infinity_value() noexcept(
    noexcept(limits<std::remove_cvref_t<T>>::infinity())) -> limit_value_t<T> {
  return limits<std::remove_cvref_t<T>>::infinity();
}

template <limited_type T>
constexpr auto quiet_nan_value() noexcept(
    noexcept(limits<std::remove_cvref_t<T>>::quiet_nan())) -> limit_value_t<T> {
  return limits<std::remove_cvref_t<T>>::quiet_nan();
}

} // namespace mcpplibs::primitives::algorithms
