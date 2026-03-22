module;
#include <atomic>
#include <cmath>
#include <cstdint>
#include <concepts>
#include <exception>
#include <expected>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

export module mcpplibs.primitives.policy.impl;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.underlying.traits;

export namespace mcpplibs::primitives::policy {

namespace value {
struct checked {};
struct unchecked {};
struct saturating {};
} // namespace value

namespace type {
struct strict {};
struct compatible {};
struct transparent {};
} // namespace type

namespace error {
struct throwing {};
struct expected {};
struct terminate {};
} // namespace error

namespace concurrency {
struct none {};
struct fenced {};
struct fenced_relaxed {};
struct fenced_acq_rel {};
struct fenced_seq_cst {};
} // namespace concurrency

template <> struct traits<value::checked> {
  using policy_type = value::checked;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<value::unchecked> {
  using policy_type = value::unchecked;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<value::saturating> {
  using policy_type = value::saturating;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<type::strict> {
  using policy_type = type::strict;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<type::compatible> {
  using policy_type = type::compatible;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<type::transparent> {
  using policy_type = type::transparent;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<error::throwing> {
  using policy_type = error::throwing;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<error::expected> {
  using policy_type = error::expected;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<error::terminate> {
  using policy_type = error::terminate;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<concurrency::none> {
  using policy_type = concurrency::none;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

template <> struct traits<concurrency::fenced> {
  using policy_type = concurrency::fenced;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

template <> struct traits<concurrency::fenced_relaxed> {
  using policy_type = concurrency::fenced_relaxed;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

template <> struct traits<concurrency::fenced_acq_rel> {
  using policy_type = concurrency::fenced_acq_rel;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

template <> struct traits<concurrency::fenced_seq_cst> {
  using policy_type = concurrency::fenced_seq_cst;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

namespace defaults {
using value = value::checked;
using type = type::strict;
using error = error::throwing;
using concurrency = concurrency::none;
} // namespace defaults

namespace details {

template <typename T, bool = std::is_trivially_copyable_v<T>>
struct atomic_ref_alignment_compatible : std::false_type {};

template <typename T>
struct atomic_ref_alignment_compatible<T, true>
    : std::bool_constant<(alignof(T) >=
                          std::atomic_ref<T>::required_alignment)> {};

template <typename T>
inline constexpr bool atomic_ref_compatible_v =
    atomic_ref_alignment_compatible<T>::value;

template <typename T> constexpr void assert_atomic_ref_compatible() {
  static_assert(std::is_trivially_copyable_v<T>,
                "concurrency::handler atomic access requires trivially "
                "copyable CommonRep");
  static_assert(
      atomic_ref_alignment_compatible<T>::value,
      "concurrency::handler atomic access requires alignof(CommonRep) to "
      "satisfy std::atomic_ref<CommonRep>::required_alignment");
}

template <typename T>
concept has_underlying_rep_not_equal =
    underlying_type<T> &&
    requires(T const &lhs, T const &rhs) {
      {
        underlying::traits<std::remove_cv_t<T>>::to_rep(lhs) !=
        underlying::traits<std::remove_cv_t<T>>::to_rep(rhs)
      } -> std::convertible_to<bool>;
    };

template <typename T>
inline constexpr bool none_compare_exchange_available_v =
    has_underlying_rep_not_equal<T>;

template <typename OpTag>
inline constexpr bool is_arithmetic_operation_v =
    operations::op_has_capability_v<OpTag, operations::capability::arithmetic>;

template <typename T>
inline constexpr bool is_boolean_or_character_v = std_bool<T> || std_char<T>;

template <typename OpTag, typename LhsRep, typename RhsRep>
inline constexpr bool rejects_arithmetic_for_boolean_or_character_v =
    is_arithmetic_operation_v<OpTag> &&
    (is_boolean_or_character_v<LhsRep> || is_boolean_or_character_v<RhsRep>);

template <typename LhsRep, typename RhsRep>
inline constexpr bool has_non_void_common_rep_v =
    has_common_rep<LhsRep, RhsRep> &&
    !std::same_as<common_rep_t<LhsRep, RhsRep>, void>;

template <typename LhsRep, typename RhsRep>
inline constexpr bool has_same_underlying_category_v =
    underlying::traits<std::remove_cv_t<LhsRep>>::enabled &&
    underlying::traits<std::remove_cv_t<RhsRep>>::enabled &&
    (underlying::traits<std::remove_cv_t<LhsRep>>::kind ==
     underlying::traits<std::remove_cv_t<RhsRep>>::kind);

template <typename T>
auto atomic_ref_load(T const &value, std::memory_order order) noexcept -> T {
  assert_atomic_ref_compatible<T>();
  // libc++ rejects std::atomic_ref<const T>; load through a non-mutating view.
  auto &mutable_value = const_cast<T &>(value);
  std::atomic_ref<T> ref(mutable_value);
  return ref.load(order);
}

} // namespace details

// Default protocol specializations.
template <operations::operation OpTag, typename LhsRep, typename RhsRep>
struct type::handler<type::strict, OpTag, LhsRep, RhsRep> {
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
struct type::handler<type::compatible, OpTag, LhsRep, RhsRep> {
  static constexpr bool enabled = true;
  static constexpr bool allowed =
      std::is_arithmetic_v<LhsRep> && std::is_arithmetic_v<RhsRep> &&
      details::has_same_underlying_category_v<LhsRep, RhsRep> &&
      details::has_non_void_common_rep_v<LhsRep, RhsRep> &&
      !details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                              RhsRep>;
  static constexpr unsigned diagnostic_id =
      details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                             RhsRep>
          ? 3u
          : (allowed ? 0u : 2u);
  using common_rep =
      std::conditional_t<allowed, common_rep_t<LhsRep, RhsRep>, void>;
};

template <operations::operation OpTag, typename LhsRep, typename RhsRep>
struct type::handler<type::transparent, OpTag, LhsRep, RhsRep> {
  static constexpr bool enabled = true;
  static constexpr bool allowed =
      details::has_non_void_common_rep_v<LhsRep, RhsRep> &&
      !details::rejects_arithmetic_for_boolean_or_character_v<OpTag, LhsRep,
                                                              RhsRep>;
  static constexpr unsigned diagnostic_id = allowed ? 0u : 3u;
  using common_rep =
      std::conditional_t<allowed, common_rep_t<LhsRep, RhsRep>, void>;
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency::handler<concurrency::none, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool requires_external_sync = false;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    return injection_type{};
  }
};

template <typename CommonRep, typename ErrorPayload>
struct concurrency::handler<concurrency::none, void, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto load(CommonRep const &value) noexcept -> CommonRep {
    return value;
  }

  static constexpr auto store(CommonRep &value, CommonRep desired) noexcept
      -> void {
    value = desired;
  }

  static constexpr auto compare_exchange(CommonRep &value, CommonRep &expected,
                                         CommonRep desired) noexcept -> bool
      requires(details::none_compare_exchange_available_v<CommonRep>) {
    using traits_type = underlying::traits<std::remove_cv_t<CommonRep>>;
    if (traits_type::to_rep(value) != traits_type::to_rep(expected)) {
      expected = value;
      return false;
    }

    value = desired;
    return true;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency::handler<concurrency::fenced, OpTag, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool requires_external_sync = true;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    injection_type out{};
    out.fence_before = true;
    out.fence_after = true;
    out.order_before = std::memory_order_seq_cst;
    out.order_after = std::memory_order_seq_cst;
    return out;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency::handler<concurrency::fenced_relaxed, OpTag, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool requires_external_sync = true;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    injection_type out{};
    out.fence_before = true;
    out.fence_after = true;
    out.order_before = std::memory_order_relaxed;
    out.order_after = std::memory_order_relaxed;
    return out;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency::handler<concurrency::fenced_acq_rel, OpTag, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool requires_external_sync = true;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    injection_type out{};
    out.fence_before = true;
    out.fence_after = true;
    out.order_before = std::memory_order_acquire;
    out.order_after = std::memory_order_release;
    return out;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct concurrency::handler<concurrency::fenced_seq_cst, OpTag, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool requires_external_sync = true;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto inject() noexcept -> injection_type {
    injection_type out{};
    out.fence_before = true;
    out.fence_after = true;
    out.order_before = std::memory_order_seq_cst;
    out.order_after = std::memory_order_seq_cst;
    return out;
  }
};

template <typename CommonRep, typename ErrorPayload>
struct concurrency::handler<concurrency::fenced, void, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = details::atomic_ref_compatible_v<CommonRep>;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static auto load(CommonRep const &value) noexcept -> CommonRep {
    return details::atomic_ref_load(value, std::memory_order_seq_cst);
  }

  static auto store(CommonRep &value, CommonRep desired) noexcept -> void {
    details::assert_atomic_ref_compatible<CommonRep>();
    std::atomic_ref<CommonRep> ref(value);
    ref.store(desired, std::memory_order_seq_cst);
  }

  static auto compare_exchange(CommonRep &value, CommonRep &expected,
                               CommonRep desired) noexcept -> bool {
    details::assert_atomic_ref_compatible<CommonRep>();
    std::atomic_ref<CommonRep> ref(value);
    return ref.compare_exchange_strong(expected, desired,
                                       std::memory_order_seq_cst,
                                       std::memory_order_seq_cst);
  }
};

template <typename CommonRep, typename ErrorPayload>
struct concurrency::handler<concurrency::fenced_relaxed, void, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = details::atomic_ref_compatible_v<CommonRep>;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static auto load(CommonRep const &value) noexcept -> CommonRep {
    return details::atomic_ref_load(value, std::memory_order_relaxed);
  }

  static auto store(CommonRep &value, CommonRep desired) noexcept -> void {
    details::assert_atomic_ref_compatible<CommonRep>();
    std::atomic_ref<CommonRep> ref(value);
    ref.store(desired, std::memory_order_relaxed);
  }

  static auto compare_exchange(CommonRep &value, CommonRep &expected,
                               CommonRep desired) noexcept -> bool {
    details::assert_atomic_ref_compatible<CommonRep>();
    std::atomic_ref<CommonRep> ref(value);
    return ref.compare_exchange_strong(expected, desired,
                                       std::memory_order_relaxed,
                                       std::memory_order_relaxed);
  }
};

template <typename CommonRep, typename ErrorPayload>
struct concurrency::handler<concurrency::fenced_acq_rel, void, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = details::atomic_ref_compatible_v<CommonRep>;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static auto load(CommonRep const &value) noexcept -> CommonRep {
    return details::atomic_ref_load(value, std::memory_order_acquire);
  }

  static auto store(CommonRep &value, CommonRep desired) noexcept -> void {
    details::assert_atomic_ref_compatible<CommonRep>();
    std::atomic_ref<CommonRep> ref(value);
    ref.store(desired, std::memory_order_release);
  }

  static auto compare_exchange(CommonRep &value, CommonRep &expected,
                               CommonRep desired) noexcept -> bool {
    details::assert_atomic_ref_compatible<CommonRep>();
    std::atomic_ref<CommonRep> ref(value);
    return ref.compare_exchange_strong(expected, desired,
                                       std::memory_order_acq_rel,
                                       std::memory_order_acquire);
  }
};

template <typename CommonRep, typename ErrorPayload>
struct concurrency::handler<concurrency::fenced_seq_cst, void, CommonRep,
                            ErrorPayload> {
  static constexpr bool enabled = details::atomic_ref_compatible_v<CommonRep>;
  using injection_type = concurrency::injection;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static auto load(CommonRep const &value) noexcept -> CommonRep {
    return concurrency::handler<concurrency::fenced, void, CommonRep,
                                ErrorPayload>::load(value);
  }

  static auto store(CommonRep &value, CommonRep desired) noexcept -> void {
    concurrency::handler<concurrency::fenced, void, CommonRep,
                         ErrorPayload>::store(value, desired);
  }

  static auto compare_exchange(CommonRep &value, CommonRep &expected,
                               CommonRep desired) noexcept -> bool {
    return concurrency::handler<concurrency::fenced, void, CommonRep,
                                ErrorPayload>::compare_exchange(value, expected,
                                                                desired);
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct value::handler<value::checked, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool may_adjust_value = false;
  using decision_type = value::decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency::injection const &) noexcept
      -> decision_type {
    return decision;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct value::handler<value::unchecked, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool may_adjust_value = false;
  using decision_type = value::decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency::injection const &) noexcept
      -> decision_type {
    return decision;
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct value::handler<value::saturating, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool may_adjust_value = true;
  using decision_type = value::decision<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto finalize(decision_type decision,
                                 concurrency::injection const &) noexcept
      -> decision_type {
    return decision;
  }
};

namespace details {
template <typename ErrorPayload>
constexpr auto to_error_payload(error::kind kind) -> ErrorPayload {
  if constexpr (std::same_as<ErrorPayload, error::kind>) {
    return kind;
  } else {
    static_cast<void>(kind);
    return ErrorPayload{};
  }
}

template <typename DestRep, typename SrcRep>
constexpr auto narrow_integral_error(SrcRep value)
    -> std::optional<error::kind> {
  using dest_type = std::remove_cv_t<DestRep>;
  using src_type = std::remove_cv_t<SrcRep>;

  if constexpr (std::is_signed_v<src_type>) {
    auto const signed_value = static_cast<std::intmax_t>(value);
    if constexpr (std::is_signed_v<dest_type>) {
      if (signed_value <
          static_cast<std::intmax_t>(std::numeric_limits<dest_type>::min())) {
        return error::kind::underflow;
      }
      if (signed_value >
          static_cast<std::intmax_t>(std::numeric_limits<dest_type>::max())) {
        return error::kind::overflow;
      }
      return std::nullopt;
    } else {
      if (signed_value < 0) {
        return error::kind::underflow;
      }

      if (static_cast<std::uintmax_t>(signed_value) >
          static_cast<std::uintmax_t>(std::numeric_limits<dest_type>::max())) {
        return error::kind::overflow;
      }
      return std::nullopt;
    }
  } else {
    auto const unsigned_value = static_cast<std::uintmax_t>(value);
    if (unsigned_value >
        static_cast<std::uintmax_t>(std::numeric_limits<dest_type>::max())) {
      return error::kind::overflow;
    }
    return std::nullopt;
  }
}

template <typename DestRep, typename SrcRep>
constexpr auto narrow_numeric_error(SrcRep value)
    -> std::optional<error::kind> {
  using dest_type = std::remove_cv_t<DestRep>;
  using src_type = std::remove_cv_t<SrcRep>;

  if constexpr (std::integral<dest_type> && std::integral<src_type>) {
    return narrow_integral_error<dest_type>(value);
  } else if constexpr (std::integral<dest_type> &&
                       std::floating_point<src_type>) {
    if (std::isnan(value)) {
      return error::kind::domain_error;
    }
    if (std::isinf(value)) {
      return value < static_cast<src_type>(0) ? error::kind::underflow
                                              : error::kind::overflow;
    }

    auto const normalized = static_cast<long double>(value);
    auto const min_value =
        static_cast<long double>(std::numeric_limits<dest_type>::lowest());
    auto const max_value =
        static_cast<long double>(std::numeric_limits<dest_type>::max());

    if (normalized < min_value) {
      return error::kind::underflow;
    }
    if (normalized > max_value) {
      return error::kind::overflow;
    }
    return std::nullopt;
  } else {
    static_cast<void>(value);
    return std::nullopt;
  }
}

template <typename DestRep, typename SrcRep>
constexpr auto safe_numeric_cast(SrcRep value) noexcept -> DestRep {
  using dest_type = std::remove_cv_t<DestRep>;
  using src_type = std::remove_cv_t<SrcRep>;

  if constexpr (std::integral<dest_type> && std::floating_point<src_type>) {
    if (auto const kind = narrow_numeric_error<dest_type>(value);
        kind.has_value()) {
      if (*kind == error::kind::overflow) {
        return std::numeric_limits<dest_type>::max();
      }
      if (*kind == error::kind::underflow) {
        return std::numeric_limits<dest_type>::lowest();
      }
      return dest_type{};
    }
  }

  return static_cast<dest_type>(value);
}
} // namespace details

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct error::handler<error::throwing, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool converts_to_expected = true;
  using request_type = error::request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static auto resolve(request_type const &request) -> result_type {
    throw std::runtime_error(std::string{request.reason});
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct error::handler<error::expected, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool converts_to_expected = true;
  using request_type = error::request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  static constexpr auto resolve(request_type const &request) -> result_type {
    return std::unexpected(
        details::to_error_payload<ErrorPayload>(request.code));
  }
};

template <operations::operation OpTag, typename CommonRep,
          typename ErrorPayload>
struct error::handler<error::terminate, OpTag, CommonRep, ErrorPayload> {
  static constexpr bool enabled = true;
  static constexpr bool converts_to_expected = false;
  using request_type = error::request<CommonRep>;
  using result_type = std::expected<CommonRep, ErrorPayload>;

  [[noreturn]] static auto resolve(request_type const &) -> result_type {
    std::terminate();
  }
};

} // namespace mcpplibs::primitives::policy
