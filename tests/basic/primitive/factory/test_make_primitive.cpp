#include <gtest/gtest.h>
#include <type_traits>

import mcpplibs.primitives;

#include "../../support/underlying_custom_types.hpp"

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::literals;
using namespace mcpplibs::primitives::test_support::underlying;

TEST(PrimitiveFactoryTest, MakesPrimitiveFromDeducedStdUnderlying) {
  using expected_t =
      types::I32<policy::value::checked, policy::error::expected>;

  auto value = with<policy::value::checked, policy::error::expected>(42_i32);

  static_assert(std::same_as<decltype(value), expected_t>);
  EXPECT_EQ(value.load(), 42);
}

TEST(PrimitiveFactoryTest, UsesDefaultPoliciesWhenNoPolicyIsSpecified) {
  using expected_t = types::I32<>;
  using meta_t = meta::traits<expected_t>;

  auto value = with(42_i32);

  static_assert(std::same_as<decltype(value), expected_t>);
  static_assert(std::same_as<meta_t::value_policy,
                             policy::defaults::value>);
  static_assert(
      std::same_as<meta_t::type_policy, policy::defaults::type>);
  static_assert(
      std::same_as<meta_t::error_policy, policy::defaults::error>);
  static_assert(std::same_as<meta_t::concurrency_policy,
                             policy::defaults::concurrency>);
  EXPECT_EQ(value.load(), 42);
}

TEST(PrimitiveFactoryTest, MakesPrimitiveFromDeducedCustomUnderlying) {
  using expected_t =
      primitive<UserInteger, policy::value::checked, policy::type::compatible>;

  auto value =
      with<policy::value::checked, policy::type::compatible>(UserInteger{7});

  static_assert(std::same_as<decltype(value), expected_t>);
  EXPECT_EQ(value.load().value, 7);
}
