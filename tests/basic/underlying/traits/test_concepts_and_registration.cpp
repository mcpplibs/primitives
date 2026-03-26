#include <gtest/gtest.h>

import mcpplibs.primitives.underlying;

#include "../../support/underlying_custom_types.hpp"

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::test_support::underlying;

TEST(PrimitiveTraitsTest, StandardTypeConcepts) {
  EXPECT_TRUE((std_integer<int>));
  EXPECT_TRUE((std_integer<long long>));
  EXPECT_FALSE((std_integer<bool>));
  EXPECT_FALSE((std_integer<char>));

  EXPECT_TRUE((std_floating<float>));
  EXPECT_TRUE((std_floating<double>));
  EXPECT_TRUE((std_floating<long double>));

  EXPECT_TRUE((std_bool<bool>));
  EXPECT_FALSE((std_bool<int>));

  EXPECT_TRUE((std_char<char>));
  EXPECT_TRUE((std_char<char8_t>));
  EXPECT_TRUE((std_char<wchar_t>));

  EXPECT_TRUE((std_underlying_type<int>));
  EXPECT_TRUE((std_underlying_type<double>));
  EXPECT_FALSE((std_underlying_type<void *>));
}

TEST(PrimitiveTraitsTest, UnderlyingTraitsDefaultsAndCustomRegistration) {
  EXPECT_TRUE((underlying_type<int>));
  EXPECT_EQ(underlying::traits<int>::kind, underlying::category::integer);

  EXPECT_TRUE((underlying_type<UserInteger>));
  EXPECT_EQ(underlying::traits<UserInteger>::to_rep(UserInteger{7}), 7);

  EXPECT_FALSE((underlying_type<NotRegistered>));
  EXPECT_FALSE((underlying::traits<NotRegistered>::enabled));
}

TEST(PrimitiveTraitsTest,
     UnderlyingTypeRequiresValidRepTypeAndCategoryConsistency) {
  EXPECT_TRUE((underlying::traits<BadRep>::enabled));
  EXPECT_FALSE((underlying_type<BadRep>));

  EXPECT_TRUE((underlying::traits<BadKind>::enabled));
  EXPECT_FALSE((underlying_type<BadKind>));
}

TEST(PrimitiveTraitsTest, UnderlyingTypeAllowsCustomNumericLikeRepType) {
  EXPECT_TRUE((underlying::traits<BigIntLike>::enabled));
  EXPECT_TRUE((underlying_type<BigIntLike>));
}

TEST(PrimitiveTraitsTest, CustomNumericLikeRepTypeRejectsInvalidCategory) {
  EXPECT_TRUE((underlying::traits<BadCustomKind>::enabled));
  EXPECT_FALSE((underlying_type<BadCustomKind>));
}

TEST(PrimitiveTraitsTest,
     CustomNumericLikeRepTypeRequiresDivisionOperatorForEligibility) {
  EXPECT_TRUE((underlying::traits<MissingDivisionLike>::enabled));
  EXPECT_FALSE((underlying_type<MissingDivisionLike>));
}
