#include <compare>
#include <gtest/gtest.h>
#include <limits>
#include <type_traits>
#include <utility>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.operations.dispatcher;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.operations.operators;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.policy;

using namespace mcpplibs::primitives;

template <typename Primitive, typename U>
concept constructible_from_underlying =
    underlying_type<U> && requires(U u) {
  Primitive{u};
};

template <typename Primitive, typename U>
concept storable_from_underlying =
    underlying_type<U> && requires(Primitive p, U u) {
  p.store(u);
};

template <typename Primitive, typename U>
concept cas_from_underlying =
    underlying_type<U> && requires(Primitive p, U expected, U desired) {
  { p.compare_exchange(expected, desired) } -> std::same_as<bool>;
};

TEST(OperationsTest, StrictTypeRejectsMixedTypesAtCompileTime) {
  using lhs_t = primitive<int, policy::value::checked, policy::type::strict,
                          policy::error::expected>;
  using rhs_t = primitive<long long, policy::value::checked,
                          policy::type::strict, policy::error::expected>;

  using strict_handler =
      policy::type::handler<policy::type::strict, operations::Addition, int,
                            long long>;
  using strict_meta = operations::dispatcher_meta<operations::Addition, lhs_t,
                                                  rhs_t, policy::error::kind>;

  static_assert(strict_handler::enabled);
  static_assert(!strict_handler::allowed);
  static_assert(std::is_same_v<typename strict_meta::common_rep, void>);

  EXPECT_EQ(strict_handler::diagnostic_id, 1u);
}

TEST(OperationsTest, StrictTypeAllowsSameTypeAtRuntime) {
  using value_t = primitive<int, policy::value::checked, policy::type::strict,
                            policy::error::expected>;

  auto const lhs = value_t{19};
  auto const rhs = value_t{23};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, StrictTypePrimitiveConstructorRejectsCrossUnderlyingType) {
  using strict_t = primitive<int, policy::type::strict, policy::error::expected>;

  static_assert(constructible_from_underlying<strict_t, int>);
  static_assert(!constructible_from_underlying<strict_t, short>);

  auto const value = strict_t{42};
  EXPECT_EQ(value.load(), 42);
}

TEST(OperationsTest, CompatibleTypePrimitiveConstructorAllowsSameCategory) {
  using compatible_t =
      primitive<int, policy::type::compatible, policy::error::expected>;

  static_assert(constructible_from_underlying<compatible_t, short>);
  static_assert(!constructible_from_underlying<compatible_t, double>);

  auto const value = compatible_t{static_cast<short>(42)};
  EXPECT_EQ(value.load(), 42);
}

TEST(OperationsTest, TransparentTypePrimitiveConstructorAllowsCrossCategory) {
  using transparent_t =
      primitive<int, policy::type::transparent, policy::error::expected>;

  static_assert(constructible_from_underlying<transparent_t, double>);

  auto const value = transparent_t{42.75};
  EXPECT_EQ(value.load(), 42);
}

TEST(OperationsTest, PrimitiveStoreAndCasRejectCrossUnderlyingWithStrictType) {
  using strict_t = primitive<int, policy::type::strict, policy::error::expected>;

  static_assert(!storable_from_underlying<strict_t, short>);
  static_assert(!cas_from_underlying<strict_t, short>);
}

TEST(OperationsTest, PrimitiveStoreAndCasAllowCompatibleSameCategory) {
  using compatible_t =
      primitive<int, policy::type::compatible, policy::error::expected>;

  static_assert(storable_from_underlying<compatible_t, short>);
  static_assert(cas_from_underlying<compatible_t, short>);
  static_assert(!storable_from_underlying<compatible_t, double>);
  static_assert(!cas_from_underlying<compatible_t, double>);

  auto value = compatible_t{10};
  value.store(static_cast<short>(12));
  EXPECT_EQ(value.load(), 12);

  short expected = 12;
  EXPECT_TRUE(value.compare_exchange(expected, static_cast<short>(20)));
  EXPECT_EQ(value.load(), 20);

  expected = 11;
  EXPECT_FALSE(value.compare_exchange(expected, static_cast<short>(30)));
  EXPECT_EQ(expected, 20);
}

TEST(OperationsTest, PrimitiveStoreAndCasAllowTransparentCrossCategory) {
  using transparent_t =
      primitive<int, policy::type::transparent, policy::error::expected>;

  static_assert(storable_from_underlying<transparent_t, double>);
  static_assert(cas_from_underlying<transparent_t, double>);

  auto value = transparent_t{10};
  value.store(42.75);
  EXPECT_EQ(value.load(), 42);

  double expected = 42.0;
  EXPECT_TRUE(value.compare_exchange(expected, 99.5));
  EXPECT_EQ(value.load(), 99);

  expected = 3.0;
  EXPECT_FALSE(value.compare_exchange(expected, 1.0));
  EXPECT_EQ(expected, 99.0);
}

TEST(OperationsTest, PrimitiveTransparentStoreClampsOutOfRangeFloatingInput) {
  using transparent_t =
      primitive<int, policy::type::transparent, policy::error::expected>;

  auto value = transparent_t{0};

  value.store(std::numeric_limits<double>::infinity());
  EXPECT_EQ(value.load(), std::numeric_limits<int>::max());

  value.store(-std::numeric_limits<double>::infinity());
  EXPECT_EQ(value.load(), std::numeric_limits<int>::lowest());

  value.store(std::numeric_limits<double>::quiet_NaN());
  EXPECT_EQ(value.load(), 0);
}

TEST(OperationsTest,
     PrimitiveSpecialMembersSupportCrossUnderlyingWithCompatibleType) {
  using dst_t =
      primitive<int, policy::type::compatible, policy::error::expected>;
  using src_t =
      primitive<short, policy::type::compatible, policy::error::expected>;

  static_assert(std::is_constructible_v<dst_t, src_t const &>);
  static_assert(std::is_constructible_v<dst_t, src_t &&>);
  static_assert(std::is_assignable_v<dst_t &, src_t const &>);
  static_assert(std::is_assignable_v<dst_t &, src_t &&>);

  auto const source = src_t{40};
  auto copy_constructed = dst_t{source};
  EXPECT_EQ(copy_constructed.load(), 40);

  auto move_constructed = dst_t{src_t{41}};
  EXPECT_EQ(move_constructed.load(), 41);

  auto copy_assigned = dst_t{0};
  copy_assigned = source;
  EXPECT_EQ(copy_assigned.load(), 40);

  auto move_assigned = dst_t{0};
  auto move_source = src_t{42};
  move_assigned = std::move(move_source);
  EXPECT_EQ(move_assigned.load(), 42);
}

TEST(OperationsTest,
     PrimitiveSpecialMembersRejectCrossUnderlyingWithStrictType) {
  using dst_t = primitive<int, policy::type::strict, policy::error::expected>;
  using src_t = primitive<short, policy::type::strict, policy::error::expected>;

  static_assert(!std::is_constructible_v<dst_t, src_t const &>);
  static_assert(!std::is_constructible_v<dst_t, src_t &&>);
  static_assert(!std::is_assignable_v<dst_t &, src_t const &>);
  static_assert(!std::is_assignable_v<dst_t &, src_t &&>);
}

TEST(OperationsTest, DispatcherMetaTracksResolvedPolicyGroupConsistency) {
  using aligned_lhs_t = primitive<int, policy::value::checked,
                                  policy::error::expected>;
  using aligned_rhs_t = primitive<int, policy::error::expected>;
  using aligned_meta = operations::dispatcher_meta<operations::Addition,
                                                   aligned_lhs_t, aligned_rhs_t,
                                                   policy::error::kind>;

  using mismatch_lhs_t = primitive<int, policy::value::checked,
                                   policy::error::expected>;
  using mismatch_rhs_t = primitive<int, policy::value::unchecked,
                                   policy::error::expected>;
  using mismatch_meta =
      operations::dispatcher_meta<operations::Addition, mismatch_lhs_t,
                                  mismatch_rhs_t, policy::error::kind>;

  static_assert(aligned_meta::policy_group_consistent);
  static_assert(!mismatch_meta::policy_group_consistent);

  EXPECT_TRUE(aligned_meta::policy_group_consistent);
  EXPECT_FALSE(mismatch_meta::policy_group_consistent);
}

TEST(OperationsTest, CompatibleTypeRequiresSameUnderlyingCategory) {
  using same_category_handler =
      policy::type::handler<policy::type::compatible, operations::Addition, int,
                            long long>;
  using cross_category_handler =
      policy::type::handler<policy::type::compatible, operations::Addition, int,
                            double>;

  static_assert(same_category_handler::enabled);
  static_assert(same_category_handler::allowed);
  static_assert(cross_category_handler::enabled);
  static_assert(!cross_category_handler::allowed);

  EXPECT_EQ(cross_category_handler::diagnostic_id, 2u);
}

TEST(OperationsTest, TransparentTypeIgnoresCategoryWhenCommonRepIsValid) {
  using transparent_handler =
      policy::type::handler<policy::type::transparent, operations::Addition,
                            int, double>;

  static_assert(transparent_handler::enabled);
  static_assert(transparent_handler::allowed);
  static_assert(
      std::is_same_v<typename transparent_handler::common_rep, double>);

  EXPECT_EQ(transparent_handler::diagnostic_id, 0u);
}

TEST(OperationsTest, BoolUnderlyingRejectsArithmeticOperationsAtCompileTime) {
  using value_t = primitive<bool, policy::value::checked, policy::type::strict,
                            policy::error::expected>;
  using bool_handler = policy::type::handler<policy::type::strict,
                                             operations::Addition, bool, bool>;
  using bool_meta = operations::dispatcher_meta<operations::Addition, value_t,
                                                value_t, policy::error::kind>;

  static_assert(bool_handler::enabled);
  static_assert(!bool_handler::allowed);
  static_assert(std::is_same_v<typename bool_meta::common_rep, void>);

  EXPECT_EQ(bool_handler::diagnostic_id, 3u);
}

TEST(OperationsTest, CharUnderlyingRejectsArithmeticEvenWithTransparentType) {
  using value_t = primitive<char, policy::value::checked,
                            policy::type::transparent, policy::error::expected>;
  using char_handler = policy::type::handler<policy::type::transparent,
                                             operations::Addition, char, char>;
  using char_meta = operations::dispatcher_meta<operations::Addition, value_t,
                                                value_t, policy::error::kind>;

  static_assert(char_handler::enabled);
  static_assert(!char_handler::allowed);
  static_assert(std::is_same_v<typename char_meta::common_rep, void>);

  EXPECT_EQ(char_handler::diagnostic_id, 3u);
}

TEST(OperationsTest, SignedAndUnsignedCharRejectArithmeticAtCompileTime) {
  using signed_handler =
      policy::type::handler<policy::type::transparent, operations::Addition,
                            signed char, signed char>;
  using unsigned_handler =
      policy::type::handler<policy::type::transparent, operations::Addition,
                            unsigned char, unsigned char>;

  static_assert(signed_handler::enabled);
  static_assert(!signed_handler::allowed);
  static_assert(unsigned_handler::enabled);
  static_assert(!unsigned_handler::allowed);

  EXPECT_EQ(signed_handler::diagnostic_id, 3u);
  EXPECT_EQ(unsigned_handler::diagnostic_id, 3u);
}

TEST(OperationsTest, BoolUnderlyingAllowsComparisonOperations) {
  using value_t = primitive<bool, policy::value::checked, policy::type::strict,
                            policy::error::expected>;

  auto const lhs = value_t{true};
  auto const rhs = value_t{false};

  auto const eq_result = operations::equal(lhs, rhs);
  auto const ne_result = operations::not_equal(lhs, rhs);

  ASSERT_TRUE(eq_result.has_value());
  ASSERT_TRUE(ne_result.has_value());
  EXPECT_FALSE(eq_result->value());
  EXPECT_TRUE(ne_result->value());
}

TEST(OperationsTest, CharUnderlyingAllowsComparisonWithTransparentType) {
  using value_t = primitive<char, policy::value::checked,
                            policy::type::transparent, policy::error::expected>;

  auto const lhs = value_t{'a'};
  auto const rhs = value_t{'a'};

  auto const eq_result = operations::equal(lhs, rhs);

  ASSERT_TRUE(eq_result.has_value());
  EXPECT_EQ(eq_result->value(), static_cast<char>(1));
}

TEST(OperationsTest, MixedBinaryOperationsSupportUnderlyingOnBothSides) {
  using value_t = primitive<int, policy::value::checked,
                            policy::type::compatible, policy::error::expected>;

  auto const lhs = value_t{40};
  short const rhs = 2;

  auto const apply_lr = operations::apply<operations::Addition>(lhs, rhs);
  auto const apply_rl = operations::apply<operations::Addition>(rhs, lhs);
  auto const add_lr = operations::add(lhs, rhs);
  auto const add_rl = operations::add(rhs, lhs);
  auto const sub_lr = operations::sub(lhs, rhs);
  auto const sub_rl = operations::sub(rhs, lhs);
  auto const div_lr = operations::div(lhs, rhs);
  auto const div_rl = operations::div(rhs, lhs);
  auto const cmp_lr = operations::three_way_compare(lhs, rhs);
  auto const cmp_rl = operations::three_way_compare(rhs, lhs);

  static_assert(std::is_same_v<typename decltype(apply_lr)::value_type, value_t>);
  static_assert(std::is_same_v<typename decltype(apply_rl)::value_type, value_t>);
  static_assert(std::is_same_v<typename decltype(add_lr)::value_type, value_t>);
  static_assert(std::is_same_v<typename decltype(add_rl)::value_type, value_t>);
  static_assert(std::is_same_v<typename decltype(cmp_lr)::value_type,
                               std::strong_ordering>);
  static_assert(std::is_same_v<typename decltype(cmp_rl)::value_type,
                               std::strong_ordering>);

  ASSERT_TRUE(apply_lr.has_value());
  ASSERT_TRUE(apply_rl.has_value());
  ASSERT_TRUE(add_lr.has_value());
  ASSERT_TRUE(add_rl.has_value());
  ASSERT_TRUE(sub_lr.has_value());
  ASSERT_TRUE(sub_rl.has_value());
  ASSERT_TRUE(div_lr.has_value());
  ASSERT_TRUE(div_rl.has_value());
  ASSERT_TRUE(cmp_lr.has_value());
  ASSERT_TRUE(cmp_rl.has_value());

  EXPECT_EQ(apply_lr->value(), 42);
  EXPECT_EQ(apply_rl->value(), 42);
  EXPECT_EQ(add_lr->value(), 42);
  EXPECT_EQ(add_rl->value(), 42);
  EXPECT_EQ(sub_lr->value(), 38);
  EXPECT_EQ(sub_rl->value(), -38);
  EXPECT_EQ(div_lr->value(), 20);
  EXPECT_EQ(div_rl->value(), 0);
  EXPECT_EQ(*cmp_lr, std::strong_ordering::greater);
  EXPECT_EQ(*cmp_rl, std::strong_ordering::less);
}
