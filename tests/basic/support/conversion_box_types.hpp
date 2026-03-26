#pragma once

#include <cstdint>

namespace mcpplibs::primitives::test_support::conversion {

struct SignedBox {
  long long value;
};

struct UnsignedBox {
  std::uint16_t value;
};

struct FloatBox {
  double value;
};

} // namespace mcpplibs::primitives::test_support::conversion

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::conversion::SignedBox> {
  using value_type = mcpplibs::primitives::test_support::conversion::SignedBox;
  using rep_type = long long;

  static constexpr bool enabled = true;
  static constexpr auto kind = mcpplibs::primitives::underlying::category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value_type{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::conversion::UnsignedBox> {
  using value_type = mcpplibs::primitives::test_support::conversion::UnsignedBox;
  using rep_type = std::uint16_t;

  static constexpr bool enabled = true;
  static constexpr auto kind = mcpplibs::primitives::underlying::category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value_type{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::conversion::FloatBox> {
  using value_type = mcpplibs::primitives::test_support::conversion::FloatBox;
  using rep_type = double;

  static constexpr bool enabled = true;
  static constexpr auto kind =
      mcpplibs::primitives::underlying::category::floating;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value_type{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};
