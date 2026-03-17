#include <gtest/gtest.h>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

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
      policy::concurrency_handler<policy::atomic, operations::Addition, int,
                                  policy::runtime_error_kind>;
  using single_handler =
      policy::concurrency_handler<policy::single_thread, operations::Addition,
                                  int, policy::runtime_error_kind>;

  auto const atomic_injection = atomic_handler::inject();
  auto const single_injection = single_handler::inject();

  EXPECT_TRUE(atomic_injection.fence_before);
  EXPECT_TRUE(atomic_injection.fence_after);
  EXPECT_FALSE(single_injection.fence_before);
  EXPECT_FALSE(single_injection.fence_after);
}

// Use the existing test runner main from other test translation unit.
