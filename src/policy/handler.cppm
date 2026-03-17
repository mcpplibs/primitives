module;

#include <expected>
#include <optional>
#include <string_view>

export module mcpplibs.primitives.policy.handler;

import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.operations.traits;

export namespace mcpplibs::primitives::policy {

// Unified runtime error kind carried across the policy chain.
enum class runtime_error_kind : unsigned char {
  none = 0,
  invalid_type_combination,
  overflow,
  underflow,
  divide_by_zero,
  domain_error,
  unspecified,
};

template <typename CommonRep> struct error_request {
  runtime_error_kind kind = runtime_error_kind::none;
  std::string_view reason{};
  std::optional<CommonRep> lhs_value{};
  std::optional<CommonRep> rhs_value{};
  std::optional<CommonRep> fallback_value{};
};

struct concurrency_injection {
  bool fence_before = false;
  bool fence_after = false;
};

template <typename OpTag, typename LhsRep, typename RhsRep, typename CommonRep>
struct operation_context {
  using op_tag = OpTag;
  using lhs_rep = LhsRep;
  using rhs_rep = RhsRep;
  using common_rep = CommonRep;
};

template <typename CommonRep> struct value_decision {
  bool has_value = false;
  CommonRep value{};
  error_request<CommonRep> error{};
};

// Compile-time type negotiation protocol.
template <typename Policy, typename OpTag, typename LhsRep, typename RhsRep>
struct type_handler {
  static constexpr bool enabled = false;
  static constexpr bool allowed = false;
  static constexpr unsigned diagnostic_id = 0;
  using common_rep = void;
};

template <typename Policy, typename OpTag, typename LhsRep, typename RhsRep>
concept type_handler_available = requires {
  requires type_policy<Policy>;
  requires operations::operation<OpTag>;
  requires type_handler<Policy, OpTag, LhsRep, RhsRep>::enabled;
};

// Runtime concurrency wrapper protocol.
template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency_handler {
  static constexpr bool enabled = false;
  using injection_type = concurrency_injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type { return {}; }
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
concept concurrency_handler_available = requires {
  requires concurrency_policy<Policy>;
  requires operations::operation<OpTag>;
  requires concurrency_handler<Policy, OpTag, CommonRep, ErrorPayload>::enabled;
};

// Runtime value-check protocol.
template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
struct value_handler {
  static constexpr bool enabled = false;
  static constexpr bool may_adjust_value = false;
  using decision_type = value_decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency_injection const &)
      -> decision_type {
    return decision;
  }
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
concept value_handler_available = requires {
  requires value_policy<Policy>;
  requires operations::operation<OpTag>;
  requires value_handler<Policy, OpTag, CommonRep, ErrorPayload>::enabled;
};

// Runtime error-resolution protocol.
template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
struct error_handler {
  static constexpr bool enabled = false;
  using request_type = error_request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto resolve(request_type const &) -> result_type {
    return std::unexpected(ErrorPayload{});
  }
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
concept error_handler_available = requires {
  requires error_policy<Policy>;
  requires operations::operation<OpTag>;
  requires error_handler<Policy, OpTag, CommonRep, ErrorPayload>::enabled;
};

} // namespace mcpplibs::primitives::policy
