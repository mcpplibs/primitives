#include <gtest/gtest.h>

import mcpplibs.primitives;

namespace {

struct UserInteger {
  int value;
};

struct NotRegistered {
  int value;
};

struct BadRep {
  int value;
};

struct BadKind {
  int value;
};

} // namespace

template <>
struct mcpplibs::primitives::underlying::traits<UserInteger> {
  using value_type = UserInteger;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value.value; }

  static constexpr value_type from_rep(rep_type value) noexcept { return UserInteger{value}; }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<BadRep> {
  using value_type = BadRep;
  using rep_type = BadRep;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept { return value; }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<BadKind> {
  using value_type = BadKind;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::floating;

  static constexpr rep_type to_rep(value_type value) noexcept { return value.value; }

  static constexpr value_type from_rep(rep_type value) noexcept { return BadKind{value}; }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

TEST(PrimitiveTraitsTest, StandardTypeConcepts) {
  EXPECT_TRUE((mcpplibs::primitives::std_integer<int>));
  EXPECT_TRUE((mcpplibs::primitives::std_integer<long long>));
  EXPECT_FALSE((mcpplibs::primitives::std_integer<bool>));
  EXPECT_FALSE((mcpplibs::primitives::std_integer<char>));

  EXPECT_TRUE((mcpplibs::primitives::std_floating<float>));
  EXPECT_TRUE((mcpplibs::primitives::std_floating<double>));
  EXPECT_TRUE((mcpplibs::primitives::std_floating<long double>));

  EXPECT_TRUE((mcpplibs::primitives::std_bool<bool>));
  EXPECT_FALSE((mcpplibs::primitives::std_bool<int>));

  EXPECT_TRUE((mcpplibs::primitives::std_char<char>));
  EXPECT_TRUE((mcpplibs::primitives::std_char<char8_t>));
  EXPECT_TRUE((mcpplibs::primitives::std_char<wchar_t>));

  EXPECT_TRUE((mcpplibs::primitives::std_underlying_type<int>));
  EXPECT_TRUE((mcpplibs::primitives::std_underlying_type<double>));
  EXPECT_FALSE((mcpplibs::primitives::std_underlying_type<void*>));
}

TEST(PrimitiveTraitsTest, UnderlyingTraitsDefaultsAndCustomRegistration) {
  EXPECT_TRUE((mcpplibs::primitives::underlying_type<int>));
  EXPECT_EQ(mcpplibs::primitives::underlying::traits<int>::kind,
            mcpplibs::primitives::underlying::category::integer);

  EXPECT_TRUE((mcpplibs::primitives::underlying_type<UserInteger>));
  EXPECT_EQ(mcpplibs::primitives::underlying::traits<UserInteger>::to_rep(UserInteger{7}), 7);

  EXPECT_FALSE((mcpplibs::primitives::underlying_type<NotRegistered>));
  EXPECT_FALSE((mcpplibs::primitives::underlying::traits<NotRegistered>::enabled));
}

TEST(PrimitiveTraitsTest, UnderlyingTypeRequiresValidRepTypeAndCategoryConsistency) {
  EXPECT_TRUE((mcpplibs::primitives::underlying::traits<BadRep>::enabled));
  EXPECT_FALSE((mcpplibs::primitives::underlying_type<BadRep>));

  EXPECT_TRUE((mcpplibs::primitives::underlying::traits<BadKind>::enabled));
  EXPECT_FALSE((mcpplibs::primitives::underlying_type<BadKind>));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
