#include <gtest/gtest.h>
#include <limits>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.conversion.underlying;

#include "../../support/conversion_box_types.hpp"

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::conversion;

TEST(ConversionUnderlyingTest, CheckedCastUsesRepBridge) {
  auto const ok = conversion::checked_cast<UnsignedBox>(SignedBox{42});
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(ok->value, 42);

  auto const bad = conversion::checked_cast<UnsignedBox>(SignedBox{-1});
  ASSERT_FALSE(bad.has_value());
  EXPECT_EQ(bad.error(), conversion::risk::kind::underflow);
}

TEST(ConversionUnderlyingTest, SaturatingCastClampsByUnderlyingRep) {
  auto const saturated =
      conversion::saturating_cast<UnsignedBox>(SignedBox{
          std::numeric_limits<long long>::max()});
  EXPECT_EQ(saturated.value, std::numeric_limits<std::uint16_t>::max());
}

TEST(ConversionUnderlyingTest, TruncatingAndExactCastSupportFloatingSources) {
  auto const truncated = conversion::truncating_cast<SignedBox>(FloatBox{12.75});
  EXPECT_EQ(truncated.value, 12);

  auto const exact = conversion::exact_cast<SignedBox>(FloatBox{12.0});
  ASSERT_TRUE(exact.has_value());
  EXPECT_EQ(exact->value, 12);
}
