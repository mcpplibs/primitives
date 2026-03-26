#include <cmath>
#include <concepts>
#include <gtest/gtest.h>
#include <limits>
#include <type_traits>

import mcpplibs.primitives.algorithms;
import mcpplibs.primitives.underlying;

#include "../../support/underlying_custom_types.hpp"

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::underlying;

TEST(AlgorithmLimitsTest, BuiltinIntegerLimitsFollowNumericLimitsShape) {
  using limits_t = algorithms::limits<int>;

  static_assert(limits_t::enabled);
  static_assert(limits_t::is_specialized);
  static_assert(std::same_as<typename limits_t::rep_type, int>);
  static_assert(limits_t::kind == underlying::category::integer);
  static_assert(limits_t::digits == std::numeric_limits<int>::digits);
  static_assert(limits_t::radix == std::numeric_limits<int>::radix);
  static_assert(algorithms::limited_type<int>);

  EXPECT_EQ(limits_t::lowest(), std::numeric_limits<int>::lowest());
  EXPECT_EQ(limits_t::max(), std::numeric_limits<int>::max());
  EXPECT_EQ(algorithms::min_value<int>(), std::numeric_limits<int>::min());
}

TEST(AlgorithmLimitsTest, BuiltinFloatingLimitsExposeSpecialValues) {
  using limits_t = algorithms::limits<double>;

  static_assert(limits_t::enabled);
  static_assert(limits_t::has_quiet_nan);
  static_assert(limits_t::has_infinity);
  static_assert(limits_t::kind == underlying::category::floating);
  static_assert(limits_t::is_iec559);

  EXPECT_EQ(algorithms::epsilon_value<double>(),
            std::numeric_limits<double>::epsilon());
  EXPECT_EQ(algorithms::infinity_value<double>(),
            std::numeric_limits<double>::infinity());
  EXPECT_TRUE(std::isnan(algorithms::quiet_nan_value<double>()));
}

TEST(AlgorithmLimitsTest, CustomUnderlyingUsesRepBackedLimits) {
  using limits_t = algorithms::limits<UserInteger>;

  static_assert(limits_t::enabled);
  static_assert(std::same_as<typename limits_t::rep_type, int>);
  static_assert(limits_t::kind == underlying::category::integer);

  EXPECT_EQ(underlying::traits<UserInteger>::to_rep(limits_t::lowest()),
            std::numeric_limits<int>::lowest());
  EXPECT_EQ(underlying::traits<UserInteger>::to_rep(limits_t::max()),
            std::numeric_limits<int>::max());
}

TEST(AlgorithmLimitsTest, SelfRepresentedCustomRepNeedsExplicitLimitsSpecialization) {
  static_assert(!algorithms::limits<BigIntLike>::enabled);
  SUCCEED();
}
