#include <gtest/gtest.h>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.policy;

#include "../../support/underlying_custom_types.hpp"

using namespace mcpplibs::primitives;

TEST(PrimitiveTraitsTest, MetaTraitsExposeValueTypeAndPrimitiveMetadata) {
  using value_t =
      primitive<short, policy::value::checked, policy::type::compatible,
                policy::error::expected, policy::concurrency::fenced>;
  using traits_t = meta::traits<value_t const &>;

  static_assert(
      std::same_as<typename traits_t::value_type, typename value_t::value_type>);
  static_assert(std::same_as<typename traits_t::policies,
                             typename value_t::policies>);
  static_assert(std::same_as<typename traits_t::value_policy,
                             policy::value::checked>);
  static_assert(
      std::same_as<typename traits_t::type_policy, policy::type::compatible>);
  static_assert(
      std::same_as<typename traits_t::error_policy, policy::error::expected>);
  static_assert(std::same_as<typename traits_t::concurrency_policy,
                             policy::concurrency::fenced>);

  SUCCEED();
}

TEST(PrimitiveTraitsTest, MetaPrimitiveConceptsMatchUnderlyingCategory) {
  using boolean_t = primitive<bool>;
  using character_t = primitive<char>;
  using integer_t = primitive<int>;
  using floating_t = primitive<double>;

  static_assert(meta::primitive_type<boolean_t const &>);
  static_assert(meta::primitive_type<integer_t>);
  static_assert(!meta::primitive_type<int>);

  static_assert(meta::boolean<boolean_t>);
  static_assert(!meta::boolean<integer_t>);

  static_assert(meta::character<character_t>);
  static_assert(!meta::character<boolean_t>);

  static_assert(meta::integer<integer_t>);
  static_assert(!meta::integer<character_t>);

  static_assert(meta::floating<floating_t>);
  static_assert(!meta::floating<integer_t>);

  static_assert(meta::numeric<integer_t>);
  static_assert(meta::numeric<floating_t>);
  static_assert(!meta::numeric<boolean_t>);
  static_assert(!meta::numeric<character_t>);
  static_assert(!meta::numeric<int>);

  EXPECT_TRUE((meta::boolean<boolean_t>));
  EXPECT_TRUE((meta::character<character_t>));
  EXPECT_TRUE((meta::integer<integer_t>));
  EXPECT_TRUE((meta::floating<floating_t>));
}

TEST(PrimitiveTraitsTest, MetaPrimitiveLikeConceptsAcceptPrimitiveOrUnderlying) {
  using boolean_t = primitive<bool>;
  using character_t = primitive<char>;
  using integer_t = primitive<int>;
  using floating_t = primitive<double>;

  static_assert(meta::primitive_like<boolean_t>);
  static_assert(meta::primitive_like<int>);
  static_assert(!meta::primitive_like<void *>);

  static_assert(meta::boolean_like<boolean_t>);
  static_assert(meta::boolean_like<bool>);
  static_assert(!meta::boolean_like<int>);

  static_assert(meta::character_like<character_t>);
  static_assert(meta::character_like<char>);
  static_assert(!meta::character_like<int>);

  static_assert(meta::integer_like<integer_t>);
  static_assert(meta::integer_like<int>);
  static_assert(!meta::integer_like<char>);

  static_assert(meta::floating_like<floating_t>);
  static_assert(meta::floating_like<double>);
  static_assert(!meta::floating_like<int>);

  static_assert(meta::numeric_like<integer_t>);
  static_assert(meta::numeric_like<floating_t>);
  static_assert(meta::numeric_like<int>);
  static_assert(meta::numeric_like<double>);
  static_assert(!meta::numeric_like<bool>);
  static_assert(!meta::numeric_like<char>);

  EXPECT_TRUE((meta::primitive_like<boolean_t>));
  EXPECT_TRUE((meta::primitive_like<int>));
}
