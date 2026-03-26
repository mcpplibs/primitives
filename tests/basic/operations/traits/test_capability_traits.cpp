#include <gtest/gtest.h>

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.impl;

using namespace mcpplibs::primitives;

namespace {
struct NullCapabilityProbe {};
} // namespace

template <> struct operations::traits<NullCapabilityProbe> {
  using op_tag = NullCapabilityProbe;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::none;
};

TEST(OperationTraitsTest, BuiltinArithmeticOperationsExposeCapability) {
  EXPECT_TRUE(
      (operations::op_has_capability_v<operations::Addition,
                                       operations::capability::arithmetic>));
  EXPECT_TRUE(
      (operations::op_has_capability_v<operations::Subtraction,
                                       operations::capability::arithmetic>));
  EXPECT_TRUE(
      (operations::op_has_capability_v<operations::Multiplication,
                                       operations::capability::arithmetic>));
  EXPECT_TRUE(
      (operations::op_has_capability_v<operations::Division,
                                       operations::capability::arithmetic>));
  EXPECT_FALSE(
      (operations::op_has_capability_v<operations::Addition,
                                       operations::capability::comparison>));
  EXPECT_TRUE(
      (operations::op_has_capability_v<operations::Equal,
                                       operations::capability::comparison>));
  EXPECT_TRUE(
      (operations::op_has_capability_v<operations::NotEqual,
                                       operations::capability::comparison>));
}

TEST(OperationTraitsTest, BuiltinOperationsHaveNonNullCapabilityMask) {
  EXPECT_TRUE((operations::op_capability_valid_v<operations::Addition>));
  EXPECT_TRUE((operations::op_capability_valid_v<operations::Subtraction>));
  EXPECT_TRUE((operations::op_capability_valid_v<operations::Multiplication>));
  EXPECT_TRUE((operations::op_capability_valid_v<operations::Division>));
  EXPECT_TRUE((operations::op_capability_valid_v<operations::Equal>));
  EXPECT_TRUE((operations::op_capability_valid_v<operations::NotEqual>));
}

TEST(OperationTraitsTest, NullCapabilityMaskIsDetectedAtCompileTime) {
  static_assert(!operations::op_capability_valid_v<NullCapabilityProbe>);
  EXPECT_FALSE((operations::op_capability_valid_v<NullCapabilityProbe>));
}
