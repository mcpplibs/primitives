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

  EXPECT_TRUE((policy::traits<policy::value::checked>::enabled));
  EXPECT_EQ(policy::traits<policy::value::checked>::kind,
            policy::category::value);

  EXPECT_TRUE((policy::traits<policy::type::compatible>::enabled));
  EXPECT_EQ(policy::traits<policy::type::compatible>::kind,
            policy::category::type);

  EXPECT_TRUE((policy::traits<policy::type::transparent>::enabled));
  EXPECT_EQ(policy::traits<policy::type::transparent>::kind,
            policy::category::type);

  EXPECT_TRUE((policy::traits<policy::error::throwing>::enabled));
  EXPECT_EQ(policy::traits<policy::error::throwing>::kind,
            policy::category::error);

  EXPECT_TRUE((policy::traits<policy::error::terminate>::enabled));
  EXPECT_EQ(policy::traits<policy::error::terminate>::kind,
            policy::category::error);

  EXPECT_TRUE((policy::traits<policy::concurrency::none>::enabled));
  EXPECT_EQ(policy::traits<policy::concurrency::none>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy::traits<policy::concurrency::atomic>::enabled));
  EXPECT_EQ(policy::traits<policy::concurrency::atomic>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy::traits<policy::value::saturating>::enabled));
  EXPECT_EQ(policy::traits<policy::value::saturating>::kind,
            policy::category::value);

  EXPECT_TRUE((policy_type<policy::value::checked>));
  EXPECT_TRUE((policy_type<policy::error::terminate>));
  EXPECT_FALSE((policy_type<int>));

  EXPECT_TRUE(
      (std::is_same_v<policy::defaults::value, policy::value::checked>));
  EXPECT_TRUE((std::is_same_v<policy::defaults::type, policy::type::strict>));
}

TEST(PolicyConcurrencyTest, AtomicInjectsFences) {
  using atomic_handler =
      policy::concurrency::handler<policy::concurrency::atomic,
                                   operations::Addition, int,
                                   policy::error::kind>;
  using single_handler = policy::concurrency::handler<policy::concurrency::none,
                                                      operations::Addition, int,
                                                      policy::error::kind>;

  auto const atomic_injection = atomic_handler::inject();
  auto const single_injection = single_handler::inject();

  EXPECT_TRUE(atomic_injection.fence_before);
  EXPECT_TRUE(atomic_injection.fence_after);
  EXPECT_FALSE(single_injection.fence_before);
  EXPECT_FALSE(single_injection.fence_after);
}

TEST(PolicyProtocolTest, BuiltinHandlersSatisfyProtocolConcepts) {
  static_assert(policy::type::handler_protocol<policy::type::strict,
                                               operations::Addition, int, int>);
  static_assert(policy::concurrency::handler_protocol<policy::concurrency::none,
                                                      operations::Addition, int,
                                                      policy::error::kind>);
  static_assert(policy::value::handler_protocol<policy::value::checked,
                                                operations::Addition, int,
                                                policy::error::kind>);
  static_assert(policy::error::handler_protocol<policy::error::expected,
                                                operations::Addition, int,
                                                policy::error::kind>);

  EXPECT_TRUE((policy::type::handler_protocol<policy::type::strict,
                                              operations::Addition, int, int>));
  EXPECT_TRUE((policy::concurrency::handler_protocol<policy::concurrency::none,
                                                     operations::Addition, int,
                                                     policy::error::kind>));
  EXPECT_TRUE((policy::value::handler_protocol<policy::value::checked,
                                               operations::Addition, int,
                                               policy::error::kind>));
  EXPECT_TRUE((policy::error::handler_protocol<policy::error::expected,
                                               operations::Addition, int,
                                               policy::error::kind>));
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
