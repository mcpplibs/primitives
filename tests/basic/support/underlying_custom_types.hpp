#pragma once

namespace mcpplibs::primitives::test_support::underlying {

struct UserInteger {
  int value;
};

struct NotRegistered {
  int value;
};

struct BadRep {
  int value;
};

struct BadKind {
  int value;
};

struct BigIntLike {
  long long value;

  friend constexpr auto operator+(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(BigIntLike lhs, BigIntLike rhs) noexcept
      -> bool {
    return lhs.value == rhs.value;
  }
};

struct BadCustomKind {
  long long value;

  friend constexpr auto operator+(BadCustomKind lhs,
                                  BadCustomKind rhs) noexcept -> BadCustomKind {
    return BadCustomKind{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(BadCustomKind lhs,
                                  BadCustomKind rhs) noexcept -> BadCustomKind {
    return BadCustomKind{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(BadCustomKind lhs,
                                  BadCustomKind rhs) noexcept -> BadCustomKind {
    return BadCustomKind{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(BadCustomKind lhs,
                                  BadCustomKind rhs) noexcept -> BadCustomKind {
    return BadCustomKind{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(BadCustomKind lhs,
                                   BadCustomKind rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct MissingDivisionLike {
  long long value;

  friend constexpr auto operator+(MissingDivisionLike lhs,
                                  MissingDivisionLike rhs) noexcept
      -> MissingDivisionLike {
    return MissingDivisionLike{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(MissingDivisionLike lhs,
                                  MissingDivisionLike rhs) noexcept
      -> MissingDivisionLike {
    return MissingDivisionLike{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(MissingDivisionLike lhs,
                                  MissingDivisionLike rhs) noexcept
      -> MissingDivisionLike {
    return MissingDivisionLike{lhs.value * rhs.value};
  }

  friend constexpr auto operator==(MissingDivisionLike lhs,
                                   MissingDivisionLike rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct NonNegativeInt {
  int value;
};

struct ExplicitCommonLhs {
  int value;
};

struct ExplicitCommonRhs {
  int value;
};

struct ExplicitCommonRep {
  long long value;

  friend constexpr auto operator+(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(ExplicitCommonRep lhs,
                                   ExplicitCommonRep rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct VoidCommonLhs {
  int value;
};

struct VoidCommonRhs {
  int value;
};

} // namespace mcpplibs::primitives::test_support::underlying

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::UserInteger> {
  using value_type = mcpplibs::primitives::test_support::underlying::UserInteger;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

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
    mcpplibs::primitives::test_support::underlying::BadRep> {
  using value_type = mcpplibs::primitives::test_support::underlying::BadRep;
  using rep_type = value_type;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::BadKind> {
  using value_type = mcpplibs::primitives::test_support::underlying::BadKind;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::floating;

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
    mcpplibs::primitives::test_support::underlying::BigIntLike> {
  using value_type = mcpplibs::primitives::test_support::underlying::BigIntLike;
  using rep_type = value_type;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::BadCustomKind> {
  using value_type =
      mcpplibs::primitives::test_support::underlying::BadCustomKind;
  using rep_type = value_type;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::boolean;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::MissingDivisionLike> {
  using value_type =
      mcpplibs::primitives::test_support::underlying::MissingDivisionLike;
  using rep_type = value_type;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::NonNegativeInt> {
  using value_type =
      mcpplibs::primitives::test_support::underlying::NonNegativeInt;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value_type{value};
  }

  static constexpr auto is_valid_rep(rep_type value) noexcept -> bool {
    return value >= 0;
  }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::ExplicitCommonLhs> {
  using value_type =
      mcpplibs::primitives::test_support::underlying::ExplicitCommonLhs;
  using rep_type = value_type;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::ExplicitCommonRhs> {
  using value_type =
      mcpplibs::primitives::test_support::underlying::ExplicitCommonRhs;
  using rep_type = value_type;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<
    mcpplibs::primitives::test_support::underlying::ExplicitCommonLhs,
    mcpplibs::primitives::test_support::underlying::ExplicitCommonRhs> {
  using type = mcpplibs::primitives::test_support::underlying::ExplicitCommonRep;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<
    mcpplibs::primitives::test_support::underlying::ExplicitCommonRhs,
    mcpplibs::primitives::test_support::underlying::ExplicitCommonLhs> {
  using type = mcpplibs::primitives::test_support::underlying::ExplicitCommonRep;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::traits<
    mcpplibs::primitives::test_support::underlying::VoidCommonLhs> {
  using value_type =
      mcpplibs::primitives::test_support::underlying::VoidCommonLhs;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

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
    mcpplibs::primitives::test_support::underlying::VoidCommonRhs> {
  using value_type =
      mcpplibs::primitives::test_support::underlying::VoidCommonRhs;
  using rep_type = short;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return static_cast<rep_type>(value.value);
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value_type{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<int, short> {
  using type = void;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<short, int> {
  using type = void;
  static constexpr bool enabled = true;
};
