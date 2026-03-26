#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.conversion.underlying;
import mcpplibs.primitives.underlying;

#include "../../support/conversion_box_types.hpp"

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::conversion;

TEST(ConversionRiskTest, NumericRiskDetectsOverflowAndUnderflow) {
  auto const overflow = conversion::numeric_risk<std::uint16_t>(70000);
  ASSERT_TRUE(overflow.has_value());
  EXPECT_EQ(*overflow, conversion::risk::kind::overflow);

  auto const underflow = conversion::numeric_risk<std::uint16_t>(-1);
  ASSERT_TRUE(underflow.has_value());
  EXPECT_EQ(*underflow, conversion::risk::kind::underflow);
}

TEST(ConversionRiskTest, NumericRiskDetectsDomainAndPrecisionLoss) {
  auto const domain =
      conversion::numeric_risk<int>(std::numeric_limits<double>::quiet_NaN());
  ASSERT_TRUE(domain.has_value());
  EXPECT_EQ(*domain, conversion::risk::kind::domain_error);

  auto const precision =
      conversion::numeric_risk<float>(std::numeric_limits<std::int64_t>::max());
  ASSERT_TRUE(precision.has_value());
  EXPECT_EQ(*precision, conversion::risk::kind::precision_loss);
}

TEST(ConversionRiskTest, NumericRiskSupportsUnderlyingRepBridge) {
  auto const underflow = conversion::numeric_risk<UnsignedBox>(SignedBox{-1});
  ASSERT_TRUE(underflow.has_value());
  EXPECT_EQ(*underflow, conversion::risk::kind::underflow);

  auto const domain = conversion::numeric_risk<SignedBox>(
      FloatBox{std::numeric_limits<double>::quiet_NaN()});
  ASSERT_TRUE(domain.has_value());
  EXPECT_EQ(*domain, conversion::risk::kind::domain_error);
}

TEST(ConversionRiskTest, NumericRiskSupportsBuiltinProxyRepBridge) {
  auto const underflow =
      conversion::numeric_risk<UnsignedBox>(BridgedIntBox{IntBridgeRep{-1}});
  ASSERT_TRUE(underflow.has_value());
  EXPECT_EQ(*underflow, conversion::risk::kind::underflow);

  auto const domain = conversion::numeric_risk<BridgedIntBox>(
      BridgedFloatBox{FloatBridgeRep{std::numeric_limits<double>::quiet_NaN()}});
  ASSERT_TRUE(domain.has_value());
  EXPECT_EQ(*domain, conversion::risk::kind::domain_error);
}
