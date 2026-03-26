#include <gtest/gtest.h>
#include <type_traits>

import mcpplibs.primitives.policy;

using namespace mcpplibs::primitives;

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

  EXPECT_TRUE((policy::traits<policy::concurrency::fenced>::enabled));
  EXPECT_EQ(policy::traits<policy::concurrency::fenced>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy::traits<policy::concurrency::fenced_relaxed>::enabled));
  EXPECT_EQ(policy::traits<policy::concurrency::fenced_relaxed>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy::traits<policy::concurrency::fenced_acq_rel>::enabled));
  EXPECT_EQ(policy::traits<policy::concurrency::fenced_acq_rel>::kind,
            policy::category::concurrency);

  EXPECT_TRUE((policy::traits<policy::concurrency::fenced_seq_cst>::enabled));
  EXPECT_EQ(policy::traits<policy::concurrency::fenced_seq_cst>::kind,
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
