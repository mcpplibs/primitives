#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.conversion.underlying;
import mcpplibs.primitives.underlying;

using namespace mcpplibs::primitives;

TEST(ConversionCastTest, SaturatingCastClampsAndHandlesNaN) {
  EXPECT_EQ(conversion::saturating_cast<std::int16_t>(100000),
            std::numeric_limits<std::int16_t>::max());
  EXPECT_EQ(conversion::saturating_cast<int>(
                std::numeric_limits<double>::quiet_NaN()),
            0);
}
