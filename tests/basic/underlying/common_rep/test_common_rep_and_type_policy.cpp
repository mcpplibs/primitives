#include <gtest/gtest.h>
#include <type_traits>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.operations.impl;

#include "../../support/underlying_custom_types.hpp"
using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::underlying;

TEST(PrimitiveTraitsTest, UnderlyingCommonRepDefaultsToStdCommonType) {
  static_assert(has_common_rep<int, long long>);
  static_assert(std::same_as<common_rep_t<int, long long>, long long>);
  SUCCEED();
}

TEST(PrimitiveTraitsTest, UnderlyingCommonRepCanBeCustomizedViaTraits) {
  static_assert(has_common_rep<ExplicitCommonLhs, ExplicitCommonRhs>);
  static_assert(std::same_as<common_rep_t<ExplicitCommonLhs, ExplicitCommonRhs>,
                             ExplicitCommonRep>);

  using handler_t = policy::type::handler<policy::type::transparent,
                                          operations::Addition,
                                          ExplicitCommonLhs, ExplicitCommonRhs>;

  static_assert(handler_t::enabled);
  static_assert(handler_t::allowed);
  static_assert(std::same_as<typename handler_t::common_rep, ExplicitCommonRep>);
  SUCCEED();
}

TEST(PrimitiveTraitsTest, TypePoliciesRequireNonVoidCommonRep) {
  static_assert(std::is_arithmetic_v<underlying::traits<VoidCommonLhs>::rep_type>);
  static_assert(std::is_arithmetic_v<underlying::traits<VoidCommonRhs>::rep_type>);

  using compatible_handler_t = policy::type::handler<
      policy::type::compatible, operations::Addition, VoidCommonLhs,
      VoidCommonRhs>;
  using transparent_handler_t = policy::type::handler<
      policy::type::transparent, operations::Addition, VoidCommonLhs,
      VoidCommonRhs>;

  static_assert(compatible_handler_t::enabled);
  static_assert(transparent_handler_t::enabled);
  static_assert(!compatible_handler_t::allowed);
  static_assert(!transparent_handler_t::allowed);
  static_assert(std::same_as<typename compatible_handler_t::common_rep, void>);
  static_assert(std::same_as<typename transparent_handler_t::common_rep, void>);

  EXPECT_EQ(compatible_handler_t::diagnostic_id, 2u);
  EXPECT_EQ(transparent_handler_t::diagnostic_id, 3u);
}
