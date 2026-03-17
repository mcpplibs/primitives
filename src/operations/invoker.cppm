module;

#include <atomic>
#include <concepts>
#include <expected>
#include <limits>
#include <optional>
#include <type_traits>
#include <utility>

export module mcpplibs.primitives.operations.invoker;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.policy.traits;

export namespace mcpplibs::primitives::operations::runtime {

namespace details {

template <typename CommonRep>
constexpr auto make_error(policy::error::kind kind, char const *reason,
                          std::optional<CommonRep> lhs = std::nullopt,
                          std::optional<CommonRep> rhs = std::nullopt)
    -> policy::value::decision<CommonRep> {
  policy::value::decision<CommonRep> out{};
  out.has_value = false;
  out.error.kind = kind;
  out.error.reason = reason;
  out.error.lhs_value = lhs;
  out.error.rhs_value = rhs;
  return out;
}

template <typename T>
constexpr auto checked_add(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs + rhs);
    return out;
  } else if constexpr (std::is_unsigned_v<T>) {
    auto const maxv = std::numeric_limits<T>::max();
    if (lhs > maxv - rhs) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked addition overflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs + rhs);
    return out;
  } else {
    auto const maxv = std::numeric_limits<T>::max();
    auto const minv = std::numeric_limits<T>::min();
    if ((rhs > 0) && (lhs > maxv - rhs)) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked addition overflow", lhs, rhs);
    }
    if ((rhs < 0) && (lhs < minv - rhs)) {
      return make_error<T>(policy::error::kind::underflow,
                           "checked addition underflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs + rhs);
    return out;
  }
}

template <typename T>
constexpr auto checked_sub(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs - rhs);
    return out;
  } else if constexpr (std::is_unsigned_v<T>) {
    if (lhs < rhs) {
      return make_error<T>(policy::error::kind::underflow,
                           "checked subtraction underflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs - rhs);
    return out;
  } else {
    auto const maxv = std::numeric_limits<T>::max();
    auto const minv = std::numeric_limits<T>::min();
    if ((rhs < 0) && (lhs > maxv + rhs)) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked subtraction overflow", lhs, rhs);
    }
    if ((rhs > 0) && (lhs < minv + rhs)) {
      return make_error<T>(policy::error::kind::underflow,
                           "checked subtraction underflow", lhs, rhs);
    }
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs - rhs);
    return out;
  }
}

template <typename T>
constexpr auto checked_mul(T lhs, T rhs) -> policy::value::decision<T> {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs * rhs);
    return out;
  } else {
    if (lhs == T{} || rhs == T{}) {
      policy::value::decision<T> out{};
      out.has_value = true;
      out.value = T{};
      return out;
    }

    if constexpr (std::is_unsigned_v<T>) {
      auto const maxv = std::numeric_limits<T>::max();
      if (lhs > maxv / rhs) {
        return make_error<T>(policy::error::kind::overflow,
                             "checked multiplication overflow", lhs, rhs);
      }
      policy::value::decision<T> out{};
      out.has_value = true;
      out.value = static_cast<T>(lhs * rhs);
      return out;
    } else {
      auto const maxv = std::numeric_limits<T>::max();
      auto const minv = std::numeric_limits<T>::min();

      if (lhs > 0) {
        if (rhs > 0) {
          if (lhs > maxv / rhs) {
            return make_error<T>(policy::error::kind::overflow,
                                 "checked multiplication overflow", lhs, rhs);
          }
        } else {
          if (rhs < minv / lhs) {
            return make_error<T>(policy::error::kind::underflow,
                                 "checked multiplication underflow", lhs, rhs);
          }
        }
      } else {
        if (rhs > 0) {
          if (lhs < minv / rhs) {
            return make_error<T>(policy::error::kind::underflow,
                                 "checked multiplication underflow", lhs, rhs);
          }
        } else {
          if (lhs != 0 && rhs < maxv / lhs) {
            return make_error<T>(policy::error::kind::overflow,
                                 "checked multiplication overflow", lhs, rhs);
          }
        }
      }

      policy::value::decision<T> out{};
      out.has_value = true;
      out.value = static_cast<T>(lhs * rhs);
      return out;
    }
  }
}

template <typename T>
constexpr auto checked_div(T lhs, T rhs) -> policy::value::decision<T> {
  if (rhs == T{}) {
    return make_error<T>(policy::error::kind::divide_by_zero,
                         "checked division by zero", lhs, rhs);
  }

  if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
    auto const minv = std::numeric_limits<T>::min();
    if (lhs == minv && rhs == static_cast<T>(-1)) {
      return make_error<T>(policy::error::kind::overflow,
                           "checked division overflow", lhs, rhs);
    }
  }

  if constexpr (requires { lhs / rhs; }) {
    policy::value::decision<T> out{};
    out.has_value = true;
    out.value = static_cast<T>(lhs / rhs);
    return out;
  }

  return make_error<T>(
      policy::error::kind::unspecified,
      "checked division not supported for negotiated common type", lhs, rhs);
}

template <typename T>
constexpr auto compare_equal(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (requires { lhs == rhs; }) {
    out.has_value = true;
    out.value = static_cast<T>(lhs == rhs);
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "comparison equality not supported for negotiated "
                       "common type");
}

template <typename T>
constexpr auto compare_not_equal(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  if constexpr (requires { lhs != rhs; }) {
    out.has_value = true;
    out.value = static_cast<T>(lhs != rhs);
    return out;
  }

  return make_error<T>(policy::error::kind::unspecified,
                       "comparison inequality not supported for negotiated "
                       "common type");
}

template <typename T>
constexpr auto unchecked_add(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs + rhs; }) {
    out.value = static_cast<T>(lhs + rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_sub(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs - rhs; }) {
    out.value = static_cast<T>(lhs - rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_mul(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  if constexpr (requires { lhs * rhs; }) {
    out.value = static_cast<T>(lhs * rhs);
    return out;
  }

  out.value = T{};
  return out;
}

template <typename T>
constexpr auto unchecked_div(T lhs, T rhs) -> policy::value::decision<T> {
  policy::value::decision<T> out{};
  out.has_value = true;

  // Intentionally no guards: unchecked policy delegates error/UB behavior
  // to the underlying language/runtime semantics.
  out.value = static_cast<T>(lhs / rhs);
  return out;
}

template <typename T> constexpr auto saturating_add(T lhs, T rhs) -> T {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    return static_cast<T>(lhs + rhs);
  } else if constexpr (std::is_unsigned_v<T>) {
    auto const maxv = std::numeric_limits<T>::max();
    return (lhs > maxv - rhs) ? maxv : static_cast<T>(lhs + rhs);
  } else {
    auto const maxv = std::numeric_limits<T>::max();
    auto const minv = std::numeric_limits<T>::min();
    if ((rhs > 0) && (lhs > maxv - rhs)) {
      return maxv;
    }
    if ((rhs < 0) && (lhs < minv - rhs)) {
      return minv;
    }
    return static_cast<T>(lhs + rhs);
  }
}

template <typename T> constexpr auto saturating_sub(T lhs, T rhs) -> T {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    return static_cast<T>(lhs - rhs);
  } else if constexpr (std::is_unsigned_v<T>) {
    return (lhs < rhs) ? T{} : static_cast<T>(lhs - rhs);
  } else {
    auto const maxv = std::numeric_limits<T>::max();
    auto const minv = std::numeric_limits<T>::min();
    if ((rhs < 0) && (lhs > maxv + rhs)) {
      return maxv;
    }
    if ((rhs > 0) && (lhs < minv + rhs)) {
      return minv;
    }
    return static_cast<T>(lhs - rhs);
  }
}

template <typename T> constexpr auto saturating_mul(T lhs, T rhs) -> T {
  if constexpr (!std::is_integral_v<T> || std::is_same_v<T, bool>) {
    return static_cast<T>(lhs * rhs);
  } else {
    if (lhs == T{} || rhs == T{}) {
      return T{};
    }

    if constexpr (std::is_unsigned_v<T>) {
      auto const maxv = std::numeric_limits<T>::max();
      return (lhs > maxv / rhs) ? maxv : static_cast<T>(lhs * rhs);
    } else {
      auto const maxv = std::numeric_limits<T>::max();
      auto const minv = std::numeric_limits<T>::min();

      if (lhs > 0) {
        if (rhs > 0) {
          if (lhs > maxv / rhs) {
            return maxv;
          }
        } else {
          if (rhs < minv / lhs) {
            return minv;
          }
        }
      } else {
        if (rhs > 0) {
          if (lhs < minv / rhs) {
            return minv;
          }
        } else {
          if (lhs != 0 && rhs < maxv / lhs) {
            return maxv;
          }
        }
      }

      return static_cast<T>(lhs * rhs);
    }
  }
}

template <typename CommonRep>
constexpr auto make_unsupported(char const *reason)
    -> policy::value::decision<CommonRep> {
  return make_error<CommonRep>(policy::error::kind::unspecified, reason);
}

template <typename CommonRep>
constexpr auto make_div_zero(char const *reason)
    -> policy::value::decision<CommonRep> {
  return make_error<CommonRep>(policy::error::kind::divide_by_zero, reason);
}

constexpr auto apply_runtime_fence(bool enabled) noexcept -> void {
  if (!enabled) {
    return;
  }

  if (!std::is_constant_evaluated()) {
    std::atomic_thread_fence(std::memory_order_seq_cst);
  }
}

} // namespace details

template <operation OpTag, policy::value_policy ValuePolicy, typename CommonRep>
struct op_binding {
  static constexpr bool enabled = false;

  static constexpr auto apply(CommonRep, CommonRep)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    out.has_value = false;
    out.error.kind = policy::error::kind::unspecified;
    out.error.reason = "operation binding is not implemented";
    return out;
  }
};

template <typename CommonRep>
struct op_binding<Addition, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_add(lhs, rhs); }) {
      return details::checked_add(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked addition not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Subtraction, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_sub(lhs, rhs); }) {
      return details::checked_sub(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked subtraction not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Multiplication, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_mul(lhs, rhs); }) {
      return details::checked_mul(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked multiplication not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Division, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    if constexpr (requires { details::checked_div(lhs, rhs); }) {
      return details::checked_div(lhs, rhs);
    }

    return details::make_unsupported<CommonRep>(
        "checked division not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Addition, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_add(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Subtraction, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_sub(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Multiplication, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_mul(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Division, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::unchecked_div(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Addition, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_add(lhs, rhs); }) {
      out.has_value = true;
      out.value = details::saturating_add(lhs, rhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating addition not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Subtraction, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_sub(lhs, rhs); }) {
      out.has_value = true;
      out.value = details::saturating_sub(lhs, rhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating subtraction not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Multiplication, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    if constexpr (requires { details::saturating_mul(lhs, rhs); }) {
      out.has_value = true;
      out.value = details::saturating_mul(lhs, rhs);
      return out;
    }
    return details::make_unsupported<CommonRep>(
        "saturating multiplication not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Division, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};

    if (rhs == CommonRep{}) {
      return details::make_div_zero<CommonRep>("saturating division by zero");
    }

    if constexpr (requires { lhs / rhs; }) {
      out.has_value = true;
      out.value = static_cast<CommonRep>(lhs / rhs);
      return out;
    }

    return details::make_unsupported<CommonRep>(
        "saturating division not supported for negotiated common type");
  }
};

template <typename CommonRep>
struct op_binding<Equal, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Equal, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<Equal, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<NotEqual, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_not_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<NotEqual, policy::value::unchecked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_not_equal(lhs, rhs);
  }
};

template <typename CommonRep>
struct op_binding<NotEqual, policy::value::saturating, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    return details::compare_not_equal(lhs, rhs);
  }
};

template <typename OpTag, typename ValuePolicy, typename CommonRep>
concept op_binding_available = requires {
  requires operation<OpTag>;
  requires policy::value_policy<ValuePolicy>;
  requires op_binding<OpTag, ValuePolicy, CommonRep>::enabled;
};

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
      op_binding_available<OpTag, ValuePolicy, CommonRep>,
      "Missing operation binding specialization for this OpTag/common type");

  details::apply_runtime_fence(injection.fence_before);

  auto decision = op_binding<OpTag, ValuePolicy, CommonRep>::apply(lhs, rhs);
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

} // namespace mcpplibs::primitives::operations::runtime
