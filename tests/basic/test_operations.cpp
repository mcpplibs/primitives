#include <atomic>
#include <compare>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

import mcpplibs.primitives;

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

TEST(OperationsTest, AddReturnsExpectedPrimitive) {
  using lhs_t = primitive<int, policy::value::checked>;
  using rhs_t = primitive<int, policy::value::checked>;

  auto const lhs = lhs_t{10};
  auto const rhs = rhs_t{32};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, DivisionByZeroReturnsError) {
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{0};

  auto const result = operations::div(lhs, rhs);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), policy::error::kind::divide_by_zero);
}

TEST(OperationsTest, SaturatingAdditionClampsUnsignedOverflow) {
  using value_t = primitive<std::uint16_t, policy::value::saturating>;

  auto const lhs = value_t{static_cast<std::uint16_t>(65530)};
  auto const rhs = value_t{static_cast<std::uint16_t>(20)};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), static_cast<std::uint16_t>(65535));
}

TEST(OperationsTest, CheckedAdditionReportsUnsignedOverflow) {
  using value_t =
      primitive<std::uint16_t, policy::value::checked, policy::error::expected>;

  auto const lhs = value_t{static_cast<std::uint16_t>(65530)};
  auto const rhs = value_t{static_cast<std::uint16_t>(20)};

  auto const result = operations::add(lhs, rhs);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), policy::error::kind::overflow);
}

TEST(OperationsTest, UncheckedAdditionWrapsUnsignedOverflow) {
  using value_t = primitive<std::uint16_t, policy::value::unchecked,
                            policy::error::expected>;

  auto const lhs = value_t{static_cast<std::uint16_t>(65530)};
  auto const rhs = value_t{static_cast<std::uint16_t>(20)};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), static_cast<std::uint16_t>(14));
}

TEST(OperationsTest, UncheckedDivisionUsesRawArithmeticWhenValid) {
  using value_t =
      primitive<int, policy::value::unchecked, policy::error::expected>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{4};

  auto const result = operations::div(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 25);
}

TEST(OperationsTest, FencedPolicyPathReturnsExpectedValue) {
  using value_t =
      primitive<int, policy::value::checked, policy::concurrency::fenced,
                policy::error::expected>;

  auto const lhs = value_t{12};
  auto const rhs = value_t{30};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, FencedPolicyConcurrentInvocationsRemainConsistent) {
  using value_t =
      primitive<int, policy::value::checked, policy::concurrency::fenced,
                policy::error::expected>;

  constexpr int kThreadCount = 8;
  constexpr int kIterationsPerThread = 20000;

  auto const lhs = value_t{12};
  auto const rhs = value_t{30};

  std::atomic<int> mismatch_count{0};
  std::atomic<int> error_count{0};
  std::atomic<bool> start{false};

  std::vector<std::thread> workers;
  workers.reserve(kThreadCount);

  for (int i = 0; i < kThreadCount; ++i) {
    workers.emplace_back([&]() {
      while (!start.load(std::memory_order_acquire)) {
      }

      for (int n = 0; n < kIterationsPerThread; ++n) {
        auto const result = operations::add(lhs, rhs);
        if (!result.has_value()) {
          error_count.fetch_add(1, std::memory_order_relaxed);
          continue;
        }

        if (result->value() != 42) {
          mismatch_count.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  start.store(true, std::memory_order_release);

  for (auto &worker : workers) {
    worker.join();
  }

  EXPECT_EQ(error_count.load(std::memory_order_relaxed), 0);
  EXPECT_EQ(mismatch_count.load(std::memory_order_relaxed), 0);
}

TEST(OperationsTest, PrimitiveFencedLoadStoreAndCasWork) {
  using value_t =
      primitive<int, policy::value::checked, policy::concurrency::fenced,
                policy::error::expected>;

  auto value = value_t{1};
  EXPECT_EQ(value.load(), 1);

  value.store(4);
  EXPECT_EQ(value.load(), 4);

  auto expected = 4;
  EXPECT_TRUE(value.compare_exchange(expected, 7));
  EXPECT_EQ(value.load(), 7);

  expected = 9;
  EXPECT_FALSE(value.compare_exchange(expected, 11));
  EXPECT_EQ(expected, 7);
}

TEST(OperationsTest, PrimitiveFencedCasSupportsConcurrentIncrements) {
  using value_t =
      primitive<int, policy::value::checked, policy::concurrency::fenced,
                policy::error::expected>;

  constexpr int kThreadCount = 6;
  constexpr int kIterationsPerThread = 5000;

  auto counter = value_t{0};
  std::vector<std::thread> workers;
  workers.reserve(kThreadCount);

  for (int i = 0; i < kThreadCount; ++i) {
    workers.emplace_back([&]() {
      for (int n = 0; n < kIterationsPerThread; ++n) {
        auto expected = counter.load();
        while (!counter.compare_exchange(expected, expected + 1)) {
        }
      }
    });
  }

  for (auto &worker : workers) {
    worker.join();
  }

  EXPECT_EQ(counter.load(), kThreadCount * kIterationsPerThread);
}

TEST(OperationsTest,
     BinaryOperationsWithLoadStoreRemainStableUnderHighConcurrency) {
  using value_t =
      primitive<int, policy::value::checked, policy::concurrency::fenced,
                policy::error::expected>;

  constexpr int kWriterThreads = 6;
  constexpr int kReaderThreads = 8;
  constexpr int kIterationsPerThread = 25000;
  constexpr int kMaxOperand = 100000;

  auto lhs = value_t{0};
  auto rhs = value_t{0};
  auto sink = value_t{0};

  std::atomic<int> add_error_count{0};
  std::atomic<int> sub_error_count{0};
  std::atomic<int> range_violation_count{0};
  std::atomic<bool> start{false};

  std::vector<std::thread> workers;
  workers.reserve(kWriterThreads + kReaderThreads);

  for (int writer = 0; writer < kWriterThreads; ++writer) {
    workers.emplace_back([&, writer]() {
      while (!start.load(std::memory_order_acquire)) {
      }

      for (int n = 0; n < kIterationsPerThread; ++n) {
        auto const v1 = (writer + n) % (kMaxOperand + 1);
        auto const v2 = (writer * 3 + n * 7) % (kMaxOperand + 1);
        lhs.store(v1);
        rhs.store(v2);
      }
    });
  }

  for (int reader = 0; reader < kReaderThreads; ++reader) {
    workers.emplace_back([&, reader]() {
      while (!start.load(std::memory_order_acquire)) {
      }

      for (int n = 0; n < kIterationsPerThread; ++n) {
        if (((reader + n) & 1) == 0) {
          auto const out = operations::add(lhs, rhs);
          if (!out.has_value()) {
            add_error_count.fetch_add(1, std::memory_order_relaxed);
            continue;
          }

          auto const v = out->load();
          if (v < 0 || v > (kMaxOperand * 2)) {
            range_violation_count.fetch_add(1, std::memory_order_relaxed);
          }
          sink.store(v);
          auto const snapshot = sink.load();
          if (snapshot < -kMaxOperand || snapshot > (kMaxOperand * 2)) {
            range_violation_count.fetch_add(1, std::memory_order_relaxed);
          }
          continue;
        }

        auto const out = operations::sub(lhs, rhs);
        if (!out.has_value()) {
          sub_error_count.fetch_add(1, std::memory_order_relaxed);
          continue;
        }

        auto const v = out->load();
        if (v < -kMaxOperand || v > kMaxOperand) {
          range_violation_count.fetch_add(1, std::memory_order_relaxed);
        }
        sink.store(v);
        auto const snapshot = sink.load();
        if (snapshot < -kMaxOperand || snapshot > (kMaxOperand * 2)) {
          range_violation_count.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  start.store(true, std::memory_order_release);

  for (auto &worker : workers) {
    worker.join();
  }

  EXPECT_EQ(add_error_count.load(std::memory_order_relaxed), 0);
  EXPECT_EQ(sub_error_count.load(std::memory_order_relaxed), 0);
  EXPECT_EQ(range_violation_count.load(std::memory_order_relaxed), 0);
}

TEST(OperationsTest, PrimitiveSupportsCopyAndMoveSpecialMembers) {
  using value_t = primitive<int, policy::value::checked, policy::error::expected>;

  static_assert(std::is_copy_constructible_v<value_t>);
  static_assert(std::is_copy_assignable_v<value_t>);
  static_assert(std::is_move_constructible_v<value_t>);
  static_assert(std::is_move_assignable_v<value_t>);

  auto original = value_t{42};
  auto copy_constructed = value_t{original};
  EXPECT_EQ(copy_constructed.load(), 42);

  auto copy_assigned = value_t{0};
  copy_assigned = original;
  EXPECT_EQ(copy_assigned.load(), 42);

  auto move_constructed = value_t{std::move(copy_assigned)};
  EXPECT_EQ(move_constructed.load(), 42);

  auto move_assigned = value_t{0};
  move_assigned = std::move(move_constructed);
  EXPECT_EQ(move_assigned.load(), 42);
}

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

TEST(OperationsTest, PrimitiveSpecialMembersSupportCrossUnderlyingWithCompatibleType) {
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

TEST(OperationsTest, CompoundAssignmentOperatorsMutateLhsOnSuccess) {
  using namespace mcpplibs::primitives::operators;
  using value_t = primitive<int, policy::value::checked, policy::error::expected>;

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

TEST(OperationsTest, CompoundAssignmentSupportsMixedTypesWithCompatibleTypePolicy) {
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

TEST(OperationsTest, ThrowErrorPolicyThrowsException) {
  using value_t =
      primitive<int, policy::value::checked, policy::error::throwing>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{0};

  EXPECT_THROW((void)operations::div(lhs, rhs), std::runtime_error);
}

TEST(OperationsTest, ThrowErrorPolicyExceptionHasReasonMessage) {
  using value_t =
      primitive<int, policy::value::checked, policy::error::throwing>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{0};

  try {
    (void)operations::div(lhs, rhs);
    FAIL() << "Expected std::runtime_error to be thrown";
  } catch (std::runtime_error const &e) {
    EXPECT_NE(std::string(e.what()).find("division by zero"),
              std::string::npos);
  } catch (...) {
    FAIL() << "Expected std::runtime_error";
  }
}
