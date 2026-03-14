#include <gtest/gtest.h>

import mcpplibs.primitive;

namespace {} // namespace

using namespace mcpplibs::primitive;

TEST(PolicyTraitsTest, BuiltinPoliciesHaveCategories) {
  using namespace policy;

  EXPECT_TRUE((policy::traits<policy::checked_value>::enabled));
  EXPECT_EQ(policy::traits<policy::checked_value>::kind,
            policy::category::value);

  EXPECT_TRUE((policy::traits<policy::relaxed_type>::enabled));
  EXPECT_EQ(policy::traits<policy::relaxed_type>::kind, policy::category::type);

  EXPECT_TRUE((policy::traits<policy::throw_error>::enabled));
  EXPECT_EQ(policy::traits<policy::throw_error>::kind, policy::category::error);

  EXPECT_TRUE((policy::traits<policy::single_thread>::enabled));
  EXPECT_EQ(policy::traits<policy::single_thread>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy_type<policy::checked_value>));
  EXPECT_FALSE((policy_type<int>));

  EXPECT_TRUE((std::is_same_v<policy::default_policies::value,
                              policy::unchecked_value>));
}

// Use the existing test runner main from other test translation unit.
