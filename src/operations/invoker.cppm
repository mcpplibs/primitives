module;

#include <atomic>
#include <compare>
#include <concepts>
#include <expected>
#include <optional>
#include <type_traits>
#include <utility>

export module mcpplibs.primitives.operations.invoker;

import mcpplibs.primitives.algorithms.limits;
import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.policy.traits;


namespace mcpplibs::primitives::operations::runtime::details {

template <typename CommonRep>
constexpr auto make_error(policy::error::kind kind, char const *reason,
                          std::optional<CommonRep> lhs = std::nullopt,
                          std::optional<CommonRep> rhs = std::nullopt)
    -> policy::value::decision<CommonRep> {
  policy::value::decision<CommonRep> out{};
  out.has_value = false;
  out.error.code = kind;
  out.error.reason = reason;
  out.error.lhs_value = lhs;
  out.error.rhs_value = rhs;
  return out;
}

template <typename T>
constexpr auto checked_add(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs + rhs);
    return out;
  } else if constexpr (std::is_unsigned_v<T>) {
    auto const maxv = algorithms::max_value<T>();
    if (lhs > maxv - rhs) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked addition overflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs + rhs);
    return out;
  } else {
    auto const maxv = algorithms::max_value<T>();
    auto const minv = algorithms::min_value<T>();
    if ((rhs > 0) && (lhs > maxv - rhs)) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked addition overflow", lhs, rhs);
    }
    if ((rhs < 0) && (lhs < minv - rhs)) {
      return make_error<T>(policy::error::kind::underflow,
                           "checked addition underflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs + rhs);
    return out;
  }
}

template <typename T>
constexpr auto checked_sub(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs - rhs);
    return out;
  } else if constexpr (std::is_unsigned_v<T>) {
    if (lhs < rhs) {
      return make_error<T>(policy::error::kind::underflow,
                           "checked subtraction underflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs - rhs);
    return out;
  } else {
    auto const maxv = algorithms::max_value<T>();
    auto const minv = algorithms::min_value<T>();
    if ((rhs < 0) && (lhs > maxv + rhs)) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked subtraction overflow", lhs, rhs);
    }
    if ((rhs > 0) && (lhs < minv + rhs)) {
      return make_error<T>(policy::error::kind::underflow,
                           "checked subtraction underflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs - rhs);
    return out;
  }
}

template <typename T>
constexpr auto checked_mul(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs * rhs);
    return out;
  } else {
    if (lhs == T{} || rhs == T{}) {
      policy::value::decision<T> out{};
      out.has_value = true;
      out.value = T{};
      return out;
    }

    if constexpr (std::is_unsigned_v<T>) {
      auto const maxv = algorithms::max_value<T>();
      if (lhs > maxv / rhs) {
        return make_error<T>(policy::error::kind::overflow,
                             "checked multiplication overflow", lhs, rhs);
      }
      policy::value::decision<T> out{};
      out.has_value = true;
      out.value = static_cast<T>(lhs * rhs);
      return out;
    } else {
      auto const maxv = algorithms::max_value<T>();
      auto const minv = algorithms::min_value<T>();

      if (lhs > 0) {
        if (rhs > 0) {
          if (lhs > maxv / rhs) {
            return make_error<T>(policy::error::kind::overflow,
                                 "checked multiplication overflow", lhs, rhs);
          }
        } else {
          if (rhs < minv / lhs) {
            return make_error<T>(policy::error::kind::underflow,
                                 "checked multiplication underflow", lhs, rhs);
          }
        }
      } else {
        if (rhs > 0) {
          if (lhs < minv / rhs) {
            return make_error<T>(policy::error::kind::underflow,
                                 "checked multiplication underflow", lhs, rhs);
          }
        } else {
          if (lhs != 0 && rhs < maxv / lhs) {
            return make_error<T>(policy::error::kind::overflow,
                                 "checked multiplication overflow", lhs, rhs);
          }
        }
      }

      policy::value::decision<T> out{};
      out.has_value = true;
      out.value = static_cast<T>(lhs * rhs);
      return out;
    }
  }
}

template <typename T>
constexpr auto checked_div(T lhs, T rhs) -> policy::value::decision<T> {
  if (rhs == T{}) {
    return make_error<T>(policy::error::kind::divide_by_zero,
                         "checked division by zero", lhs, rhs);
  }

  if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
    auto const minv = algorithms::min_value<T>();
    if (lhs == minv && rhs == static_cast<T>(-1)) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked division overflow", lhs, rhs);
    }
  }

  if constexpr (requires { lhs / rhs; }) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs / rhs);
    return out;
  }

  return make_error<T>(
      policy::error::kind::unspecified,
      "checked division not supported for negotiated common type", lhs, rhs);
}

template <typename T>
constexpr auto checked_mod(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T>) {
    if constexpr (requires { lhs % rhs; }) {
      policy::value::decision<T> out{};
      out.has_value = true;
      out.value = static_cast<T>(lhs % rhs);
      return out;
    }
    return make_error<T>(
        policy::error::kind::unspecified,
        "checked modulus not supported for negotiated common type", lhs, rhs);
  } else {
    if (rhs == T{}) {
      return make_error<T>(policy::error::kind::divide_by_zero,
                           "checked modulus by zero", lhs, rhs);
    }

    if constexpr (std::is_signed_v<T>) {
      auto const minv = algorithms::min_value<T>();
      if (lhs == minv && rhs == static_cast<T>(-1)) {
        return make_error<T>(policy::error::kind::overflow,
                             "checked modulus overflow", lhs, rhs);
      }
    }

    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs % rhs);
    return out;
  }
}

template <typename T>
constexpr auto checked_unary_plus(T lhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (requires { +lhs; }) {
    out.has_value = true;
    out.value = static_cast<T>(+lhs);
    return out;
  }
  return make_error<T>(policy::error::kind::unspecified,
                       "checked unary plus not supported for negotiated "
                       "common type");
}

template <typename T>
constexpr auto checked_unary_minus(T lhs) -> policy::value::decision<T> {
  if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
    auto const minv = algorithms::min_value<T>();
    if (lhs == minv) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked unary minus overflow", lhs);
    }
  }

  policy::value::decision<T> out{};
  if constexpr (requires { -lhs; }) {
    out.has_value = true;
    out.value = static_cast<T>(-lhs);
    return out;
  }
  return make_error<T>(policy::error::kind::unspecified,
                       "checked unary minus not supported for negotiated "
                       "common type");
}

template <typename T>
constexpr auto checked_shift_left(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::same_as<T, bool>) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "checked left shift not supported for negotiated common type", lhs,
        rhs);
  } else {
    using unsigned_t = std::make_unsigned_t<T>;
    constexpr auto bit_width = algorithms::limits<unsigned_t>::digits;

    if constexpr (std::is_signed_v<T>) {
      if (rhs < T{}) {
        return make_error<T>(policy::error::kind::domain_error,
                             "checked left shift negative count", lhs, rhs);
      }
    }

    auto const shift = static_cast<unsigned long long>(rhs);
    if (shift >= static_cast<unsigned long long>(bit_width)) {
      return make_error<T>(policy::error::kind::domain_error,
                           "checked left shift count out of range", lhs, rhs);
    }

    if constexpr (std::is_signed_v<T>) {
      if (lhs < T{}) {
        return make_error<T>(policy::error::kind::domain_error,
                             "checked left shift negative lhs", lhs, rhs);
      }

      auto const maxv = algorithms::max_value<T>();
      if (lhs > static_cast<T>(maxv >> shift)) {
        return make_error<T>(policy::error::kind::overflow,
                             "checked left shift overflow", lhs, rhs);
      }
    }

    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs << shift);
    return out;
  }
}

template <typename T>
constexpr auto checked_shift_right(T lhs, T rhs)
    -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::same_as<T, bool>) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "checked right shift not supported for negotiated common type", lhs,
        rhs);
  } else {
    using unsigned_t = std::make_unsigned_t<T>;
    constexpr auto bit_width = algorithms::limits<unsigned_t>::digits;

    if constexpr (std::is_signed_v<T>) {
      if (rhs < T{}) {
        return make_error<T>(policy::error::kind::domain_error,
                             "checked right shift negative count", lhs, rhs);
      }
    }

    auto const shift = static_cast<unsigned long long>(rhs);
    if (shift >= static_cast<unsigned long long>(bit_width)) {
      return make_error<T>(policy::error::kind::domain_error,
                           "checked right shift count out of range", lhs, rhs);
    }

    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs >> shift);
    return out;
  }
}

template <typename T>
constexpr auto checked_bit_and(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (std::integral<T>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs & rhs);
    return out;
  }

  return make_error<T>(
      policy::error::kind::unspecified,
      "checked bitwise and not supported for negotiated common type", lhs, rhs);
}

template <typename T>
constexpr auto checked_bit_or(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (std::integral<T>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs | rhs);
    return out;
  }

  return make_error<T>(
      policy::error::kind::unspecified,
      "checked bitwise or not supported for negotiated common type", lhs, rhs);
}

template <typename T>
constexpr auto checked_bit_xor(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (std::integral<T>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs ^ rhs);
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "checked bitwise xor not supported for negotiated "
                       "common type",
                       lhs, rhs);
}

template <typename T>
constexpr auto checked_bit_not(T lhs) -> policy::value::decision<T> {
  if constexpr (std::integral<T>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(~lhs);
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "checked bitwise not supported for negotiated "
                       "common type",
                       lhs);
}

template <typename T>
constexpr auto checked_inc(T lhs) -> policy::value::decision<T> {
  return checked_add(lhs, static_cast<T>(1));
}

template <typename T>
constexpr auto checked_dec(T lhs) -> policy::value::decision<T> {
  return checked_sub(lhs, static_cast<T>(1));
}

template <typename T>
constexpr auto compare_equal(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (requires { lhs == rhs; }) {
    out.has_value = true;
    out.value = T{lhs == rhs};
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "comparison equality not supported for negotiated "
                       "common type");
}

template <typename T>
constexpr auto compare_not_equal(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (requires { lhs != rhs; }) {
    out.has_value = true;
    out.value = T{lhs != rhs};
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "comparison inequality not supported for negotiated "
                       "common type");
}

template <typename T>
constexpr auto compare_three_way(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (!(requires { T{0}; T{1}; T{2}; T{3}; })) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "three-way comparison codes are not representable for common type");
  }

  if constexpr (std::same_as<std::remove_cv_t<T>, bool>) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "three-way comparison is not representable for bool common type");
  } else if constexpr (requires { lhs <=> rhs; }) {
    auto const cmp = lhs <=> rhs;
    out.has_value = true;

    if (cmp < 0) {
      out.value = T{0};
      return out;
    }
    if (cmp > 0) {
      out.value = T{2};
      return out;
    }

    if constexpr (std::same_as<std::remove_cvref_t<decltype(cmp)>,
                               std::partial_ordering>) {
      if (cmp == std::partial_ordering::unordered) {
        out.value = T{3};
        return out;
      }
    }

    out.value = T{1};
    return out;
  } else if constexpr (requires { lhs < rhs; lhs > rhs; }) {
    out.has_value = true;
    if (lhs < rhs) {
      out.value = T{0};
      return out;
    }
    if (lhs > rhs) {
      out.value = T{2};
      return out;
    }
    out.value = T{1};
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "three-way comparison not supported for negotiated "
                       "common type");
}

template <typename T>
constexpr auto compare_less_than(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (!(requires { T{0}; T{1}; T{2}; T{3}; })) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "less than comparison codes are not representable for common type");
  }

  if constexpr (std::same_as<std::remove_cv_t<T>, bool>) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "less than comparison is not representable for bool common type");
  } else if constexpr (requires { lhs < rhs; }) {
    out.has_value = true;
    out.value = T{lhs < rhs};
    return out;
  } else if constexpr (requires { lhs <=> rhs; }) {
    auto const cmp = lhs <=> rhs;
    out.has_value = true;
    out.value = T{cmp < 0};
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "less than not supported for negotiated common type");
}

template <typename T>
constexpr auto compare_greater_than(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (!(requires { T{0}; T{1}; T{2}; T{3}; })) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "greater than comparison codes are not representable for common type");
  }

  if constexpr (std::same_as<std::remove_cv_t<T>, bool>) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "greater than comparison is not representable for bool common type");
  } else if constexpr (requires { lhs > rhs; }) {
    out.has_value = true;
    out.value = T{lhs > rhs};
    return out;
  } else if constexpr (requires { lhs <=> rhs; }) {
    auto const cmp = lhs <=> rhs;
    out.has_value = true;
    out.value = T{cmp > 0};
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "greater than not supported for negotiated common type");
}

template <typename T>
constexpr auto compare_less_than_or_equal(T lhs, T rhs)
    -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (!(requires { T{0}; T{1}; T{2}; T{3}; })) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "less than or equal comparison codes are not representable for common type");
  }

  if constexpr (std::same_as<std::remove_cv_t<T>, bool>) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "less than or equal comparison is not representable for bool common type");
  } else if constexpr (requires { lhs <= rhs; }) {
    out.has_value = true;
    out.value = T{lhs <= rhs};
    return out;
  } else if constexpr (requires { lhs <=> rhs; }) {
    auto const cmp = lhs <=> rhs;
    out.has_value = true;
    out.value = T{cmp <= 0};
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "less than or equal not supported for negotiated common type");
}

template <typename T>
constexpr auto compare_greater_than_or_equal(T lhs, T rhs)
    -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (!(requires { T{0}; T{1}; T{2}; T{3}; })) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "greater than or equal comparison codes are not representable for common type");
  }

  if constexpr (std::same_as<std::remove_cv_t<T>, bool>) {
    return make_error<T>(
        policy::error::kind::unspecified,
        "greater than or equal comparison is not representable for bool common type");
  } else if constexpr (requires { lhs >= rhs; }) {
    out.has_value = true;
    out.value = T{lhs >= rhs};
    return out;
  } else if constexpr (requires { lhs <=> rhs; }) {
    auto const cmp = lhs <=> rhs;
    out.has_value = true;
    out.value = T{cmp >= 0};
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "greater than or equal not supported for negotiated common type");
}

template <typename T>
constexpr auto unchecked_add(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs + rhs; }) {
    out.value = static_cast<T>(lhs + rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_sub(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs - rhs; }) {
    out.value = static_cast<T>(lhs - rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_mul(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs * rhs; }) {
    out.value = static_cast<T>(lhs * rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_div(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  // Intentionally no guards: unchecked policy delegates error/UB behavior
  // to the underlying language/runtime semantics.
  out.value = static_cast<T>(lhs / rhs);
  return out;
}

template <typename T>
constexpr auto unchecked_mod(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs % rhs; }) {
    out.value = static_cast<T>(lhs % rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_shift_left(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs << rhs; }) {
    out.value = static_cast<T>(lhs << rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_shift_right(T lhs, T rhs)
    -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs >> rhs; }) {
    out.value = static_cast<T>(lhs >> rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_bit_and(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs & rhs; }) {
    out.value = static_cast<T>(lhs & rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_bit_or(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs | rhs; }) {
    out.value = static_cast<T>(lhs | rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_bit_xor(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs ^ rhs; }) {
    out.value = static_cast<T>(lhs ^ rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_bit_not(T lhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { ~lhs; }) {
    out.value = static_cast<T>(~lhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_unary_plus(T lhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { +lhs; }) {
    out.value = static_cast<T>(+lhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_unary_minus(T lhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { -lhs; }) {
    out.value = static_cast<T>(-lhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_inc(T lhs) -> policy::value::decision<T> {
  return unchecked_add(lhs, static_cast<T>(1));
}

template <typename T>
constexpr auto unchecked_dec(T lhs) -> policy::value::decision<T> {
  return unchecked_sub(lhs, static_cast<T>(1));
}

template <typename T> constexpr auto saturating_add(T lhs, T rhs) -> T {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    return static_cast<T>(lhs + rhs);
  } else if constexpr (std::is_unsigned_v<T>) {
    auto const maxv = algorithms::max_value<T>();
    return (lhs > maxv - rhs) ? maxv : static_cast<T>(lhs + rhs);
  } else {
    auto const maxv = algorithms::max_value<T>();
    auto const minv = algorithms::min_value<T>();
    if ((rhs > 0) && (lhs > maxv - rhs)) {
      return maxv;
    }
    if ((rhs < 0) && (lhs < minv - rhs)) {
      return minv;
    }
    return static_cast<T>(lhs + rhs);
  }
}

template <typename T> constexpr auto saturating_sub(T lhs, T rhs) -> T {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    return static_cast<T>(lhs - rhs);
  } else if constexpr (std::is_unsigned_v<T>) {
    return (lhs < rhs) ? T{} : static_cast<T>(lhs - rhs);
  } else {
    auto const maxv = algorithms::max_value<T>();
    auto const minv = algorithms::min_value<T>();
    if ((rhs < 0) && (lhs > maxv + rhs)) {
      return maxv;
    }
    if ((rhs > 0) && (lhs < minv + rhs)) {
      return minv;
    }
    return static_cast<T>(lhs - rhs);
  }
}

template <typename T> constexpr auto saturating_mul(T lhs, T rhs) -> T {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    return static_cast<T>(lhs * rhs);
  } else {
    if (lhs == T{} || rhs == T{}) {
      return T{};
    }

    if constexpr (std::is_unsigned_v<T>) {
      auto const maxv = algorithms::max_value<T>();
      return (lhs > maxv / rhs) ? maxv : static_cast<T>(lhs * rhs);
    } else {
      auto const maxv = algorithms::max_value<T>();
      auto const minv = algorithms::min_value<T>();

      if (lhs > 0) {
        if (rhs > 0) {
          if (lhs > maxv / rhs) {
            return maxv;
          }
        } else {
          if (rhs < minv / lhs) {
            return minv;
          }
        }
      } else {
        if (rhs > 0) {
          if (lhs < minv / rhs) {
            return minv;
          }
        } else {
          if (lhs != 0 && rhs < maxv / lhs) {
            return maxv;
          }
        }
      }

      return static_cast<T>(lhs * rhs);
    }
  }
}

template <typename T> constexpr auto saturating_unary_minus(T lhs) -> T {
  return saturating_sub(static_cast<T>(0), lhs);
}

template <typename T> constexpr auto saturating_inc(T lhs) -> T {
  return saturating_add(lhs, static_cast<T>(1));
}

template <typename T> constexpr auto saturating_dec(T lhs) -> T {
  return saturating_sub(lhs, static_cast<T>(1));
}

template <typename CommonRep>
constexpr auto make_unsupported(char const *reason)
    -> policy::value::decision<CommonRep> {
  return make_error<CommonRep>(policy::error::kind::unspecified, reason);
}

template <typename CommonRep>
constexpr auto make_div_zero(char const *reason)
    -> policy::value::decision<CommonRep> {
  return make_error<CommonRep>(policy::error::kind::divide_by_zero, reason);
}

constexpr auto apply_runtime_fence(const bool enabled,
                                   const std::memory_order order) noexcept -> void {
  if (!enabled) {
    return;
  }

  if !consteval {
    std::atomic_thread_fence(order);
  }
}

} // namespace mcpplibs::primitives::operations::runtime::details



export namespace mcpplibs::primitives::operations::runtime {

template <operation OpTag, policy::value_policy ValuePolicy, typename CommonRep>
struct op_binding {
  static constexpr bool enabled = false;

  static constexpr auto apply(CommonRep, CommonRep)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    out.has_value = false;
    out.error.code = policy::error::kind::unspecified;
    out.error.reason = "operation binding is not implemented";
    return out;
  }
};

template <typename CommonRep>
struct op_binding<Addition, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_add(lhs, rhs); }) {
      return details::checked_add(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked addition not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Subtraction, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_sub(lhs, rhs); }) {
      return details::checked_sub(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked subtraction not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Multiplication, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_mul(lhs, rhs); }) {
      return details::checked_mul(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked multiplication not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Division, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_div(lhs, rhs); }) {
      return details::checked_div(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked division not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Modulus, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_mod(lhs, rhs); }) {
      return details::checked_mod(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked modulus not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<LeftShift, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_shift_left(lhs, rhs); }) {
      return details::checked_shift_left(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked left shift not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<RightShift, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_shift_right(lhs, rhs); }) {
      return details::checked_shift_right(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked right shift not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<BitwiseAnd, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_and(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseOr, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_or(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseXor, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_xor(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Increment, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::checked_inc(lhs);
  }
};

template <typename CommonRep>
struct op_binding<Decrement, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::checked_dec(lhs);
  }
};

template <typename CommonRep>
struct op_binding<UnaryPlus, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::checked_unary_plus(lhs);
  }
};

template <typename CommonRep>
struct op_binding<UnaryMinus, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::checked_unary_minus(lhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseNot, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_not(lhs);
  }
};

template <typename CommonRep>
struct op_binding<Addition, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_add(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Subtraction, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_sub(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Multiplication, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_mul(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Division, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_div(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Modulus, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_mod(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<LeftShift, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_shift_left(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<RightShift, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_shift_right(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseAnd, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_bit_and(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseOr, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_bit_or(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseXor, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_bit_xor(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Increment, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_inc(lhs);
  }
};

template <typename CommonRep>
struct op_binding<Decrement, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_dec(lhs);
  }
};

template <typename CommonRep>
struct op_binding<UnaryPlus, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_unary_plus(lhs);
  }
};

template <typename CommonRep>
struct op_binding<UnaryMinus, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_unary_minus(lhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseNot, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_bit_not(lhs);
  }
};

template <typename CommonRep>
struct op_binding<Addition, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_add(lhs, rhs); }) {
      out.has_value = true;
      out.value = details::saturating_add(lhs, rhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating addition not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Subtraction, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_sub(lhs, rhs); }) {
      out.has_value = true;
      out.value = details::saturating_sub(lhs, rhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating subtraction not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Multiplication, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_mul(lhs, rhs); }) {
      out.has_value = true;
      out.value = details::saturating_mul(lhs, rhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating multiplication not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Division, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};

    if (rhs == CommonRep{}) {
      return details::make_div_zero<CommonRep>("saturating division by zero");
    }

    if constexpr (requires { lhs / rhs; }) {
      out.has_value = true;
      out.value = static_cast<CommonRep>(lhs / rhs);
      return out;
    }

    return details::make_unsupported<CommonRep>(
        "saturating division not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Modulus, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return op_binding<Modulus, policy::value::checked, CommonRep>::apply(lhs,
                                                                          rhs);
  }
};

template <typename CommonRep>
struct op_binding<LeftShift, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return op_binding<LeftShift, policy::value::checked, CommonRep>::apply(lhs,
                                                                            rhs);
  }
};

template <typename CommonRep>
struct op_binding<RightShift, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return op_binding<RightShift, policy::value::checked, CommonRep>::apply(
        lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseAnd, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_and(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseOr, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_or(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<BitwiseXor, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_xor(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Increment, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_inc(lhs); }) {
      out.has_value = true;
      out.value = details::saturating_inc(lhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating increment not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Decrement, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_dec(lhs); }) {
      out.has_value = true;
      out.value = details::saturating_dec(lhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating decrement not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<UnaryPlus, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::checked_unary_plus(lhs);
  }
};

template <typename CommonRep>
struct op_binding<UnaryMinus, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_unary_minus(lhs); }) {
      out.has_value = true;
      out.value = details::saturating_unary_minus(lhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating unary minus not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<BitwiseNot, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep)
      -> policy::value::decision<CommonRep> {
    return details::checked_bit_not(lhs);
  }
};

template <typename CommonRep>
struct op_binding<Equal, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Equal, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Equal, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<NotEqual, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_not_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<NotEqual, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_not_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<NotEqual, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_not_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<ThreeWayCompare, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_three_way(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<ThreeWayCompare, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_three_way(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<ThreeWayCompare, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_three_way(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<LessThan, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_less_than(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<LessThan, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_less_than(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<LessThan, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_less_than(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<GreaterThan, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_greater_than(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<GreaterThan, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_greater_than(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<GreaterThan, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_greater_than(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<LessThanOrEqual, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_less_than_or_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<LessThanOrEqual, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_less_than_or_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<LessThanOrEqual, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_less_than_or_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<GreaterThanOrEqual, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_greater_than_or_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<GreaterThanOrEqual, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_greater_than_or_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<GreaterThanOrEqual, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_greater_than_or_equal(lhs, rhs);
  }
};

template <typename OpTag, typename ValuePolicy, typename CommonRep>
concept op_binding_available = requires {
  requires operation<OpTag>;
  requires policy::value_policy<ValuePolicy>;
  requires op_binding<OpTag, ValuePolicy, CommonRep>::enabled;
};

template <typename ConcurrencyHandler, typename CommonRep>
constexpr auto inject_concurrency() -> policy::concurrency::injection {
  static_cast<void>(sizeof(CommonRep));
  static_assert(ConcurrencyHandler::enabled,
                "Selected concurrency handler is not enabled");
  static_assert(std::same_as<typename ConcurrencyHandler::injection_type,
                             policy::concurrency::injection>,
                "concurrency handler must use policy::concurrency::injection");

  return ConcurrencyHandler::inject();
}

template <operation OpTag, typename ValuePolicy, typename CommonRep,
          typename ValueHandler, typename ErrorPayload>
constexpr auto run_value(CommonRep lhs, CommonRep rhs,
                         policy::concurrency::injection const &injection)
    -> policy::value::decision<CommonRep> {
  static_cast<void>(sizeof(ErrorPayload));
  static_assert(ValueHandler::enabled, "Selected value handler is not enabled");
  static_assert(
      std::same_as<typename ValueHandler::decision_type,
                   policy::value::decision<CommonRep>>,
      "value handler decision_type must match policy::value::decision");

  static_assert(
      op_binding_available<OpTag, ValuePolicy, CommonRep>,
      "Missing operation binding specialization for this OpTag/common type");

  details::apply_runtime_fence(injection.fence_before, injection.order_before);

  auto decision = op_binding<OpTag, ValuePolicy, CommonRep>::apply(lhs, rhs);
  auto finalized = ValueHandler::finalize(std::move(decision), injection);

  details::apply_runtime_fence(injection.fence_after, injection.order_after);
  return finalized;
}

template <typename ErrorPolicy, operation OpTag, typename CommonRep,
          typename ErrorPayload>
constexpr auto resolve_error(policy::error::request<CommonRep> const &request)
    -> std::expected<CommonRep, ErrorPayload> {
  using handler_t =
      policy::error::handler<ErrorPolicy, OpTag, CommonRep, ErrorPayload>;
  return handler_t::resolve(request);
}

} // namespace mcpplibs::primitives::operations::runtime
