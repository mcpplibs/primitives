#include <gtest/gtest.h>

import mcpplibs.primitive;

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
struct mcpplibs::primitive::underlying::traits<UserInteger> {
  using value_type = UserInteger;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = mcpplibs::primitive::underlying::category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value.value; }

  static constexpr value_type from_rep(rep_type value) noexcept { return UserInteger{value}; }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <>
struct mcpplibs::primitive::underlying::traits<BadRep> {
  using value_type = BadRep;
  using rep_type = BadRep;

  static constexpr bool enabled = true;
  static constexpr auto kind = mcpplibs::primitive::underlying::category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept { return value; }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <>
struct mcpplibs::primitive::underlying::traits<BadKind> {
  using value_type = BadKind;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = mcpplibs::primitive::underlying::category::floating;

  static constexpr rep_type to_rep(value_type value) noexcept { return value.value; }

  static constexpr value_type from_rep(rep_type value) noexcept { return BadKind{value}; }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

TEST(PrimitiveTraitsTest, StandardTypeConcepts) {
  EXPECT_TRUE((mcpplibs::primitive::std_integer<int>));
  EXPECT_TRUE((mcpplibs::primitive::std_integer<long long>));
  EXPECT_FALSE((mcpplibs::primitive::std_integer<bool>));
  EXPECT_FALSE((mcpplibs::primitive::std_integer<char>));

  EXPECT_TRUE((mcpplibs::primitive::std_floating<float>));
  EXPECT_TRUE((mcpplibs::primitive::std_floating<double>));
  EXPECT_TRUE((mcpplibs::primitive::std_floating<long double>));

  EXPECT_TRUE((mcpplibs::primitive::std_bool<bool>));
  EXPECT_FALSE((mcpplibs::primitive::std_bool<int>));

  EXPECT_TRUE((mcpplibs::primitive::std_char<char>));
  EXPECT_TRUE((mcpplibs::primitive::std_char<char8_t>));
  EXPECT_TRUE((mcpplibs::primitive::std_char<wchar_t>));

  EXPECT_TRUE((mcpplibs::primitive::std_underlying_type<int>));
  EXPECT_TRUE((mcpplibs::primitive::std_underlying_type<double>));
  EXPECT_FALSE((mcpplibs::primitive::std_underlying_type<void*>));
}

TEST(PrimitiveTraitsTest, UnderlyingTraitsDefaultsAndCustomRegistration) {
  EXPECT_TRUE((mcpplibs::primitive::underlying_type<int>));
  EXPECT_EQ(mcpplibs::primitive::underlying::traits<int>::kind,
            mcpplibs::primitive::underlying::category::integer);

  EXPECT_TRUE((mcpplibs::primitive::underlying_type<UserInteger>));
  EXPECT_EQ(mcpplibs::primitive::underlying::traits<UserInteger>::to_rep(UserInteger{7}), 7);

  EXPECT_FALSE((mcpplibs::primitive::underlying_type<NotRegistered>));
  EXPECT_FALSE((mcpplibs::primitive::underlying::traits<NotRegistered>::enabled));
}

TEST(PrimitiveTraitsTest, UnderlyingTypeRequiresValidRepTypeAndCategoryConsistency) {
  EXPECT_TRUE((mcpplibs::primitive::underlying::traits<BadRep>::enabled));
  EXPECT_FALSE((mcpplibs::primitive::underlying_type<BadRep>));

  EXPECT_TRUE((mcpplibs::primitive::underlying::traits<BadKind>::enabled));
  EXPECT_FALSE((mcpplibs::primitive::underlying_type<BadKind>));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
