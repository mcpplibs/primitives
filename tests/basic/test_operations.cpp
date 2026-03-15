#include <limits>
#include <type_traits>
#include <gtest/gtest.h>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

namespace {

struct debug_value_policy {};
struct debug_type_policy {};
struct debug_error_policy {};
struct debug_concurrency_policy {};

} // namespace

template <>
struct policy::traits<debug_value_policy> {
  using policy_type = debug_value_policy;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <>
struct policy::traits<debug_type_policy> {
  using policy_type = debug_type_policy;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <>
struct policy::traits<debug_error_policy> {
  using policy_type = debug_error_policy;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <>
struct policy::traits<debug_concurrency_policy> {
  using policy_type = debug_concurrency_policy;
  static constexpr bool enabled = true;
  static constexpr auto kind =
      category::concurrency;
};

template <typename OpTag>
struct operations::value_policy_behavior<
    debug_value_policy, OpTag> {
  template <typename T>
  static constexpr T add(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs + rhs + 100);
  }

  template <typename T>
  static constexpr T sub(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs - rhs - 100);
  }

  template <typename T>
  static constexpr T mul(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs * rhs * 2);
  }
};

template <typename OpTag>
struct operations::type_policy_behavior<
    debug_type_policy, OpTag> {
  template <typename L, typename R>
  using result_type = long long;

  template <typename Out, typename In>
  static constexpr Out cast_lhs(In const &value) noexcept {
    return static_cast<Out>(value + 1);
  }

  template <typename Out, typename In>
  static constexpr Out cast_rhs(In const &value) noexcept {
    return static_cast<Out>(value + 2);
  }
};

template <typename OpTag>
struct operations::error_policy_behavior<
    debug_error_policy, OpTag> {
  template <typename Result, typename Fn>
  static constexpr Result evaluate(Fn &&fn) noexcept(noexcept(fn())) {
    return static_cast<Result>(fn() + static_cast<Result>(1000));
  }
};

template <typename OpTag>
struct operations::concurrency_policy_behavior<
    debug_concurrency_policy, OpTag> {
  template <typename Fn>
  static constexpr auto execute(Fn &&fn) noexcept(noexcept(fn()))
      -> decltype(fn()) {
    return fn() + 7;
  }
};



template <>
struct operations::value_policy_behavior<
    debug_value_policy, operations::add_tag> {
  template <typename T>
  static constexpr T add(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs + rhs + 200);
  }

  template <typename T>
  static constexpr T sub(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs - rhs - 100);
  }

  template <typename T>
  static constexpr T mul(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs * rhs * 2);
  }
};

TEST(OperationsTest, UnderlyingOperationsWork) {
  static_assert(operations::binary_operation<operations::add_tag, int, int>);
  static_assert(operations::binary_operation<operations::sub_tag, int, int>);
  static_assert(operations::binary_operation<operations::mul_tag, int, int>);

  EXPECT_EQ(operations::add(2, 3), 5);
  EXPECT_EQ(operations::sub(9, 4), 5);
  EXPECT_EQ(operations::mul(3, 4), 12);
}

TEST(OperationsTest, PrimitiveOperationsWork) {
  using namespace mcpplibs::primitives::types;

  using lhs_t = I8<>;
  using rhs_t = I8<>;

  lhs_t lhs{10};
  rhs_t rhs{5};

  auto added = operations::add(lhs, rhs);
  auto subed = operations::sub(lhs, rhs);
  auto muled = operations::mul(lhs, rhs);

  EXPECT_EQ(added.value(), 15);
  EXPECT_EQ(subed.value(), 5);
  EXPECT_EQ(muled.value(), 50);
}

TEST(OperationsTest, PolicyBehaviorTraitsCoverAllPolicyCategories) {
  EXPECT_EQ((operations::add.operator()<
                 debug_value_policy, debug_type_policy, debug_error_policy,
                 debug_concurrency_policy>(2, 3)),
            1215);
  EXPECT_EQ((operations::sub.operator()<
                 debug_value_policy, debug_type_policy, debug_error_policy,
                 debug_concurrency_policy>(9, 4)),
            911);
  EXPECT_EQ((operations::mul.operator()<
                 debug_value_policy, debug_type_policy, debug_error_policy,
                 debug_concurrency_policy>(3, 4)),
            1055);
}


TEST(OperationsTest, BuiltinPolicyTagsAreSpecialized) {
  static_assert(std::is_same_v<
      operations::result_t<operations::add_tag, int, int, policy::strict_type>,
      int>);
  static_assert(std::is_same_v<
      operations::result_t<operations::add_tag, int, double,
                           policy::transparent_type>,
      double>);

  constexpr int kMax = std::numeric_limits<int>::max();
  constexpr int kMin = std::numeric_limits<int>::min();

  EXPECT_EQ((operations::add.operator()<policy::saturating_value>(kMax, 1)),
            kMax);
  EXPECT_EQ((operations::sub.operator()<policy::saturating_value>(kMin, 1)),
            kMin);
  EXPECT_EQ((operations::mul.operator()<policy::saturating_value>(kMax, 2)),
            kMax);
}
