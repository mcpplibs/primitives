#include <gtest/gtest.h>

import mcpplibs.primitives.underlying;

#include "../../support/conversion_box_types.hpp"

import mcpplibs.primitives.conversion;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::conversion;

TEST(ConversionPrimitiveUnderlyingTest,
     CheckedCastSupportsUnderlyingToPrimitiveBridge) {
  using value_t =
      primitive<UnsignedBox, policy::value::checked, policy::error::expected>;

  auto const ok = conversion::checked_cast<value_t>(SignedBox{42});
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(ok->load().value, 42);

  auto const bad = conversion::checked_cast<value_t>(SignedBox{-1});
  ASSERT_FALSE(bad.has_value());
  EXPECT_EQ(bad.error(), conversion::risk::kind::underflow);
}

TEST(ConversionPrimitiveUnderlyingTest,
     PrimitiveToUnderlyingCastsReuseUnderlyingConversion) {
  using value_t =
      primitive<SignedBox, policy::value::checked, policy::error::expected>;

  auto const ok = conversion::checked_cast<UnsignedBox>(value_t{SignedBox{42}});
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(ok->value, 42);

  auto const truncated =
      conversion::truncating_cast<SignedBox>(primitive<FloatBox>{
          FloatBox{12.75}});
  EXPECT_EQ(truncated.value, 12);
}
