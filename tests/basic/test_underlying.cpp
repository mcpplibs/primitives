#include <gtest/gtest.h>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.operations.impl;

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

struct BigIntLike {
  long long value;

  friend constexpr auto operator+(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(BigIntLike lhs, BigIntLike rhs) noexcept
      -> BigIntLike {
    return BigIntLike{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(BigIntLike lhs, BigIntLike rhs) noexcept
      -> bool {
    return lhs.value == rhs.value;
  }
};

struct BadCustomKind {
  long long value;

  friend constexpr auto operator+(BadCustomKind lhs, BadCustomKind rhs) noexcept
      -> BadCustomKind {
    return BadCustomKind{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(BadCustomKind lhs, BadCustomKind rhs) noexcept
      -> BadCustomKind {
    return BadCustomKind{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(BadCustomKind lhs, BadCustomKind rhs) noexcept
      -> BadCustomKind {
    return BadCustomKind{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(BadCustomKind lhs, BadCustomKind rhs) noexcept
      -> BadCustomKind {
    return BadCustomKind{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(BadCustomKind lhs,
                                   BadCustomKind rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct MissingDivisionLike {
  long long value;

  friend constexpr auto operator+(MissingDivisionLike lhs,
                                  MissingDivisionLike rhs) noexcept
      -> MissingDivisionLike {
    return MissingDivisionLike{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(MissingDivisionLike lhs,
                                  MissingDivisionLike rhs) noexcept
      -> MissingDivisionLike {
    return MissingDivisionLike{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(MissingDivisionLike lhs,
                                  MissingDivisionLike rhs) noexcept
      -> MissingDivisionLike {
    return MissingDivisionLike{lhs.value * rhs.value};
  }

  friend constexpr auto operator==(MissingDivisionLike lhs,
                                   MissingDivisionLike rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct NonNegativeInt {
  int value;
};

struct ExplicitCommonLhs {
  int value;
};

struct ExplicitCommonRhs {
  int value;
};

struct ExplicitCommonRep {
  long long value;

  friend constexpr auto operator+(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(ExplicitCommonRep lhs,
                                  ExplicitCommonRep rhs) noexcept
      -> ExplicitCommonRep {
    return ExplicitCommonRep{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(ExplicitCommonRep lhs,
                                   ExplicitCommonRep rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct VoidCommonLhs {
  int value;

  friend constexpr auto operator+(VoidCommonLhs lhs, VoidCommonLhs rhs) noexcept
      -> VoidCommonLhs {
    return VoidCommonLhs{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(VoidCommonLhs lhs, VoidCommonLhs rhs) noexcept
      -> VoidCommonLhs {
    return VoidCommonLhs{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(VoidCommonLhs lhs, VoidCommonLhs rhs) noexcept
      -> VoidCommonLhs {
    return VoidCommonLhs{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(VoidCommonLhs lhs, VoidCommonLhs rhs) noexcept
      -> VoidCommonLhs {
    return VoidCommonLhs{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(VoidCommonLhs lhs,
                                   VoidCommonLhs rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

struct VoidCommonRhs {
  int value;

  friend constexpr auto operator+(VoidCommonRhs lhs, VoidCommonRhs rhs) noexcept
      -> VoidCommonRhs {
    return VoidCommonRhs{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(VoidCommonRhs lhs, VoidCommonRhs rhs) noexcept
      -> VoidCommonRhs {
    return VoidCommonRhs{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(VoidCommonRhs lhs, VoidCommonRhs rhs) noexcept
      -> VoidCommonRhs {
    return VoidCommonRhs{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(VoidCommonRhs lhs, VoidCommonRhs rhs) noexcept
      -> VoidCommonRhs {
    return VoidCommonRhs{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(VoidCommonRhs lhs,
                                   VoidCommonRhs rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
};

} // namespace

template <> struct mcpplibs::primitives::underlying::traits<UserInteger> {
  using value_type = UserInteger;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept {
    return value.value;
  }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return UserInteger{value};
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<BadRep> {
  using value_type = BadRep;
  using rep_type = BadRep;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<BadKind> {
  using value_type = BadKind;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::floating;

  static constexpr rep_type to_rep(value_type value) noexcept {
    return value.value;
  }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return BadKind{value};
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<BigIntLike> {
  using value_type = BigIntLike;
  using rep_type = BigIntLike;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<BadCustomKind> {
  using value_type = BadCustomKind;
  using rep_type = BadCustomKind;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::boolean;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <>
struct mcpplibs::primitives::underlying::traits<MissingDivisionLike> {
  using value_type = MissingDivisionLike;
  using rep_type = MissingDivisionLike;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<NonNegativeInt> {
  using value_type = NonNegativeInt;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept {
    return value.value;
  }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return NonNegativeInt{value};
  }

  static constexpr bool is_valid_rep(rep_type value) noexcept {
    return value >= 0;
  }
};

template <> struct mcpplibs::primitives::underlying::traits<ExplicitCommonLhs> {
  using value_type = ExplicitCommonLhs;
  using rep_type = ExplicitCommonLhs;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<ExplicitCommonRhs> {
  using value_type = ExplicitCommonRhs;
  using rep_type = ExplicitCommonRhs;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<ExplicitCommonLhs,
                                                           ExplicitCommonRhs> {
  using type = ExplicitCommonRep;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<ExplicitCommonRhs,
                                                           ExplicitCommonLhs> {
  using type = ExplicitCommonRep;
  static constexpr bool enabled = true;
};

template <> struct mcpplibs::primitives::underlying::traits<VoidCommonLhs> {
  using value_type = VoidCommonLhs;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept {
    return value.value;
  }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return VoidCommonLhs{value};
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<VoidCommonRhs> {
  using value_type = VoidCommonRhs;
  using rep_type = short;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr rep_type to_rep(value_type value) noexcept {
    return static_cast<rep_type>(value.value);
  }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return VoidCommonRhs{value};
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<int, short> {
  using type = void;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<short, int> {
  using type = void;
  static constexpr bool enabled = true;
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
  EXPECT_FALSE((mcpplibs::primitives::std_underlying_type<void *>));
}

TEST(PrimitiveTraitsTest, UnderlyingTraitsDefaultsAndCustomRegistration) {
  EXPECT_TRUE((mcpplibs::primitives::underlying_type<int>));
  EXPECT_EQ(mcpplibs::primitives::underlying::traits<int>::kind,
            mcpplibs::primitives::underlying::category::integer);

  EXPECT_TRUE((mcpplibs::primitives::underlying_type<UserInteger>));
  EXPECT_EQ(mcpplibs::primitives::underlying::traits<UserInteger>::to_rep(
                UserInteger{7}),
            7);

  EXPECT_FALSE((mcpplibs::primitives::underlying_type<NotRegistered>));
  EXPECT_FALSE(
      (mcpplibs::primitives::underlying::traits<NotRegistered>::enabled));
}

TEST(PrimitiveTraitsTest, MetaTraitsExposeValueTypeAndPrimitiveMetadata) {
  using value_t = mcpplibs::primitives::primitive<
      short, mcpplibs::primitives::policy::value::checked,
      mcpplibs::primitives::policy::type::compatible,
      mcpplibs::primitives::policy::error::expected,
      mcpplibs::primitives::policy::concurrency::fenced>;
  using traits_t = mcpplibs::primitives::meta::traits<value_t const &>;

  static_assert(
      std::same_as<typename traits_t::value_type, typename value_t::value_type>);
  static_assert(std::same_as<typename traits_t::policies, typename value_t::policies>);
  static_assert(std::same_as<typename traits_t::value_policy,
                             mcpplibs::primitives::policy::value::checked>);
  static_assert(std::same_as<typename traits_t::type_policy,
                             mcpplibs::primitives::policy::type::compatible>);
  static_assert(std::same_as<typename traits_t::error_policy,
                             mcpplibs::primitives::policy::error::expected>);
  static_assert(std::same_as<typename traits_t::concurrency_policy,
                             mcpplibs::primitives::policy::concurrency::fenced>);

  SUCCEED();
}

TEST(PrimitiveTraitsTest, MetaPrimitiveConceptsMatchUnderlyingCategory) {
  using boolean_t = mcpplibs::primitives::primitive<bool>;
  using character_t = mcpplibs::primitives::primitive<char>;
  using integer_t = mcpplibs::primitives::primitive<int>;
  using floating_t = mcpplibs::primitives::primitive<double>;

  static_assert(mcpplibs::primitives::meta::primitive_type<boolean_t const &>);
  static_assert(mcpplibs::primitives::meta::primitive_type<integer_t>);
  static_assert(!mcpplibs::primitives::meta::primitive_type<int>);

  static_assert(mcpplibs::primitives::meta::boolean<boolean_t>);
  static_assert(!mcpplibs::primitives::meta::boolean<integer_t>);

  static_assert(mcpplibs::primitives::meta::character<character_t>);
  static_assert(!mcpplibs::primitives::meta::character<boolean_t>);

  static_assert(mcpplibs::primitives::meta::integer<integer_t>);
  static_assert(!mcpplibs::primitives::meta::integer<character_t>);

  static_assert(mcpplibs::primitives::meta::floating<floating_t>);
  static_assert(!mcpplibs::primitives::meta::floating<integer_t>);

  static_assert(mcpplibs::primitives::meta::numeric<integer_t>);
  static_assert(mcpplibs::primitives::meta::numeric<floating_t>);
  static_assert(!mcpplibs::primitives::meta::numeric<boolean_t>);
  static_assert(!mcpplibs::primitives::meta::numeric<character_t>);
  static_assert(!mcpplibs::primitives::meta::numeric<int>);

  EXPECT_TRUE((mcpplibs::primitives::meta::boolean<boolean_t>));
  EXPECT_TRUE((mcpplibs::primitives::meta::character<character_t>));
  EXPECT_TRUE((mcpplibs::primitives::meta::integer<integer_t>));
  EXPECT_TRUE((mcpplibs::primitives::meta::floating<floating_t>));
}

TEST(PrimitiveTraitsTest, MetaPrimitiveLikeConceptsAcceptPrimitiveOrUnderlying) {
  using boolean_t = mcpplibs::primitives::primitive<bool>;
  using character_t = mcpplibs::primitives::primitive<char>;
  using integer_t = mcpplibs::primitives::primitive<int>;
  using floating_t = mcpplibs::primitives::primitive<double>;

  static_assert(mcpplibs::primitives::meta::primitive_like<boolean_t>);
  static_assert(mcpplibs::primitives::meta::primitive_like<int>);
  static_assert(!mcpplibs::primitives::meta::primitive_like<void *>);

  static_assert(mcpplibs::primitives::meta::boolean_like<boolean_t>);
  static_assert(mcpplibs::primitives::meta::boolean_like<bool>);
  static_assert(!mcpplibs::primitives::meta::boolean_like<int>);

  static_assert(mcpplibs::primitives::meta::character_like<character_t>);
  static_assert(mcpplibs::primitives::meta::character_like<char>);
  static_assert(!mcpplibs::primitives::meta::character_like<int>);

  static_assert(mcpplibs::primitives::meta::integer_like<integer_t>);
  static_assert(mcpplibs::primitives::meta::integer_like<int>);
  static_assert(!mcpplibs::primitives::meta::integer_like<char>);

  static_assert(mcpplibs::primitives::meta::floating_like<floating_t>);
  static_assert(mcpplibs::primitives::meta::floating_like<double>);
  static_assert(!mcpplibs::primitives::meta::floating_like<int>);

  static_assert(mcpplibs::primitives::meta::numeric_like<integer_t>);
  static_assert(mcpplibs::primitives::meta::numeric_like<floating_t>);
  static_assert(mcpplibs::primitives::meta::numeric_like<int>);
  static_assert(mcpplibs::primitives::meta::numeric_like<double>);
  static_assert(!mcpplibs::primitives::meta::numeric_like<bool>);
  static_assert(!mcpplibs::primitives::meta::numeric_like<char>);

  EXPECT_TRUE((mcpplibs::primitives::meta::primitive_like<boolean_t>));
  EXPECT_TRUE((mcpplibs::primitives::meta::primitive_like<int>));
}

TEST(PrimitiveTraitsTest,
     UnderlyingTypeRequiresValidRepTypeAndCategoryConsistency) {
  EXPECT_TRUE((mcpplibs::primitives::underlying::traits<BadRep>::enabled));
  EXPECT_FALSE((mcpplibs::primitives::underlying_type<BadRep>));

  EXPECT_TRUE((mcpplibs::primitives::underlying::traits<BadKind>::enabled));
  EXPECT_FALSE((mcpplibs::primitives::underlying_type<BadKind>));
}

TEST(PrimitiveTraitsTest, UnderlyingTypeAllowsCustomNumericLikeRepType) {
  EXPECT_TRUE((mcpplibs::primitives::underlying::traits<BigIntLike>::enabled));
  EXPECT_TRUE((mcpplibs::primitives::underlying_type<BigIntLike>));
}

TEST(PrimitiveTraitsTest, CustomNumericLikeRepTypeRejectsInvalidCategory) {
  EXPECT_TRUE(
      (mcpplibs::primitives::underlying::traits<BadCustomKind>::enabled));
  EXPECT_FALSE((mcpplibs::primitives::underlying_type<BadCustomKind>));
}

TEST(PrimitiveTraitsTest,
     CustomNumericLikeRepTypeRequiresDivisionOperatorForEligibility) {
  EXPECT_TRUE(
      (mcpplibs::primitives::underlying::traits<MissingDivisionLike>::enabled));
  EXPECT_FALSE((mcpplibs::primitives::underlying_type<MissingDivisionLike>));
}

TEST(PrimitiveTraitsTest, UnderlyingCommonRepDefaultsToStdCommonType) {
  static_assert(mcpplibs::primitives::has_common_rep<int, long long>);
  static_assert(std::same_as<mcpplibs::primitives::common_rep_t<int, long long>,
                             long long>);
  SUCCEED();
}

TEST(PrimitiveTraitsTest, UnderlyingCommonRepCanBeCustomizedViaTraits) {
  static_assert(mcpplibs::primitives::has_common_rep<ExplicitCommonLhs,
                                                     ExplicitCommonRhs>);
  static_assert(
      std::same_as<mcpplibs::primitives::common_rep_t<ExplicitCommonLhs,
                                                      ExplicitCommonRhs>,
                   ExplicitCommonRep>);

  using handler_t = mcpplibs::primitives::policy::type::handler<
      mcpplibs::primitives::policy::type::transparent,
      mcpplibs::primitives::operations::Addition, ExplicitCommonLhs,
      ExplicitCommonRhs>;

  static_assert(handler_t::enabled);
  static_assert(handler_t::allowed);
  static_assert(
      std::same_as<typename handler_t::common_rep, ExplicitCommonRep>);
  SUCCEED();
}

TEST(PrimitiveTraitsTest, TypePoliciesRequireNonVoidCommonRep) {
  static_assert(std::is_arithmetic_v<
                mcpplibs::primitives::underlying::traits<VoidCommonLhs>::rep_type>);
  static_assert(std::is_arithmetic_v<
                mcpplibs::primitives::underlying::traits<VoidCommonRhs>::rep_type>);

  using compatible_handler_t = mcpplibs::primitives::policy::type::handler<
      mcpplibs::primitives::policy::type::compatible,
      mcpplibs::primitives::operations::Addition, VoidCommonLhs, VoidCommonRhs>;
  using transparent_handler_t = mcpplibs::primitives::policy::type::handler<
      mcpplibs::primitives::policy::type::transparent,
      mcpplibs::primitives::operations::Addition, VoidCommonLhs, VoidCommonRhs>;

  static_assert(compatible_handler_t::enabled);
  static_assert(transparent_handler_t::enabled);
  static_assert(!compatible_handler_t::allowed);
  static_assert(!transparent_handler_t::allowed);
  static_assert(std::same_as<typename compatible_handler_t::common_rep, void>);
  static_assert(std::same_as<typename transparent_handler_t::common_rep, void>);

  EXPECT_EQ(compatible_handler_t::diagnostic_id, 2u);
  EXPECT_EQ(transparent_handler_t::diagnostic_id, 3u);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
