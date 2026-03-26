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

struct IntBridgeRep {
  int value;

  friend constexpr auto operator+(IntBridgeRep lhs,
                                  IntBridgeRep rhs) noexcept -> IntBridgeRep {
    return IntBridgeRep{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(IntBridgeRep lhs,
                                  IntBridgeRep rhs) noexcept -> IntBridgeRep {
    return IntBridgeRep{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(IntBridgeRep lhs,
                                  IntBridgeRep rhs) noexcept -> IntBridgeRep {
    return IntBridgeRep{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(IntBridgeRep lhs,
                                  IntBridgeRep rhs) noexcept -> IntBridgeRep {
    return IntBridgeRep{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(IntBridgeRep lhs,
                                   IntBridgeRep rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }

  constexpr explicit operator int() const noexcept { return value; }
};

struct FloatBridgeRep {
  double value;

  friend constexpr auto operator+(FloatBridgeRep lhs,
                                  FloatBridgeRep rhs) noexcept
      -> FloatBridgeRep {
    return FloatBridgeRep{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(FloatBridgeRep lhs,
                                  FloatBridgeRep rhs) noexcept
      -> FloatBridgeRep {
    return FloatBridgeRep{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(FloatBridgeRep lhs,
                                  FloatBridgeRep rhs) noexcept
      -> FloatBridgeRep {
    return FloatBridgeRep{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(FloatBridgeRep lhs,
                                  FloatBridgeRep rhs) noexcept
      -> FloatBridgeRep {
    return FloatBridgeRep{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(FloatBridgeRep lhs,
                                   FloatBridgeRep rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }

  constexpr explicit operator double() const noexcept { return value; }
};

struct BridgedIntBox {
  IntBridgeRep value;
};

struct BridgedFloatBox {
  FloatBridgeRep value;
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

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::conversion::BridgedIntBox> {
  using value_type =
      mcpplibs::primitives::test_support::conversion::BridgedIntBox;
  using rep_type =
      mcpplibs::primitives::test_support::conversion::IntBridgeRep;

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
    mcpplibs::primitives::test_support::conversion::BridgedFloatBox> {
  using value_type =
      mcpplibs::primitives::test_support::conversion::BridgedFloatBox;
  using rep_type =
      mcpplibs::primitives::test_support::conversion::FloatBridgeRep;

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

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<
    mcpplibs::primitives::test_support::conversion::IntBridgeRep, int> {
  using type = int;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<
    int, mcpplibs::primitives::test_support::conversion::IntBridgeRep> {
  using type = int;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<
    mcpplibs::primitives::test_support::conversion::FloatBridgeRep, double> {
  using type = double;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<
    double, mcpplibs::primitives::test_support::conversion::FloatBridgeRep> {
  using type = double;
  static constexpr bool enabled = true;
};
