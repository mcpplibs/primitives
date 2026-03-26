#include <functional>
#include <gtest/gtest.h>
#include <type_traits>

import mcpplibs.primitives.algorithms;
import mcpplibs.primitives.underlying;

#include "../../support/underlying_custom_types.hpp"

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::underlying;

TEST(AlgorithmHashTest, BuiltinHashDelegatesToStdHash) {
  static_assert(algorithms::hashable<int>);
  static_assert(std::same_as<algorithms::hash_result_t<int>, std::size_t>);

  auto const value = 42;
  EXPECT_EQ(algorithms::hash<int>{}(value), std::hash<int>{}(value));
  EXPECT_EQ(algorithms::hash_value(value), std::hash<int>{}(value));
}

TEST(AlgorithmHashTest, CustomUnderlyingHashesThroughRepType) {
  static_assert(algorithms::hashable<UserInteger>);

  auto const value = UserInteger{42};
  auto const expected = std::hash<int>{}(42);

  EXPECT_EQ(algorithms::hash<UserInteger>{}(value), expected);
  EXPECT_EQ(algorithms::hash_value(value), expected);
}

TEST(AlgorithmHashTest, UnsupportedSelfRepresentedRepRemainsDisabled) {
  static_assert(!algorithms::hashable<BigIntLike>);
  SUCCEED();
}
