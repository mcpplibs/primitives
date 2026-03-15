module;
#include <atomic>
#include <concepts>
#include <expected>
#include <exception>
#include <limits>
#include <stdexcept>
#include <type_traits>

export module mcpplibs.primitives.operations.impl;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.underlying;

export namespace mcpplibs::primitives::operations {

namespace details {

template <typename T>
struct is_expected : std::false_type {};

template <typename V, typename E>
struct is_expected<std::expected<V, E>> : std::true_type {};

template <typename T>
inline constexpr bool is_expected_v = is_expected<T>::value;

template <typename T> constexpr bool add_overflow(T lhs, T rhs) noexcept {
  if constexpr (!std::is_integral_v<T>) {
    return false;
  } else if constexpr (std::is_signed_v<T>) {
    using limits = std::numeric_limits<T>;
    return (rhs > 0 && lhs > limits::max() - rhs) ||
           (rhs < 0 && lhs < limits::min() - rhs);
  } else {
    using limits = std::numeric_limits<T>;
    return lhs > limits::max() - rhs;
  }
}

template <typename T> constexpr bool sub_overflow(T lhs, T rhs) noexcept {
  if constexpr (!std::is_integral_v<T>) {
    return false;
  } else if constexpr (std::is_signed_v<T>) {
    using limits = std::numeric_limits<T>;
    return (rhs < 0 && lhs > limits::max() + rhs * static_cast<T>(-1)) ||
           (rhs > 0 && lhs < limits::min() + rhs);
  } else {
    return lhs < rhs;
  }
}

template <typename T> constexpr bool mul_overflow(T lhs, T rhs) noexcept {
  if constexpr (!std::is_integral_v<T>) {
    return false;
  } else if (lhs == 0 || rhs == 0) {
    return false;
  } else if constexpr (std::is_signed_v<T>) {
    using limits = std::numeric_limits<T>;
    if (lhs == -1) {
      return rhs == limits::min();
    }
    if (rhs == -1) {
      return lhs == limits::min();
    }
    if (lhs > 0) {
      return (rhs > 0) ? lhs > limits::max() / rhs : rhs < limits::min() / lhs;
    }
    return (rhs > 0) ? lhs < limits::min() / rhs
                     : lhs != 0 && rhs < limits::max() / lhs;
  } else {
    using limits = std::numeric_limits<T>;
    return lhs > limits::max() / rhs;
  }
}

template <typename T> constexpr T saturating_add(T lhs, T rhs) noexcept {
  if constexpr (!std::is_integral_v<T>) {
    return static_cast<T>(lhs + rhs);
  } else {
    using limits = std::numeric_limits<T>;
    if (!add_overflow(lhs, rhs)) {
      return static_cast<T>(lhs + rhs);
    }
    if constexpr (std::is_signed_v<T>) {
      return rhs >= 0 ? limits::max() : limits::min();
    } else {
      return limits::max();
    }
  }
}

template <typename T> constexpr T saturating_sub(T lhs, T rhs) noexcept {
  if constexpr (!std::is_integral_v<T>) {
    return static_cast<T>(lhs - rhs);
  } else {
    using limits = std::numeric_limits<T>;
    if (!sub_overflow(lhs, rhs)) {
      return static_cast<T>(lhs - rhs);
    }
    if constexpr (std::is_signed_v<T>) {
      return rhs > 0 ? limits::min() : limits::max();
    } else {
      return limits::min();
    }
  }
}

template <typename T> constexpr T saturating_mul(T lhs, T rhs) noexcept {
  if constexpr (!std::is_integral_v<T>) {
    return static_cast<T>(lhs * rhs);
  } else {
    using limits = std::numeric_limits<T>;
    if (!mul_overflow(lhs, rhs)) {
      return static_cast<T>(lhs * rhs);
    }
    if constexpr (std::is_signed_v<T>) {
      return ((lhs < 0) ^ (rhs < 0)) ? limits::min() : limits::max();
    } else {
      return limits::max();
    }
  }
}

} // namespace details

// ===== Built-in value policy behaviors =====

template <typename OpTag>
struct value_policy_behavior<policy::checked_value, OpTag> {
  template <typename T> static T add(T lhs, T rhs) {
    if (details::add_overflow<T>(lhs, rhs)) {
      throw std::overflow_error("overflow in add");
    }
    return static_cast<T>(lhs + rhs);
  }

  template <typename T> static T sub(T lhs, T rhs) {
    if (details::sub_overflow<T>(lhs, rhs)) {
      throw std::overflow_error("overflow in sub");
    }
    return static_cast<T>(lhs - rhs);
  }

  template <typename T> static T mul(T lhs, T rhs) {
    if (details::mul_overflow<T>(lhs, rhs)) {
      throw std::overflow_error("overflow in mul");
    }
    return static_cast<T>(lhs * rhs);
  }
};

template <typename OpTag>
struct value_policy_behavior<policy::unchecked_value, OpTag> {
  template <typename T> static constexpr T add(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs + rhs);
  }

  template <typename T> static constexpr T sub(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs - rhs);
  }

  template <typename T> static constexpr T mul(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs * rhs);
  }
};

template <typename OpTag>
struct value_policy_behavior<policy::saturating_value, OpTag> {
  template <typename T> static constexpr T add(T lhs, T rhs) noexcept {
    return details::saturating_add(lhs, rhs);
  }

  template <typename T> static constexpr T sub(T lhs, T rhs) noexcept {
    return details::saturating_sub(lhs, rhs);
  }

  template <typename T> static constexpr T mul(T lhs, T rhs) noexcept {
    return details::saturating_mul(lhs, rhs);
  }
};

// ===== Built-in type policy behaviors =====

template <typename OpTag>
struct type_policy_behavior<policy::strict_type, OpTag> {
  template <typename L, typename R> using result_type = std::remove_cv_t<L>;

  template <typename Out, typename In>
  static constexpr Out cast_lhs(In const &value) noexcept {
    return static_cast<Out>(value);
  }

  template <typename Out, typename In>
  static constexpr Out cast_rhs(In const &value) noexcept {
    static_assert(std::same_as<std::remove_cv_t<Out>, std::remove_cv_t<In>>,
                  "strict_type requires both operands to have same type");
    return static_cast<Out>(value);
  }
};

template <typename OpTag>
struct type_policy_behavior<policy::category_compatible_type, OpTag> {
  template <typename L, typename R>
  using result_type = std::common_type_t<L, R>;

  template <typename Out, typename In>
  static constexpr Out cast_lhs(In const &value) noexcept {
    return static_cast<Out>(value);
  }

  template <typename Out, typename In>
  static constexpr Out cast_rhs(In const &value) noexcept {
    static_assert(underlying::traits<std::remove_cv_t<Out>>::kind ==
                      underlying::traits<std::remove_cv_t<In>>::kind,
                  "category_compatible_type requires same underlying category");
    return static_cast<Out>(value);
  }
};

template <typename OpTag>
struct type_policy_behavior<policy::transparent_type, OpTag> {
  template <typename L, typename R>
  using result_type = std::common_type_t<L, R>;

  template <typename Out, typename In>
  static constexpr Out cast_lhs(In const &value) noexcept {
    return static_cast<Out>(value);
  }

  template <typename Out, typename In>
  static constexpr Out cast_rhs(In const &value) noexcept {
    return static_cast<Out>(value);
  }
};

// ===== Built-in error policy behaviors =====

template <typename OpTag>
struct error_policy_behavior<policy::throw_error, OpTag> {
  template <typename Result, typename Fn>
  static constexpr Result evaluate(Fn &&fn) noexcept(noexcept(fn())) {
    return static_cast<Result>(fn());
  }
};

template <typename OpTag>
struct error_policy_behavior<policy::expected_error, OpTag> {
  template <typename Result, typename Fn>
  static constexpr Result evaluate(Fn &&fn) noexcept(noexcept(fn())) {
    if constexpr (details::is_expected_v<Result>) {
      return Result{std::in_place, std::forward<Fn>(fn)()};
    } else {
      return static_cast<Result>(fn());
    }
  }
};

template <typename OpTag>
struct error_policy_behavior<policy::terminate_error, OpTag> {
  template <typename Result, typename Fn>
  static Result evaluate(Fn &&fn) noexcept {
    if constexpr (noexcept(fn())) {
      return static_cast<Result>(fn());
    } else {
      try {
        return static_cast<Result>(fn());
      } catch (...) {
        std::terminate();
      }
    }
  }
};

// ===== Built-in concurrency policy behaviors =====

template <typename OpTag>
struct concurrency_policy_behavior<policy::single_thread, OpTag> {
  template <typename Fn>
  static constexpr auto execute(Fn &&fn) noexcept(noexcept(fn()))
      -> decltype(fn()) {
    return fn();
  }
};

template <typename OpTag>
struct concurrency_policy_behavior<policy::atomic, OpTag> {
  template <typename Fn>
  static auto execute(Fn &&fn) noexcept(noexcept(fn())) -> decltype(fn()) {
    std::atomic_signal_fence(std::memory_order_seq_cst);
    auto result = fn();
    std::atomic_signal_fence(std::memory_order_seq_cst);
    return result;
  }
};

// default underlying operations

template <underlying_type L, underlying_type R, policy::policy_type... Policies>
struct traits<add_tag, L, R, Policies...> {
  static constexpr bool enabled = true;
  static constexpr bool noexcept_invocable = true;

  using value_policy = resolved_value_policy_t<Policies...>;
  using type_policy = resolved_type_policy_t<Policies...>;
  using error_policy = resolved_error_policy_t<Policies...>;
  using concurrency_policy = resolved_concurrency_policy_t<Policies...>;

  using result_type =
      type_policy_behavior<type_policy, add_tag>::template result_type<L, R>;

  static constexpr result_type invoke(L const &lhs, R const &rhs) noexcept {
    return concurrency_policy_behavior<
        concurrency_policy, add_tag>::execute([&]() constexpr noexcept {
      return error_policy_behavior<error_policy, add_tag>::template evaluate<
          result_type>([&]() constexpr noexcept {
        return value_policy_behavior<value_policy, add_tag>::template add<
            result_type>(
            type_policy_behavior<type_policy,
                                 add_tag>::template cast_lhs<result_type>(lhs),
            type_policy_behavior<type_policy,
                                 add_tag>::template cast_rhs<result_type>(rhs));
      });
    });
  }
};

template <underlying_type L, underlying_type R, policy::policy_type... Policies>
struct traits<sub_tag, L, R, Policies...> {
  static constexpr bool enabled = true;
  static constexpr bool noexcept_invocable = true;

  using value_policy = resolved_value_policy_t<Policies...>;
  using type_policy = resolved_type_policy_t<Policies...>;
  using error_policy = resolved_error_policy_t<Policies...>;
  using concurrency_policy = resolved_concurrency_policy_t<Policies...>;

  using result_type =
      type_policy_behavior<type_policy, sub_tag>::template result_type<L, R>;

  static constexpr result_type invoke(L const &lhs, R const &rhs) noexcept {
    return concurrency_policy_behavior<
        concurrency_policy, sub_tag>::execute([&]() constexpr noexcept {
      return error_policy_behavior<error_policy, sub_tag>::template evaluate<
          result_type>([&]() constexpr noexcept {
        return value_policy_behavior<value_policy, sub_tag>::template sub<
            result_type>(
            type_policy_behavior<type_policy,
                                 sub_tag>::template cast_lhs<result_type>(lhs),
            type_policy_behavior<type_policy,
                                 sub_tag>::template cast_rhs<result_type>(rhs));
      });
    });
  }
};

template <underlying_type L, underlying_type R, policy::policy_type... Policies>
struct traits<mul_tag, L, R, Policies...> {
  static constexpr bool enabled = true;
  static constexpr bool noexcept_invocable = true;

  using value_policy = resolved_value_policy_t<Policies...>;
  using type_policy = resolved_type_policy_t<Policies...>;
  using error_policy = resolved_error_policy_t<Policies...>;
  using concurrency_policy = resolved_concurrency_policy_t<Policies...>;

  using result_type =
      type_policy_behavior<type_policy, mul_tag>::template result_type<L, R>;

  static constexpr result_type invoke(L const &lhs, R const &rhs) noexcept {
    return concurrency_policy_behavior<
        concurrency_policy, mul_tag>::execute([&]() constexpr noexcept {
      return error_policy_behavior<error_policy, mul_tag>::template evaluate<
          result_type>([&]() constexpr noexcept {
        return value_policy_behavior<value_policy, mul_tag>::template mul<
            result_type>(
            type_policy_behavior<type_policy,
                                 mul_tag>::template cast_lhs<result_type>(lhs),
            type_policy_behavior<type_policy,
                                 mul_tag>::template cast_rhs<result_type>(rhs));
      });
    });
  }
};

// primitive operations (keep lhs policy set by default)
template <underlying_type L, policy::policy_type... LPs, underlying_type R,
          policy::policy_type... RPs, policy::policy_type... Policies>
struct traits<add_tag, primitive<L, LPs...>, primitive<R, RPs...>,
              Policies...> {
  static constexpr bool enabled = true;
  static constexpr bool noexcept_invocable = true;

  using value_policy = resolved_value_policy_t<LPs..., Policies...>;
  using type_policy = resolved_type_policy_t<LPs..., Policies...>;
  using error_policy = resolved_error_policy_t<LPs..., Policies...>;
  using concurrency_policy = resolved_concurrency_policy_t<LPs..., Policies...>;

  using rep_type =
      type_policy_behavior<type_policy, add_tag>::template result_type<L, R>;
    using raw_result_type = primitive<rep_type, LPs...>;
    using result_type = std::conditional_t<
      std::is_same_v<error_policy, policy::expected_error>,
      std::expected<raw_result_type, std::overflow_error>, raw_result_type>;

  static result_type invoke(primitive<L, LPs...> const &lhs,
                            primitive<R, RPs...> const &rhs) {
    // Prepare casted representations
    rep_type lhs_rep =
        type_policy_behavior<type_policy, add_tag>::template cast_lhs<rep_type>(
            lhs.value());
    rep_type rhs_rep =
        type_policy_behavior<type_policy, add_tag>::template cast_rhs<rep_type>(
            rhs.value());

    // If value policy is checked, handle overflow according to error policy.
    if constexpr (std::is_same_v<value_policy, policy::checked_value>) {
      if (details::add_overflow<rep_type>(lhs_rep, rhs_rep)) {
        if constexpr (std::is_same_v<error_policy, policy::throw_error>) {
          throw std::overflow_error("overflow in add");
        } else if constexpr (std::is_same_v<error_policy,
                                            policy::terminate_error>) {
          std::terminate();
        } else if constexpr (std::is_same_v<error_policy,
                                            policy::expected_error>) {
          return std::unexpected(std::overflow_error("overflow in add"));
        }
      }
    }

    const auto rep = concurrency_policy_behavior<concurrency_policy,
                                                 add_tag>::execute([&]() {
      return error_policy_behavior<error_policy,
                                   add_tag>::template evaluate<rep_type>([&]() {
        return value_policy_behavior<value_policy,
                                     add_tag>::template add<rep_type>(lhs_rep,
                                                                      rhs_rep);
      });
    });

    if constexpr (std::is_same_v<error_policy, policy::expected_error>) {
      return raw_result_type{rep};
    } else {
      return raw_result_type{rep};
    }
  }
};

template <underlying_type L, policy::policy_type... LPs, underlying_type R,
          policy::policy_type... RPs, policy::policy_type... Policies>
struct traits<sub_tag, primitive<L, LPs...>, primitive<R, RPs...>,
              Policies...> {
  static constexpr bool enabled = true;
  static constexpr bool noexcept_invocable = true;

  using value_policy = resolved_value_policy_t<LPs..., Policies...>;
  using type_policy = resolved_type_policy_t<LPs..., Policies...>;
  using error_policy = resolved_error_policy_t<LPs..., Policies...>;
  using concurrency_policy = resolved_concurrency_policy_t<LPs..., Policies...>;

  using rep_type =
      type_policy_behavior<type_policy, sub_tag>::template result_type<L, R>;
    using raw_result_type = primitive<rep_type, LPs...>;
    using result_type = std::conditional_t<
      std::is_same_v<error_policy, policy::expected_error>,
      std::expected<raw_result_type, std::overflow_error>, raw_result_type>;

  static result_type invoke(primitive<L, LPs...> const &lhs,
                            primitive<R, RPs...> const &rhs) {
    rep_type lhs_rep =
        type_policy_behavior<type_policy, sub_tag>::template cast_lhs<rep_type>(
            lhs.value());
    rep_type rhs_rep =
        type_policy_behavior<type_policy, sub_tag>::template cast_rhs<rep_type>(
            rhs.value());

    if constexpr (std::is_same_v<value_policy, policy::checked_value>) {
      if (details::sub_overflow<rep_type>(lhs_rep, rhs_rep)) {
        if constexpr (std::is_same_v<error_policy, policy::throw_error>) {
          throw std::overflow_error("overflow in sub");
        } else if constexpr (std::is_same_v<error_policy,
                                            policy::terminate_error>) {
          std::terminate();
        } else if constexpr (std::is_same_v<error_policy,
                                            policy::expected_error>) {
          return std::unexpected(std::overflow_error("overflow in sub"));
        }
      }
    }

    const auto rep = concurrency_policy_behavior<concurrency_policy,
                                                 sub_tag>::execute([&]() {
      return error_policy_behavior<error_policy,
                                   sub_tag>::template evaluate<rep_type>([&]() {
        return value_policy_behavior<value_policy, sub_tag>::template sub<
            rep_type>(lhs_rep, rhs_rep);
      });
    });

    if constexpr (std::is_same_v<error_policy, policy::expected_error>) {
      return raw_result_type{rep};
    } else {
      return raw_result_type{rep};
    }
  }
};

template <underlying_type L, policy::policy_type... LPs, underlying_type R,
          policy::policy_type... RPs, policy::policy_type... Policies>
struct traits<mul_tag, primitive<L, LPs...>, primitive<R, RPs...>,
              Policies...> {
  static constexpr bool enabled = true;
  static constexpr bool noexcept_invocable = true;

  using value_policy = resolved_value_policy_t<LPs..., Policies...>;
  using type_policy = resolved_type_policy_t<LPs..., Policies...>;
  using error_policy = resolved_error_policy_t<LPs..., Policies...>;
  using concurrency_policy = resolved_concurrency_policy_t<LPs..., Policies...>;

  using rep_type =
      type_policy_behavior<type_policy, mul_tag>::template result_type<L, R>;
    using raw_result_type = primitive<rep_type, LPs...>;
    using result_type = std::conditional_t<
      std::is_same_v<error_policy, policy::expected_error>,
      std::expected<raw_result_type, std::overflow_error>, raw_result_type>;

  static result_type invoke(primitive<L, LPs...> const &lhs,
                            primitive<R, RPs...> const &rhs) {
    rep_type lhs_rep =
        type_policy_behavior<type_policy, mul_tag>::template cast_lhs<rep_type>(
            lhs.value());
    rep_type rhs_rep =
        type_policy_behavior<type_policy, mul_tag>::template cast_rhs<rep_type>(
            rhs.value());

    if constexpr (std::is_same_v<value_policy, policy::checked_value>) {
      if (details::mul_overflow<rep_type>(lhs_rep, rhs_rep)) {
        if constexpr (std::is_same_v<error_policy, policy::throw_error>) {
          throw std::overflow_error("overflow in mul");
        } else if constexpr (std::is_same_v<error_policy,
                                            policy::terminate_error>) {
          std::terminate();
        } else if constexpr (std::is_same_v<error_policy,
                                            policy::expected_error>) {
          return std::unexpected(std::overflow_error("overflow in mul"));
        }
      }
    }

    const auto rep = concurrency_policy_behavior<concurrency_policy,
                                                 mul_tag>::execute([&]() {
      return error_policy_behavior<error_policy,
                                   mul_tag>::template evaluate<rep_type>([&]() {
        return value_policy_behavior<value_policy, mul_tag>::template mul<
            rep_type>(lhs_rep, rhs_rep);
      });
    });

    if constexpr (std::is_same_v<error_policy, policy::expected_error>) {
      return raw_result_type{rep};
    } else {
      return raw_result_type{rep};
    }
  }
};

struct add_fn {
  template <policy::policy_type... Policies, typename L, typename R>
    requires binary_operation<add_tag, L, R, Policies...>
  auto operator()(L &&lhs, R &&rhs) const
      -> result_t<add_tag, L, R, Policies...> {
    using impl = traits<add_tag, std::remove_cvref_t<L>, std::remove_cvref_t<R>,
                        Policies...>;
    return impl::invoke(lhs, rhs);
  }
};

struct sub_fn {
  template <policy::policy_type... Policies, typename L, typename R>
    requires binary_operation<sub_tag, L, R, Policies...>
  auto operator()(L &&lhs, R &&rhs) const
      -> result_t<sub_tag, L, R, Policies...> {
    using impl = traits<sub_tag, std::remove_cvref_t<L>, std::remove_cvref_t<R>,
                        Policies...>;
    return impl::invoke(lhs, rhs);
  }
};

struct mul_fn {
  template <policy::policy_type... Policies, typename L, typename R>
    requires binary_operation<mul_tag, L, R, Policies...>
  auto operator()(L &&lhs, R &&rhs) const
      -> result_t<mul_tag, L, R, Policies...> {
    using impl = traits<mul_tag, std::remove_cvref_t<L>, std::remove_cvref_t<R>,
                        Policies...>;
    return impl::invoke(lhs, rhs);
  }
};

inline constexpr add_fn add{};
inline constexpr sub_fn sub{};
inline constexpr mul_fn mul{};

} // namespace mcpplibs::primitives::operations
