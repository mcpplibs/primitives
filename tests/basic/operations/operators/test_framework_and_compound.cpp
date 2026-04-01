#include <compare>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <type_traits>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.operations.operators;

using namespace mcpplibs::primitives;

TEST(OperationsTest, PrimitiveAliasWorksWithFrameworkOperators) {
  using namespace mcpplibs::primitives::types;
  using namespace mcpplibs::primitives::operators;
  using value_t = I32<policy::value::checked, policy::error::expected>;

  auto const lhs = value_t{20};
  auto const rhs = value_t{22};

  auto const result = lhs + rhs;

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, PrimitiveAliasMixesWithBuiltinArithmeticExplicitly) {
  using namespace mcpplibs::primitives::types;
  using namespace mcpplibs::primitives::operators;
  using value_t = I32<policy::value::checked, policy::error::expected>;

  static_assert(!std::is_convertible_v<value_t, int>);

  auto const lhs = value_t{40};
  auto const mixed = static_cast<int>(lhs) + 2;
  EXPECT_EQ(mixed, 42);

  auto const wrapped = value_t{mixed};
  auto const result = wrapped + value_t{1};

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 43);
}

TEST(OperationsTest, MixedFrameworkOperatorsSupportUnderlyingOnBothSides) {
  using namespace mcpplibs::primitives::operators;
  using value_t = primitive<int, policy::value::checked,
                            policy::type::compatible, policy::error::expected>;

  auto const lhs = value_t{40};
  short const rhs = 2;

  auto const add_lr = lhs + rhs;
  auto const add_rl = rhs + lhs;
  auto const sub_lr = lhs - rhs;
  auto const sub_rl = rhs - lhs;
  auto const eq_lr = lhs == static_cast<short>(40);
  auto const eq_rl = static_cast<short>(40) == lhs;
  auto const ne_lr = lhs != static_cast<short>(41);
  auto const ne_rl = static_cast<short>(41) != lhs;
  auto const cmp_lr = lhs <=> rhs;
  auto const cmp_rl = rhs <=> lhs;

  static_assert(std::is_same_v<typename decltype(cmp_lr)::value_type,
                               std::strong_ordering>);
  static_assert(std::is_same_v<typename decltype(cmp_rl)::value_type,
                               std::strong_ordering>);

  ASSERT_TRUE(add_lr.has_value());
  ASSERT_TRUE(add_rl.has_value());
  ASSERT_TRUE(sub_lr.has_value());
  ASSERT_TRUE(sub_rl.has_value());
  ASSERT_TRUE(eq_lr.has_value());
  ASSERT_TRUE(eq_rl.has_value());
  ASSERT_TRUE(ne_lr.has_value());
  ASSERT_TRUE(ne_rl.has_value());
  ASSERT_TRUE(cmp_lr.has_value());
  ASSERT_TRUE(cmp_rl.has_value());

  EXPECT_EQ(add_lr->value(), 42);
  EXPECT_EQ(add_rl->value(), 42);
  EXPECT_EQ(sub_lr->value(), 38);
  EXPECT_EQ(sub_rl->value(), -38);
  EXPECT_EQ(eq_lr->value(), 1);
  EXPECT_EQ(eq_rl->value(), 1);
  EXPECT_EQ(ne_lr->value(), 1);
  EXPECT_EQ(ne_rl->value(), 1);
  EXPECT_EQ(*cmp_lr, std::strong_ordering::greater);
  EXPECT_EQ(*cmp_rl, std::strong_ordering::less);
}

TEST(OperationsTest, OperatorEqualDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t = primitive<int, policy::value::checked, policy::type::strict,
                            policy::error::expected>;

  auto const lhs = value_t{7};
  auto const rhs = value_t{7};

  auto const result = (lhs == rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 1);
}

TEST(OperationsTest, OperatorPlusDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t = primitive<int, policy::value::checked>;

  auto const lhs = value_t{7};
  auto const rhs = value_t{8};

  auto const result = lhs + rhs;

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 15);
}

TEST(OperationsTest, UnaryOperatorsDelegateToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto value = value_t{10};

  auto const inc = ++value;
  ASSERT_TRUE(inc.has_value());
  EXPECT_EQ(inc->value(), 11);
  EXPECT_EQ(value.load(), 11);

  auto const dec = --value;
  ASSERT_TRUE(dec.has_value());
  EXPECT_EQ(dec->value(), 10);
  EXPECT_EQ(value.load(), 10);

  auto const post_inc = value++;
  ASSERT_TRUE(post_inc.has_value());
  EXPECT_EQ(post_inc->value(), 10);
  EXPECT_EQ(value.load(), 11);

  auto const post_dec = value--;
  ASSERT_TRUE(post_dec.has_value());
  EXPECT_EQ(post_dec->value(), 11);
  EXPECT_EQ(value.load(), 10);

  auto const pos = +value;
  auto const neg = -value;
  auto const inv = ~value;

  ASSERT_TRUE(pos.has_value());
  ASSERT_TRUE(neg.has_value());
  ASSERT_TRUE(inv.has_value());
  EXPECT_EQ(pos->value(), 10);
  EXPECT_EQ(neg->value(), -10);
  EXPECT_EQ(inv->value(), ~10);
}

TEST(OperationsTest, NewBinaryOperatorsDelegateToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const lhs = value_t{12};
  auto const rhs = value_t{5};

  auto const mod = lhs % rhs;
  auto const shl = rhs << value_t{1};
  auto const shr = lhs >> value_t{2};
  auto const bit_and = lhs & value_t{10};
  auto const bit_or = lhs | value_t{10};
  auto const bit_xor = lhs ^ value_t{10};

  ASSERT_TRUE(mod.has_value());
  ASSERT_TRUE(shl.has_value());
  ASSERT_TRUE(shr.has_value());
  ASSERT_TRUE(bit_and.has_value());
  ASSERT_TRUE(bit_or.has_value());
  ASSERT_TRUE(bit_xor.has_value());
  EXPECT_EQ(mod->value(), 2);
  EXPECT_EQ(shl->value(), 10);
  EXPECT_EQ(shr->value(), 3);
  EXPECT_EQ(bit_and->value(), 8);
  EXPECT_EQ(bit_or->value(), 14);
  EXPECT_EQ(bit_xor->value(), 6);
}

TEST(OperationsTest, NewCompoundAssignmentOperatorsMutateLhsOnSuccess) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto value = value_t{12};

  auto mod = (value %= value_t{5});
  ASSERT_TRUE(mod.has_value());
  EXPECT_EQ(value.load(), 2);

  auto shl = (value <<= value_t{2});
  ASSERT_TRUE(shl.has_value());
  EXPECT_EQ(value.load(), 8);

  auto shr = (value >>= value_t{1});
  ASSERT_TRUE(shr.has_value());
  EXPECT_EQ(value.load(), 4);

  auto bit_and = (value &= value_t{6});
  ASSERT_TRUE(bit_and.has_value());
  EXPECT_EQ(value.load(), 4);

  auto bit_or = (value |= value_t{1});
  ASSERT_TRUE(bit_or.has_value());
  EXPECT_EQ(value.load(), 5);

  auto bit_xor = (value ^= value_t{7});
  ASSERT_TRUE(bit_xor.has_value());
  EXPECT_EQ(value.load(), 2);
}

TEST(OperationsTest, ModulusAndShiftReportExpectedErrors) {
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const mod_zero = operations::mod(value_t{7}, value_t{0});
  ASSERT_FALSE(mod_zero.has_value());
  EXPECT_EQ(mod_zero.error(), policy::error::kind::divide_by_zero);

  auto const invalid_shift = operations::shift_left(
      value_t{1}, value_t{std::numeric_limits<int>::digits + 1});
  ASSERT_FALSE(invalid_shift.has_value());
  EXPECT_EQ(invalid_shift.error(), policy::error::kind::domain_error);
}

TEST(OperationsTest, ThreeWayCompareReturnsStrongOrderingForIntegers) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const less = (value_t{3} <=> value_t{7});
  auto const equal = (value_t{5} <=> value_t{5});
  auto const greater = (value_t{9} <=> value_t{1});

  static_assert(
      std::is_same_v<typename decltype(less)::value_type, std::strong_ordering>);

  ASSERT_TRUE(less.has_value());
  ASSERT_TRUE(equal.has_value());
  ASSERT_TRUE(greater.has_value());
  EXPECT_EQ(*less, std::strong_ordering::less);
  EXPECT_EQ(*equal, std::strong_ordering::equal);
  EXPECT_EQ(*greater, std::strong_ordering::greater);
}

TEST(OperationsTest, ThreeWayCompareReturnsPartialOrderingForFloatingPoint) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<double, policy::value::checked, policy::error::expected>;

  auto const less = (value_t{1.0} <=> value_t{2.0});
  auto const nan = std::numeric_limits<double>::quiet_NaN();
  auto const unordered = (value_t{nan} <=> value_t{1.0});

  static_assert(std::is_same_v<typename decltype(less)::value_type,
                               std::partial_ordering>);

  ASSERT_TRUE(less.has_value());
  ASSERT_TRUE(unordered.has_value());
  EXPECT_EQ(*less, std::partial_ordering::less);
  EXPECT_EQ(*unordered, std::partial_ordering::unordered);
}

TEST(OperationsTest, ThreeWayCompareOnBoolReturnsError) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<bool, policy::value::checked, policy::error::expected>;

  auto const result = (value_t{false} <=> value_t{true});

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), policy::error::kind::unspecified);
}

TEST(OperationsTest, OrderedComparisonOnBoolReturnsError) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<bool, policy::value::checked, policy::error::expected>;

  auto const less_result = (value_t{false} < value_t{true});
  auto const greater_result = (value_t{true} > value_t{false});
  auto const less_or_equal_result = (value_t{false} <= value_t{true});
  auto const greater_or_equal_result = (value_t{true} >= value_t{false});

  ASSERT_FALSE(less_result.has_value());
  ASSERT_FALSE(greater_result.has_value());
  ASSERT_FALSE(less_or_equal_result.has_value());
  ASSERT_FALSE(greater_or_equal_result.has_value());
  EXPECT_EQ(less_result.error(), policy::error::kind::unspecified);
  EXPECT_EQ(greater_result.error(), policy::error::kind::unspecified);
  EXPECT_EQ(less_or_equal_result.error(), policy::error::kind::unspecified);
  EXPECT_EQ(greater_or_equal_result.error(), policy::error::kind::unspecified);
}

TEST(OperationsTest, CompoundAssignmentOperatorsMutateLhsOnSuccess) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto value = value_t{20};

  auto add_result = (value += value_t{22});
  ASSERT_TRUE(add_result.has_value());
  EXPECT_EQ(value.load(), 42);
  EXPECT_EQ(add_result->value(), 42);

  auto sub_result = (value -= value_t{2});
  ASSERT_TRUE(sub_result.has_value());
  EXPECT_EQ(value.load(), 40);
  EXPECT_EQ(sub_result->value(), 40);

  auto mul_result = (value *= value_t{3});
  ASSERT_TRUE(mul_result.has_value());
  EXPECT_EQ(value.load(), 120);
  EXPECT_EQ(mul_result->value(), 120);

  auto div_result = (value /= value_t{4});
  ASSERT_TRUE(div_result.has_value());
  EXPECT_EQ(value.load(), 30);
  EXPECT_EQ(div_result->value(), 30);
}

TEST(OperationsTest, CompoundAssignmentKeepsLhsWhenOperationFails) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto value = value_t{100};

  auto result = (value /= value_t{0});

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), policy::error::kind::divide_by_zero);
  EXPECT_EQ(value.load(), 100);
}

TEST(OperationsTest,
     CompoundAssignmentCheckedRejectsFloatingToIntegralOutOfRange) {
  using lhs_t = primitive<int, policy::value::checked, policy::type::transparent,
                          policy::error::expected>;
  using rhs_t =
      primitive<double, policy::value::checked, policy::type::transparent,
                policy::error::expected>;

  auto value = lhs_t{7};

  auto const overflow_result =
      operations::add_assign(value, rhs_t{std::numeric_limits<double>::max()});
  ASSERT_FALSE(overflow_result.has_value());
  EXPECT_EQ(overflow_result.error(), policy::error::kind::overflow);
  EXPECT_EQ(value.load(), 7);

  auto const domain_result = operations::add_assign(
      value, rhs_t{std::numeric_limits<double>::quiet_NaN()});
  ASSERT_FALSE(domain_result.has_value());
  EXPECT_EQ(domain_result.error(), policy::error::kind::domain_error);
  EXPECT_EQ(value.load(), 7);
}

TEST(OperationsTest,
     CompoundAssignmentSupportsMixedTypesWithCompatibleTypePolicy) {
  using namespace mcpplibs::primitives::operators;

  using lhs_t = primitive<short, policy::value::checked, policy::error::expected,
                          policy::type::compatible>;
  using rhs_t = primitive<int, policy::value::checked, policy::error::expected,
                          policy::type::compatible>;

  auto value = lhs_t{10};

  auto add_result = (value += rhs_t{32});

  ASSERT_TRUE(add_result.has_value());
  EXPECT_EQ(value.load(), 42);
  EXPECT_EQ(add_result->value(), 42);
}

TEST(OperationsTest,
     CompoundAssignmentKeepsLhsOnMixedTypeOverflowWithCompatibleTypePolicy) {
  using namespace mcpplibs::primitives::operators;

  using lhs_t = primitive<std::int16_t, policy::value::checked,
                          policy::error::expected, policy::type::compatible>;
  using rhs_t = primitive<int, policy::value::checked, policy::error::expected,
                          policy::type::compatible>;

  auto value = lhs_t{static_cast<std::int16_t>(32000)};

  auto add_result = (value += rhs_t{1000});

  ASSERT_FALSE(add_result.has_value());
  EXPECT_EQ(add_result.error(), policy::error::kind::overflow);
  EXPECT_EQ(value.load(), static_cast<std::int16_t>(32000));
}

TEST(OperationsTest, LessThanReturnsCorrectResultForIntegers) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const less_result = operations::less_than(value_t{3}, value_t{7});
  auto const greater_result = operations::less_than(value_t{9}, value_t{1});
  auto const equal_result = operations::less_than(value_t{5}, value_t{5});

  ASSERT_TRUE(less_result.has_value());
  ASSERT_TRUE(greater_result.has_value());
  ASSERT_TRUE(equal_result.has_value());
  EXPECT_EQ(less_result->value(), 1);
  EXPECT_EQ(greater_result->value(), 0);
  EXPECT_EQ(equal_result->value(), 0);
}

TEST(OperationsTest, GreaterThanReturnsCorrectResultForIntegers) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const less_result = operations::greater_than(value_t{3}, value_t{7});
  auto const greater_result = operations::greater_than(value_t{9}, value_t{1});
  auto const equal_result = operations::greater_than(value_t{5}, value_t{5});

  ASSERT_TRUE(less_result.has_value());
  ASSERT_TRUE(greater_result.has_value());
  ASSERT_TRUE(equal_result.has_value());
  EXPECT_EQ(less_result->value(), 0);
  EXPECT_EQ(greater_result->value(), 1);
  EXPECT_EQ(equal_result->value(), 0);
}

TEST(OperationsTest, LessThanOrEqualReturnsCorrectResultForIntegers) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const less_result = operations::less_than_or_equal(value_t{3}, value_t{7});
  auto const greater_result = operations::less_than_or_equal(value_t{9}, value_t{1});
  auto const equal_result = operations::less_than_or_equal(value_t{5}, value_t{5});

  ASSERT_TRUE(less_result.has_value());
  ASSERT_TRUE(greater_result.has_value());
  ASSERT_TRUE(equal_result.has_value());
  EXPECT_EQ(less_result->value(), 1);
  EXPECT_EQ(greater_result->value(), 0);
  EXPECT_EQ(equal_result->value(), 1);
}

TEST(OperationsTest, GreaterThanOrEqualReturnsCorrectResultForIntegers) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const less_result = operations::greater_than_or_equal(value_t{3}, value_t{7});
  auto const greater_result = operations::greater_than_or_equal(value_t{9}, value_t{1});
  auto const equal_result = operations::greater_than_or_equal(value_t{5}, value_t{5});

  ASSERT_TRUE(less_result.has_value());
  ASSERT_TRUE(greater_result.has_value());
  ASSERT_TRUE(equal_result.has_value());
  EXPECT_EQ(less_result->value(), 0);
  EXPECT_EQ(greater_result->value(), 1);
  EXPECT_EQ(equal_result->value(), 1);
}

TEST(OperationsTest, OperatorLessThanDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const result = (value_t{3} < value_t{7});

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 1);
}

TEST(OperationsTest, OperatorGreaterThanDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const result = (value_t{9} > value_t{1});

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 1);
}

TEST(OperationsTest, OperatorLessThanOrEqualDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const less_result = (value_t{3} <= value_t{7});
  auto const equal_result = (value_t{5} <= value_t{5});

  ASSERT_TRUE(less_result.has_value());
  ASSERT_TRUE(equal_result.has_value());
  EXPECT_EQ(less_result->value(), 1);
  EXPECT_EQ(equal_result->value(), 1);
}

TEST(OperationsTest, OperatorGreaterThanOrEqualDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const greater_result = (value_t{9} >= value_t{1});
  auto const equal_result = (value_t{5} >= value_t{5});

  ASSERT_TRUE(greater_result.has_value());
  ASSERT_TRUE(equal_result.has_value());
  EXPECT_EQ(greater_result->value(), 1);
  EXPECT_EQ(equal_result->value(), 1);
}

TEST(OperationsTest, FourWayComparisonOperatorsWorkTogether) {
  using namespace mcpplibs::primitives::operators;
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const a = value_t{10};
  auto const b = value_t{20};
  auto const c = value_t{10};

  auto const a_lt_b = (a < b);
  auto const a_gt_b = (a > b);
  auto const a_le_b = (a <= b);
  auto const a_ge_b = (a >= b);
  auto const a_lt_c = (a < c);
  auto const a_le_c = (a <= c);
  auto const a_ge_c = (a >= c);
  auto const a_gt_c = (a > c);

  ASSERT_TRUE(a_lt_b.has_value());
  ASSERT_TRUE(a_gt_b.has_value());
  ASSERT_TRUE(a_le_b.has_value());
  ASSERT_TRUE(a_ge_b.has_value());
  ASSERT_TRUE(a_lt_c.has_value());
  ASSERT_TRUE(a_le_c.has_value());
  ASSERT_TRUE(a_ge_c.has_value());
  ASSERT_TRUE(a_gt_c.has_value());

  EXPECT_EQ(a_lt_b->value(), 1);
  EXPECT_EQ(a_gt_b->value(), 0);
  EXPECT_EQ(a_le_b->value(), 1);
  EXPECT_EQ(a_ge_b->value(), 0);
  EXPECT_EQ(a_lt_c->value(), 0);
  EXPECT_EQ(a_le_c->value(), 1);
  EXPECT_EQ(a_ge_c->value(), 1);
  EXPECT_EQ(a_gt_c->value(), 0);
}

TEST(OperationsTest, ComparisonOperatorsWorkWithMixedTypes) {
  using namespace mcpplibs::primitives::operators;
  using value_t = primitive<int, policy::value::checked, policy::error::expected,
                           policy::type::compatible>;

  auto const lhs = value_t{10};
  short const rhs = 20;

  auto const lt_lr = lhs < rhs;
  auto const lt_rl = rhs < lhs;
  auto const gt_lr = lhs > rhs;
  auto const gt_rl = rhs > lhs;
  auto const le_lr = lhs <= rhs;
  auto const le_rl = rhs <= lhs;
  auto const ge_lr = lhs >= rhs;
  auto const ge_rl = rhs >= lhs;

  ASSERT_TRUE(lt_lr.has_value());
  ASSERT_TRUE(lt_rl.has_value());
  ASSERT_TRUE(gt_lr.has_value());
  ASSERT_TRUE(gt_rl.has_value());
  ASSERT_TRUE(le_lr.has_value());
  ASSERT_TRUE(le_rl.has_value());
  ASSERT_TRUE(ge_lr.has_value());
  ASSERT_TRUE(ge_rl.has_value());

  EXPECT_EQ(lt_lr->value(), 1);
  EXPECT_EQ(lt_rl->value(), 0);
  EXPECT_EQ(gt_lr->value(), 0);
  EXPECT_EQ(gt_rl->value(), 1);
  EXPECT_EQ(le_lr->value(), 1);
  EXPECT_EQ(le_rl->value(), 0);
  EXPECT_EQ(ge_lr->value(), 0);
  EXPECT_EQ(ge_rl->value(), 1);
}
