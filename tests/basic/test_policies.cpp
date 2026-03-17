#include <gtest/gtest.h>

import mcpplibs.primitives;

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

TEST(PolicyTraitsTest, BuiltinPoliciesHaveCategories) {
  using namespace policy;

  EXPECT_TRUE((policy::traits<policy::checked_value>::enabled));
  EXPECT_EQ(policy::traits<policy::checked_value>::kind,
            policy::category::value);

  EXPECT_TRUE((policy::traits<policy::category_compatible_type>::enabled));
  EXPECT_EQ(policy::traits<policy::category_compatible_type>::kind,
            policy::category::type);

  EXPECT_TRUE((policy::traits<policy::transparent_type>::enabled));
  EXPECT_EQ(policy::traits<policy::transparent_type>::kind,
            policy::category::type);

  EXPECT_TRUE((policy::traits<policy::throw_error>::enabled));
  EXPECT_EQ(policy::traits<policy::throw_error>::kind, policy::category::error);

  EXPECT_TRUE((policy::traits<policy::terminate_error>::enabled));
  EXPECT_EQ(policy::traits<policy::terminate_error>::kind,
            policy::category::error);

  EXPECT_TRUE((policy::traits<policy::single_thread>::enabled));
  EXPECT_EQ(policy::traits<policy::single_thread>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy::traits<policy::atomic>::enabled));
  EXPECT_EQ(policy::traits<policy::atomic>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy::traits<policy::saturating_value>::enabled));
  EXPECT_EQ(policy::traits<policy::saturating_value>::kind,
            policy::category::value);

  EXPECT_TRUE((policy_type<policy::checked_value>));
  EXPECT_TRUE((policy_type<policy::terminate_error>));
  EXPECT_FALSE((policy_type<int>));

  EXPECT_TRUE((std::is_same_v<policy::default_value, policy::checked_value>));
  EXPECT_TRUE((std::is_same_v<policy::default_type, policy::strict_type>));
}

TEST(PolicyConcurrencyTest, AtomicInjectsFences) {
  using atomic_handler =
      policy::concurrency::handler<policy::atomic, operations::Addition, int,
                                   policy::error::kind>;
  using single_handler =
      policy::concurrency::handler<policy::single_thread, operations::Addition,
                                   int, policy::error::kind>;

  auto const atomic_injection = atomic_handler::inject();
  auto const single_injection = single_handler::inject();

  EXPECT_TRUE(atomic_injection.fence_before);
  EXPECT_TRUE(atomic_injection.fence_after);
  EXPECT_FALSE(single_injection.fence_before);
  EXPECT_FALSE(single_injection.fence_after);
}

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

// Use the existing test runner main from other test translation unit.
