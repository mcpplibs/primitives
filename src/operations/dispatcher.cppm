module;

#include <expected>
#include <type_traits>

export module mcpplibs.primitives.operations.dispatcher;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.invoker;
import mcpplibs.primitives.primitive.impl;
import mcpplibs.primitives.primitive.traits;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.underlying;

export namespace mcpplibs::primitives::operations {

template <typename T>
concept primitive_instance = requires {
  typename primitives::traits::primitive_traits<
      std::remove_cvref_t<T>>::value_type;
};

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
struct dispatcher_meta {
  using lhs_primitive = std::remove_cvref_t<Lhs>;
  using rhs_primitive = std::remove_cvref_t<Rhs>;

  using lhs_traits = primitives::traits::primitive_traits<lhs_primitive>;
  using rhs_traits = primitives::traits::primitive_traits<rhs_primitive>;

  using lhs_value_type = lhs_traits::value_type;
  using rhs_value_type = rhs_traits::value_type;

  using lhs_rep = underlying::traits<lhs_value_type>::rep_type;
  using rhs_rep = underlying::traits<rhs_value_type>::rep_type;

  using type_policy = lhs_traits::type_policy;
  using value_policy = lhs_traits::value_policy;
  using error_policy = lhs_traits::error_policy;
  using concurrency_policy = lhs_traits::concurrency_policy;

  using common_rep =
      policy::type::handler<type_policy, OpTag, lhs_rep, rhs_rep>::common_rep;

  static constexpr bool type_ready =
      policy::type::handler_available<type_policy, OpTag, lhs_rep, rhs_rep>;
  static constexpr bool type_protocol_ready =
      policy::type::handler_protocol<type_policy, OpTag, lhs_rep, rhs_rep>;
  static constexpr bool concurrency_ready =
      policy::concurrency::handler_available<concurrency_policy, OpTag,
                                             common_rep, ErrorPayload>;
  static constexpr bool concurrency_protocol_ready =
      policy::concurrency::handler_protocol<concurrency_policy, OpTag,
                                            common_rep, ErrorPayload>;
  static constexpr bool value_ready =
      policy::value::handler_available<value_policy, OpTag, common_rep,
                                       ErrorPayload>;
  static constexpr bool value_protocol_ready =
      policy::value::handler_protocol<value_policy, OpTag, common_rep,
                                      ErrorPayload>;
  static constexpr bool error_ready =
      policy::error::handler_available<error_policy, OpTag, common_rep,
                                       ErrorPayload>;
  static constexpr bool error_protocol_ready =
      policy::error::handler_protocol<error_policy, OpTag, common_rep,
                                      ErrorPayload>;
};

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
using dispatch_result_t = std::expected<
    typename dispatcher_meta<OpTag, Lhs, Rhs, ErrorPayload>::common_rep,
    ErrorPayload>;

// Dispatcher pipeline: compile-time negotiation plus runtime chain
// (concurrency -> value -> error) through selected policy handlers.
template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto dispatch(Lhs const &lhs, Rhs const &rhs)
    -> dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload> {
  using meta = dispatcher_meta<OpTag, Lhs, Rhs, ErrorPayload>;
  using common_rep = meta::common_rep;
  using concurrency_handler_t =
      policy::concurrency::handler<typename meta::concurrency_policy, OpTag,
                                   common_rep, ErrorPayload>;
  using value_handler_t =
      policy::value::handler<typename meta::value_policy, OpTag, common_rep,
                             ErrorPayload>;

  static_assert(
      meta::type_ready,
      "Missing type_handler specialization for this operation/policy");
  static_assert(meta::type_protocol_ready,
                "type::handler does not satisfy the policy protocol contract");
  static_assert(policy::type::handler<typename meta::type_policy, OpTag,
                                      typename meta::lhs_rep,
                                      typename meta::rhs_rep>::allowed,
                "Type policy rejected this operation for the given operands");
  static_assert(op_capability_valid_v<OpTag>,
                "Operation must declare a non-none capability_mask");
  static_assert(
      meta::concurrency_ready,
      "Missing concurrency_handler specialization for this operation/policy");
  static_assert(
      meta::concurrency_protocol_ready,
      "concurrency::handler does not satisfy the policy protocol contract");
  static_assert(
      meta::value_ready,
      "Missing value_handler specialization for this operation/policy");
  static_assert(meta::value_protocol_ready,
                "value::handler does not satisfy the policy protocol contract");
  static_assert(
      meta::error_ready,
      "Missing error_handler specialization for this operation/policy");
  static_assert(meta::error_protocol_ready,
                "error::handler does not satisfy the policy protocol contract");

  // Runtime stage 1: concurrency context injection.
  auto const injection =
      invoker::inject_concurrency<concurrency_handler_t, common_rep>();

  // Runtime stage 2: value path.
  auto const lhs_rep_raw =
      underlying::traits<typename meta::lhs_value_type>::to_rep(lhs.value());
  auto const rhs_rep_raw =
      underlying::traits<typename meta::rhs_value_type>::to_rep(rhs.value());

  if (!underlying::traits<typename meta::lhs_value_type>::is_valid_rep(
          lhs_rep_raw) ||
      !underlying::traits<typename meta::rhs_value_type>::is_valid_rep(
          rhs_rep_raw)) {
    policy::error::request<common_rep> request{};
    request.kind = policy::error::kind::domain_error;
    request.reason = "invalid underlying representation";
    return invoker::resolve_error<typename meta::error_policy, OpTag,
                                  common_rep, ErrorPayload>(request);
  }

  auto const lhs_value_normalized =
      underlying::traits<typename meta::lhs_value_type>::from_rep(lhs_rep_raw);
  auto const rhs_value_normalized =
      underlying::traits<typename meta::rhs_value_type>::from_rep(rhs_rep_raw);

  auto const lhs_common = static_cast<common_rep>(
      underlying::traits<typename meta::lhs_value_type>::to_rep(
          lhs_value_normalized));
  auto const rhs_common = static_cast<common_rep>(
      underlying::traits<typename meta::rhs_value_type>::to_rep(
          rhs_value_normalized));

  auto const decision =
      invoker::run_value<OpTag, typename meta::value_policy, common_rep,
                         value_handler_t, ErrorPayload>(lhs_common, rhs_common,
                                                        injection);

  if (decision.has_value) {
    return decision.value;
  }

  // Runtime stage 3: error policy.
  return invoker::resolve_error<typename meta::error_policy, OpTag, common_rep,
                                ErrorPayload>(decision.error);
}

} // namespace mcpplibs::primitives::operations
