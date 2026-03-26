#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.conversion.underlying;

using namespace mcpplibs::primitives;

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
