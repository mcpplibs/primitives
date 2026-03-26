#include <gtest/gtest.h>
#include <limits>

import mcpplibs.primitives.underlying;

#include "../../support/conversion_box_types.hpp"

import mcpplibs.primitives.conversion;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::conversion;

TEST(ConversionPrimitiveTest, CheckedCastBetweenPrimitivesUsesRepBridge) {
  using src_t =
      primitive<SignedBox, policy::value::checked, policy::error::expected>;
  using dest_t =
      primitive<UnsignedBox, policy::value::checked, policy::error::expected>;

  auto const ok = conversion::checked_cast<dest_t>(src_t{SignedBox{42}});
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(ok->load().value, 42);

  auto const bad = conversion::checked_cast<dest_t>(src_t{SignedBox{-1}});
  ASSERT_FALSE(bad.has_value());
  EXPECT_EQ(bad.error(), conversion::risk::kind::underflow);
}

TEST(ConversionPrimitiveTest,
     TruncatingAndExactCastBetweenPrimitivesFollowUnderlyingRules) {
  using float_t =
      primitive<FloatBox, policy::value::checked, policy::error::expected>;
  using signed_t =
      primitive<SignedBox, policy::value::checked, policy::error::expected>;

  auto const truncated = conversion::truncating_cast<signed_t>(
      float_t{FloatBox{12.75}});
  EXPECT_EQ(truncated.load().value, 12);

  auto const exact = conversion::exact_cast<signed_t>(float_t{FloatBox{12.0}});
  ASSERT_TRUE(exact.has_value());
  EXPECT_EQ(exact->load().value, 12);

  auto const bad_exact = conversion::exact_cast<signed_t>(
      float_t{FloatBox{std::numeric_limits<double>::quiet_NaN()}});
  ASSERT_FALSE(bad_exact.has_value());
  EXPECT_EQ(bad_exact.error(), conversion::risk::kind::domain_error);
}
