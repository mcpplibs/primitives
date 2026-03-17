module;

#include <expected>
#include <optional>
#include <string_view>

export module mcpplibs.primitives.policy.handler;

import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.operations.traits;

export namespace mcpplibs::primitives::policy {

namespace error {

// Unified runtime error kind carried across the policy chain.
enum class kind : unsigned char {
  none = 0,
  invalid_type_combination,
  overflow,
  underflow,
  divide_by_zero,
  domain_error,
  unspecified,
};

template <typename CommonRep> struct request {
  kind kind = kind::none;
  std::string_view reason{};
  std::optional<CommonRep> lhs_value{};
  std::optional<CommonRep> rhs_value{};
  std::optional<CommonRep> fallback_value{};
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
struct handler {
  static constexpr bool enabled = false;
  using request_type = request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto resolve(request_type const &) -> result_type {
    return std::unexpected(ErrorPayload{});
  }
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
concept handler_available = requires {
  requires error_policy<Policy>;
  requires operations::operation<OpTag>;
  requires handler<Policy, OpTag, CommonRep, ErrorPayload>::enabled;
};

} // namespace error

namespace concurrency {

struct injection {
  bool fence_before = false;
  bool fence_after = false;
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
struct handler {
  static constexpr bool enabled = false;
  using injection_type = injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type { return {}; }
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
concept handler_available = requires {
  requires concurrency_policy<Policy>;
  requires operations::operation<OpTag>;
  requires handler<Policy, OpTag, CommonRep, ErrorPayload>::enabled;
};

} // namespace concurrency

namespace type {

template <typename OpTag, typename LhsRep, typename RhsRep, typename CommonRep>
struct operation_context {
  using op_tag = OpTag;
  using lhs_rep = LhsRep;
  using rhs_rep = RhsRep;
  using common_rep = CommonRep;
};

// Compile-time type negotiation protocol.
template <typename Policy, typename OpTag, typename LhsRep, typename RhsRep>
struct handler {
  static constexpr bool enabled = false;
  static constexpr bool allowed = false;
  static constexpr unsigned diagnostic_id = 0;
  using common_rep = void;
};

template <typename Policy, typename OpTag, typename LhsRep, typename RhsRep>
concept handler_available = requires {
  requires type_policy<Policy>;
  requires operations::operation<OpTag>;
  requires handler<Policy, OpTag, LhsRep, RhsRep>::enabled;
};

} // namespace type

namespace value {

template <typename CommonRep> struct decision {
  bool has_value = false;
  CommonRep value{};
  error::request<CommonRep> error{};
};

// Runtime value-check protocol.
template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
struct handler {
  static constexpr bool enabled = false;
  static constexpr bool may_adjust_value = false;
  using decision_type = decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency::injection const &)
      -> decision_type {
    return decision;
  }
};

template <typename Policy, typename OpTag, typename CommonRep,
          typename ErrorPayload>
concept handler_available = requires {
  requires value_policy<Policy>;
  requires operations::operation<OpTag>;
  requires handler<Policy, OpTag, CommonRep, ErrorPayload>::enabled;
};

} // namespace value

} // namespace mcpplibs::primitives::policy
