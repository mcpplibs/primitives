module;

#include <atomic>
#include <concepts>
#include <expected>
#include <type_traits>
#include <utility>

export module mcpplibs.primitives.operations.invoker;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.binding;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.policy.impl;

export namespace mcpplibs::primitives::operations::invoker {

namespace details {

constexpr auto apply_runtime_fence(bool enabled) noexcept -> void {
  if (!enabled) {
    return;
  }

  if (!std::is_constant_evaluated()) {
    std::atomic_thread_fence(std::memory_order_seq_cst);
  }
}

} // namespace details

template <typename ConcurrencyHandler, typename CommonRep>
constexpr auto inject_concurrency() -> policy::concurrency::injection {
  static_cast<void>(sizeof(CommonRep));
  static_assert(ConcurrencyHandler::enabled,
                "Selected concurrency handler is not enabled");
  static_assert(std::same_as<typename ConcurrencyHandler::injection_type,
                             policy::concurrency::injection>,
                "concurrency handler must use policy::concurrency::injection");

  return ConcurrencyHandler::inject();
}

template <operation OpTag, typename ValuePolicy, typename CommonRep,
          typename ValueHandler, typename ErrorPayload>
constexpr auto run_value(CommonRep lhs, CommonRep rhs,
                         policy::concurrency::injection const &injection)
    -> policy::value::decision<CommonRep> {
  static_cast<void>(sizeof(ErrorPayload));
  static_assert(ValueHandler::enabled, "Selected value handler is not enabled");
  static_assert(
      std::same_as<typename ValueHandler::decision_type,
                   policy::value::decision<CommonRep>>,
      "value handler decision_type must match policy::value::decision");

  static_assert(
      binding::op_binding_available<OpTag, ValuePolicy, CommonRep>,
      "Missing operation binding specialization for this OpTag/common type");

  details::apply_runtime_fence(injection.fence_before);

  auto decision =
      binding::op_binding<OpTag, ValuePolicy, CommonRep>::apply(lhs, rhs);
  auto finalized = ValueHandler::finalize(std::move(decision), injection);

  details::apply_runtime_fence(injection.fence_after);
  return finalized;
}

template <typename ErrorPolicy, operation OpTag, typename CommonRep,
          typename ErrorPayload>
constexpr auto resolve_error(policy::error::request<CommonRep> const &request)
    -> std::expected<CommonRep, ErrorPayload> {
  using handler_t =
      policy::error::handler<ErrorPolicy, OpTag, CommonRep, ErrorPayload>;
  return handler_t::resolve(request);
}

} // namespace mcpplibs::primitives::operations::invoker
