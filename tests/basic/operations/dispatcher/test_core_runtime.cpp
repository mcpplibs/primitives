#include <atomic>
#include <cstdint>
#include <gtest/gtest.h>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.operations.operators;

using namespace mcpplibs::primitives;

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
