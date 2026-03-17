module;

#include <concepts>
#include <exception>
#include <expected>
#include <stdexcept>
#include <type_traits>

export module mcpplibs.primitives.policy.impl;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.underlying.traits;

export namespace mcpplibs::primitives::policy {

struct checked_value {};
struct unchecked_value {};
struct saturating_value {};

struct strict_type {};
struct category_compatible_type {};
struct transparent_type {};

struct throw_error {};
struct expected_error {};
struct terminate_error {};

struct single_thread {};
struct atomic {};

template <> struct traits<checked_value> {
  using policy_type = checked_value;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<unchecked_value> {
  using policy_type = unchecked_value;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<saturating_value> {
  using policy_type = saturating_value;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<strict_type> {
  using policy_type = strict_type;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<category_compatible_type> {
  using policy_type = category_compatible_type;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<transparent_type> {
  using policy_type = transparent_type;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<throw_error> {
  using policy_type = throw_error;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<expected_error> {
  using policy_type = expected_error;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<terminate_error> {
  using policy_type = terminate_error;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<single_thread> {
  using policy_type = single_thread;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

template <> struct traits<atomic> {
  using policy_type = atomic;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

using default_value = checked_value;
using default_type = strict_type;
using default_error = throw_error;
using default_concurrency = single_thread;

namespace details {

template <typename OpTag>
inline constexpr bool is_arithmetic_operation_v =
    operations::op_has_capability_v<OpTag, operations::capability::arithmetic>;

template <typename T>
inline constexpr bool is_boolean_or_character_v = std_bool<T> || std_char<T>;

template <typename OpTag, typename LhsRep, typename RhsRep>
inline constexpr bool rejects_arithmetic_for_boolean_or_character_v =
    is_arithmetic_operation_v<OpTag> &&
    (is_boolean_or_character_v<LhsRep> || is_boolean_or_character_v<RhsRep>);

} // namespace details

// Default protocol specializations.
template <operations::operation OpTag, typename LhsRep, typename RhsRep>
struct type_handler<strict_type, OpTag, LhsRep, RhsRep> {
  static constexpr bool enabled = true;
  static constexpr bool allowed =
      std::same_as<LhsRep, RhsRep> &&
      !details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                              RhsRep>;
  static constexpr unsigned diagnostic_id =
      details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                             RhsRep>
          ? 3u
          : (allowed ? 0u : 1u);
  using common_rep = std::conditional_t<allowed, LhsRep, void>;
};

template <operations::operation OpTag, typename LhsRep, typename RhsRep>
struct type_handler<category_compatible_type, OpTag, LhsRep, RhsRep> {
  static constexpr bool enabled = true;
  static constexpr bool allowed =
      std::is_arithmetic_v<LhsRep> && std::is_arithmetic_v<RhsRep> &&
      !details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                              RhsRep>;
  static constexpr unsigned diagnostic_id =
      details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                             RhsRep>
          ? 3u
          : (allowed ? 0u : 2u);
  using common_rep =
      std::conditional_t<allowed, std::common_type_t<LhsRep, RhsRep>, void>;
};

template <operations::operation OpTag, typename LhsRep, typename RhsRep>
struct type_handler<transparent_type, OpTag, LhsRep, RhsRep> {
  static constexpr bool enabled = true;
  static constexpr bool allowed =
      !details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                              RhsRep>;
  static constexpr unsigned diagnostic_id = allowed ? 0u : 3u;
  using common_rep = std::common_type_t<LhsRep, RhsRep>;
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency_handler<single_thread, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool requires_external_sync = false;
  using injection_type = concurrency_injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    return injection_type{};
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency_handler<atomic, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool requires_external_sync = true;
  using injection_type = concurrency_injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    injection_type out{};
    out.fence_before = true;
    out.fence_after = true;
    return out;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct value_handler<checked_value, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool may_adjust_value = false;
  using decision_type = value_decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency_injection const &) noexcept
      -> decision_type {
    return decision;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct value_handler<unchecked_value, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool may_adjust_value = false;
  using decision_type = value_decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency_injection const &) noexcept
      -> decision_type {
    return decision;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct value_handler<saturating_value, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool may_adjust_value = true;
  using decision_type = value_decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency_injection const &) noexcept
      -> decision_type {
    return decision;
  }
};

namespace details {
template <typename ErrorPayload>
constexpr auto to_error_payload(runtime_error_kind kind) -> ErrorPayload {
  if constexpr (std::same_as<ErrorPayload, runtime_error_kind>) {
    return kind;
  } else {
    static_cast<void>(kind);
    return ErrorPayload{};
  }
}
} // namespace details

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct error_handler<throw_error, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool converts_to_expected = true;
  using request_type = error_request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static auto resolve(request_type const &request) -> result_type {
    throw std::runtime_error(request.reason.data());
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct error_handler<expected_error, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool converts_to_expected = true;
  using request_type = error_request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto resolve(request_type const &request) -> result_type {
    return std::unexpected(
        details::to_error_payload<ErrorPayload>(request.kind));
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct error_handler<terminate_error, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool converts_to_expected = false;
  using request_type = error_request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  [[noreturn]] static auto resolve(request_type const &) -> result_type {
    std::terminate();
  }
};

} // namespace mcpplibs::primitives::policy
