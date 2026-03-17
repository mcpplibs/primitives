#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

TEST(OperationsTest, AddReturnsExpectedPrimitive) {
  using lhs_t = primitive<int, policy::checked_value>;
  using rhs_t = primitive<int, policy::checked_value>;

  auto const lhs = lhs_t{10};
  auto const rhs = rhs_t{32};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, DivisionByZeroReturnsError) {
  using value_t = primitive<int, policy::checked_value, policy::expected_error>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{0};

  auto const result = operations::div(lhs, rhs);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), policy::runtime_error_kind::divide_by_zero);
}

TEST(OperationsTest, SaturatingAdditionClampsUnsignedOverflow) {
  using value_t = primitive<std::uint16_t, policy::saturating_value>;

  auto const lhs = value_t{static_cast<std::uint16_t>(65530)};
  auto const rhs = value_t{static_cast<std::uint16_t>(20)};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), static_cast<std::uint16_t>(65535));
}

TEST(OperationsTest, CheckedAdditionReportsUnsignedOverflow) {
  using value_t =
      primitive<std::uint16_t, policy::checked_value, policy::expected_error>;

  auto const lhs = value_t{static_cast<std::uint16_t>(65530)};
  auto const rhs = value_t{static_cast<std::uint16_t>(20)};

  auto const result = operations::add(lhs, rhs);

  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), policy::runtime_error_kind::overflow);
}

TEST(OperationsTest, UncheckedAdditionWrapsUnsignedOverflow) {
  using value_t =
      primitive<std::uint16_t, policy::unchecked_value, policy::expected_error>;

  auto const lhs = value_t{static_cast<std::uint16_t>(65530)};
  auto const rhs = value_t{static_cast<std::uint16_t>(20)};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), static_cast<std::uint16_t>(14));
}

TEST(OperationsTest, UncheckedDivisionUsesRawArithmeticWhenValid) {
  using value_t =
      primitive<int, policy::unchecked_value, policy::expected_error>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{4};

  auto const result = operations::div(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 25);
}

TEST(OperationsTest, AtomicPolicyPathReturnsExpectedValue) {
  using value_t = primitive<int, policy::checked_value, policy::atomic,
                            policy::expected_error>;

  auto const lhs = value_t{12};
  auto const rhs = value_t{30};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, AtomicPolicyConcurrentInvocationsRemainConsistent) {
  using value_t = primitive<int, policy::checked_value, policy::atomic,
                            policy::expected_error>;

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

TEST(OperationsTest, StrictTypeRejectsMixedTypesAtCompileTime) {
  using lhs_t = primitive<int, policy::checked_value, policy::strict_type,
                          policy::expected_error>;
  using rhs_t = primitive<long long, policy::checked_value, policy::strict_type,
                          policy::expected_error>;

  using strict_handler =
      policy::type_handler<policy::strict_type, operations::Addition, int,
                           long long>;
  using strict_meta =
      operations::dispatcher_meta<operations::Addition, lhs_t, rhs_t,
                                  policy::runtime_error_kind>;

  static_assert(strict_handler::enabled);
  static_assert(!strict_handler::allowed);
  static_assert(std::is_same_v<typename strict_meta::common_rep, void>);

  EXPECT_EQ(strict_handler::diagnostic_id, 1u);
}

TEST(OperationsTest, StrictTypeAllowsSameTypeAtRuntime) {
  using value_t = primitive<int, policy::checked_value, policy::strict_type,
                            policy::expected_error>;

  auto const lhs = value_t{19};
  auto const rhs = value_t{23};

  auto const result = operations::add(lhs, rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, BoolUnderlyingRejectsArithmeticOperationsAtCompileTime) {
  using value_t = primitive<bool, policy::checked_value, policy::strict_type,
                            policy::expected_error>;
  using bool_handler = policy::type_handler<policy::strict_type,
                                            operations::Addition, bool, bool>;
  using bool_meta =
      operations::dispatcher_meta<operations::Addition, value_t, value_t,
                                  policy::runtime_error_kind>;

  static_assert(bool_handler::enabled);
  static_assert(!bool_handler::allowed);
  static_assert(std::is_same_v<typename bool_meta::common_rep, void>);

  EXPECT_EQ(bool_handler::diagnostic_id, 3u);
}

TEST(OperationsTest, CharUnderlyingRejectsArithmeticEvenWithTransparentType) {
  using value_t = primitive<char, policy::checked_value,
                            policy::transparent_type, policy::expected_error>;
  using char_handler = policy::type_handler<policy::transparent_type,
                                            operations::Addition, char, char>;
  using char_meta =
      operations::dispatcher_meta<operations::Addition, value_t, value_t,
                                  policy::runtime_error_kind>;

  static_assert(char_handler::enabled);
  static_assert(!char_handler::allowed);
  static_assert(std::is_same_v<typename char_meta::common_rep, char>);

  EXPECT_EQ(char_handler::diagnostic_id, 3u);
}

TEST(OperationsTest, SignedAndUnsignedCharRejectArithmeticAtCompileTime) {
  using signed_handler =
      policy::type_handler<policy::transparent_type, operations::Addition,
                           signed char, signed char>;
  using unsigned_handler =
      policy::type_handler<policy::transparent_type, operations::Addition,
                           unsigned char, unsigned char>;

  static_assert(signed_handler::enabled);
  static_assert(!signed_handler::allowed);
  static_assert(unsigned_handler::enabled);
  static_assert(!unsigned_handler::allowed);

  EXPECT_EQ(signed_handler::diagnostic_id, 3u);
  EXPECT_EQ(unsigned_handler::diagnostic_id, 3u);
}

TEST(OperationsTest, BoolUnderlyingAllowsComparisonOperations) {
  using value_t = primitive<bool, policy::checked_value, policy::strict_type,
                            policy::expected_error>;

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
  using value_t = primitive<char, policy::checked_value,
                            policy::transparent_type, policy::expected_error>;

  auto const lhs = value_t{'a'};
  auto const rhs = value_t{'a'};

  auto const eq_result = operations::equal(lhs, rhs);

  ASSERT_TRUE(eq_result.has_value());
  EXPECT_EQ(eq_result->value(), static_cast<char>(1));
}

TEST(OperationsTest, PrimitiveAliasWorksWithFrameworkOperators) {
  using namespace mcpplibs::primitives::types;
  using namespace mcpplibs::primitives::operators;
  using value_t = I32<policy::checked_value, policy::expected_error>;

  auto const lhs = value_t{20};
  auto const rhs = value_t{22};

  auto const result = lhs + rhs;

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 42);
}

TEST(OperationsTest, PrimitiveAliasMixesWithBuiltinArithmeticExplicitly) {
  using namespace mcpplibs::primitives::types;
  using namespace mcpplibs::primitives::operators;
  using value_t = I32<policy::checked_value, policy::expected_error>;

  static_assert(!std::is_convertible_v<value_t, int>);

  auto const lhs = value_t{40};
  auto const mixed = static_cast<int>(lhs) + 2;
  EXPECT_EQ(mixed, 42);

  auto const wrapped = value_t{mixed};
  auto const result = wrapped + value_t{1};

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 43);
}

TEST(OperationsTest, OperatorEqualDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t = primitive<int, policy::checked_value, policy::strict_type,
                            policy::expected_error>;

  auto const lhs = value_t{7};
  auto const rhs = value_t{7};

  auto const result = (lhs == rhs);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 1);
}

TEST(OperationsTest, OperatorPlusDelegatesToDispatcher) {
  using namespace mcpplibs::primitives::operators;
  using value_t = primitive<int, policy::checked_value>;

  auto const lhs = value_t{7};
  auto const rhs = value_t{8};

  auto const result = lhs + rhs;

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->value(), 15);
}

TEST(OperationsTest, ThrowErrorPolicyThrowsException) {
  using value_t = primitive<int, policy::checked_value, policy::throw_error>;

  auto const lhs = value_t{100};
  auto const rhs = value_t{0};

  EXPECT_THROW((void)operations::div(lhs, rhs), std::runtime_error);
}

TEST(OperationsTest, ThrowErrorPolicyExceptionHasReasonMessage) {
  using value_t = primitive<int, policy::checked_value, policy::throw_error>;

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
