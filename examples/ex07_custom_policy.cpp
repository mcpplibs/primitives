/*
 * Example: ex07_custom_policy
 *
 * Purpose:
 * Show end-to-end customization of all policy dimensions (type, value,
 * error, concurrency) and custom operation binding integration.
 *
 * Expected results:
 * - Custom policy traits are recognized and selected by dispatcher.
 * - Custom value finalize step adjusts addition result by +1.
 * - Input 20 and 21 produces final value 42.
 * - Program prints a success message and exits with code 0.
 */

#include <atomic>
#include <expected>
#include <iostream>
#include <type_traits>


import mcpplibs.primitives;
import mcpplibs.primitives.operations.invoker;

using namespace mcpplibs::primitives;

namespace demo {
// Point 7 / Step 1: Define one custom tag for each policy dimension.
struct custom_value {};
struct custom_type {};
struct custom_error {};
struct custom_concurrency {};
} // namespace demo

// Point 7 / Step 2: Register tags into policy::traits.
template <> struct mcpplibs::primitives::policy::traits<demo::custom_value> {
  using policy_type = demo::custom_value;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct mcpplibs::primitives::policy::traits<demo::custom_type> {
  using policy_type = demo::custom_type;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct mcpplibs::primitives::policy::traits<demo::custom_error> {
  using policy_type = demo::custom_error;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <>
struct mcpplibs::primitives::policy::traits<demo::custom_concurrency> {
  using policy_type = demo::custom_concurrency;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

// Point 7 / Step 3A: Implement custom type handler.
// Always allow and negotiate through std::common_type_t.
template <operations::operation OpTag, typename LhsRep, typename RhsRep>
struct mcpplibs::primitives::policy::type::handler<demo::custom_type, OpTag,
                                                   LhsRep, RhsRep> {
  static constexpr bool enabled = true;
  static constexpr bool allowed = true;
  static constexpr unsigned diagnostic_id = 0;
  using common_rep = std::common_type_t<LhsRep, RhsRep>;
};

// Point 7 / Step 3B: Implement custom concurrency handler.
template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct mcpplibs::primitives::policy::concurrency::handler<
    demo::custom_concurrency, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  using injection_type = mcpplibs::primitives::policy::concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    injection_type out{};
    out.fence_before = true;
    out.fence_after = false;
    out.order_before = std::memory_order_acquire;
    out.order_after = std::memory_order_relaxed;
    return out;
  }
};

// Point 7 / Step 3C: Implement custom value handler.
// Complex point: finalize() post-processes decision and adjusts output.
template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct mcpplibs::primitives::policy::value::handler<demo::custom_value, OpTag,
                                                    CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool may_adjust_value = true;
  using decision_type =
      mcpplibs::primitives::policy::value::decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto
  finalize(decision_type decision,
           mcpplibs::primitives::policy::concurrency::injection const &)
      -> decision_type {
    if (decision.has_value) {
      decision.value = static_cast<CommonRep>(decision.value + 1);
    }
    return decision;
  }
};

// Point 7 / Step 3D: Provide binding for custom value policy + Addition.
// Without this specialization, runtime::run_value static_assert will fail.
template <typename CommonRep>
struct mcpplibs::primitives::operations::runtime::op_binding<
    mcpplibs::primitives::operations::Addition, demo::custom_value, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> mcpplibs::primitives::policy::value::decision<CommonRep> {
    mcpplibs::primitives::policy::value::decision<CommonRep> out{};
    out.has_value = true;
    out.value = static_cast<CommonRep>(lhs + rhs);
    return out;
  }
};

// Point 7 / Step 3E: Implement custom error handler.
template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct mcpplibs::primitives::policy::error::handler<demo::custom_error, OpTag,
                                                    CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  using request_type = mcpplibs::primitives::policy::error::request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto resolve(request_type const &) -> result_type {
    return std::unexpected(policy::error::kind::unspecified);
  }
};

int main() {
  // Point 7 / Step 4: Compose all custom tags and execute a call path.
  using custom_t = primitive<int, demo::custom_value, demo::custom_type,
                             demo::custom_error, demo::custom_concurrency>;

  auto const lhs = custom_t{20};
  auto const rhs = custom_t{21};
  auto const result = operations::add(lhs, rhs);

  if (!result.has_value() || result->value() != 42) {
    std::cerr << "custom policy pipeline failed\n";
    return 1;
  }

  std::cout << "custom policy demo passed\n";
  return 0;
}
