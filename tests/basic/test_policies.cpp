#include <gtest/gtest.h>

import mcpplibs.primitive;

namespace {} // namespace

using namespace mcpplibs::primitive;

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

  EXPECT_TRUE((policy::traits<policy::saturating_value>::enabled));
  EXPECT_EQ(policy::traits<policy::saturating_value>::kind,
            policy::category::value);

  EXPECT_TRUE((policy_type<policy::checked_value>));
  EXPECT_TRUE((policy_type<policy::terminate_error>));
  EXPECT_FALSE((policy_type<int>));

  EXPECT_TRUE((std::is_same_v<::mcpplibs::primitives::default_value_policy,
                              ::mcpplibs::primitive::policy::unchecked_value>));
  EXPECT_TRUE(
      (std::is_same_v<::mcpplibs::primitives::default_type_policy,
                      ::mcpplibs::primitive::policy::transparent_type>));
}

// Use the existing test runner main from other test translation unit.
