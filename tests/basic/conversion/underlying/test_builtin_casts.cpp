#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.conversion.underlying;
import mcpplibs.primitives.underlying;

using namespace mcpplibs::primitives;

TEST(ConversionCastTest, CheckedCastReportsErrorForInvalidInput) {
  auto const ok = conversion::checked_cast<int>(42u);
  ASSERT_TRUE(ok.has_value());
  EXPECT_EQ(*ok, 42);

  auto const bad = conversion::checked_cast<std::uint16_t>(-7);
  ASSERT_FALSE(bad.has_value());
  EXPECT_EQ(bad.error(), conversion::risk::kind::underflow);
}
