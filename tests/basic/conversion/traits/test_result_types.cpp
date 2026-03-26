#include <expected>
#include <gtest/gtest.h>

import mcpplibs.primitives.conversion.traits;

using namespace mcpplibs::primitives;

TEST(ConversionTypeTest, ResultTypesMatchContracts) {
  static_assert(std::same_as<conversion::cast_result<int>,
                             std::expected<int, conversion::risk::kind>>);
}
