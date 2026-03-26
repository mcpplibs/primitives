#include <gtest/gtest.h>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.operations.dispatcher;

using namespace mcpplibs::primitives;

namespace {

struct OpsBigIntLike {
  long long value;

  friend constexpr auto operator+(OpsBigIntLike lhs,
                                  OpsBigIntLike rhs) noexcept
      -> OpsBigIntLike {
    return OpsBigIntLike{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(OpsBigIntLike lhs,
                                  OpsBigIntLike rhs) noexcept
      -> OpsBigIntLike {
    return OpsBigIntLike{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(OpsBigIntLike lhs,
                                  OpsBigIntLike rhs) noexcept
      -> OpsBigIntLike {
    return OpsBigIntLike{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(OpsBigIntLike lhs,
                                  OpsBigIntLike rhs) noexcept
      -> OpsBigIntLike {
    return OpsBigIntLike{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(OpsBigIntLike lhs,
                                   OpsBigIntLike rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct OpsUserInteger {
  int value;
};

struct OpsNonNegativeInt {
  int value;
};

} // namespace

template <>
struct mcpplibs::primitives::underlying::traits<OpsBigIntLike> {
  using value_type = OpsBigIntLike;
  using rep_type = OpsBigIntLike;

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
struct mcpplibs::primitives::underlying::traits<OpsUserInteger> {
  using value_type = OpsUserInteger;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return OpsUserInteger{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<OpsNonNegativeInt> {
  using value_type = OpsNonNegativeInt;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return OpsNonNegativeInt{value};
  }

  static constexpr auto is_valid_rep(rep_type value) noexcept -> bool {
    return value >= 0;
  }
};

TEST(UnderlyingOperationsTest, CustomUnderlyingParticipatesInPrimitiveOperations) {
  using value_t =
      primitive<OpsBigIntLike, policy::value::checked, policy::error::expected>;

  auto const lhs = value_t{OpsBigIntLike{40}};
  auto const rhs = value_t{OpsBigIntLike{2}};

  auto const result = operations::dispatch<operations::Addition>(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value, 42);
}

TEST(UnderlyingOperationsTest, CustomWrappedUnderlyingUsesRepBridgeForArithmetic) {
  using value_t =
      primitive<OpsUserInteger, policy::value::checked, policy::error::expected>;

  auto const lhs = value_t{OpsUserInteger{40}};
  auto const rhs = value_t{OpsUserInteger{2}};

  auto const result = operations::dispatch<operations::Addition>(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 42);
}

TEST(UnderlyingOperationsTest, InvalidUnderlyingRepIsRejectedByDispatcher) {
  using value_t = primitive<OpsNonNegativeInt, policy::value::checked,
                            policy::error::expected>;

  auto const lhs = value_t{OpsNonNegativeInt{-1}};
  auto const rhs = value_t{OpsNonNegativeInt{2}};

  auto const result = operations::dispatch<operations::Addition>(lhs, rhs);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), policy::error::kind::domain_error);
}
