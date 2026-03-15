module;
#include <tuple>
#include <type_traits>
#include <utility>

export module mcpplibs.primitives.operations.traits;

import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.underlying;

export namespace mcpplibs::primitives::operations {

struct add_tag {};
struct sub_tag {};
struct mul_tag {};

template <typename OpTag, typename L, typename R, typename... Policies>
struct traits {
  static constexpr bool enabled = false;
  static constexpr bool noexcept_invocable = false;
  using result_type = void;
};

template <typename OpTag, typename L, typename R, typename... Policies>
concept binary_operation =
    traits<OpTag, std::remove_cvref_t<L>, std::remove_cvref_t<R>,
           Policies...>::enabled;

template <typename OpTag, typename L, typename R, typename... Policies>
concept nothrow_binary_operation =
    binary_operation<OpTag, L, R, Policies...> &&
    traits<OpTag, std::remove_cvref_t<L>, std::remove_cvref_t<R>,
           Policies...>::noexcept_invocable;

template <typename OpTag, typename L, typename R, typename... Policies>
using result_t =
  traits<OpTag, std::remove_cvref_t<L>, std::remove_cvref_t<R>,
         Policies...>::result_type;

namespace details {

template <typename Fallback, typename... Policies>
struct first_value_policy_impl {
  using type = Fallback;
};

template <typename Fallback, typename First, typename... Rest>
struct first_value_policy_impl<Fallback, First, Rest...> {
  using type = std::conditional_t<
      policy::value_policy<First>, First,
      typename first_value_policy_impl<Fallback, Rest...>::type>;
};

template <typename Fallback, typename... Policies>
struct first_type_policy_impl {
  using type = Fallback;
};

template <typename Fallback, typename First, typename... Rest>
struct first_type_policy_impl<Fallback, First, Rest...> {
  using type = std::conditional_t<
      policy::type_policy<First>, First,
      typename first_type_policy_impl<Fallback, Rest...>::type>;
};

template <typename Fallback, typename... Policies>
struct first_error_policy_impl {
  using type = Fallback;
};

template <typename Fallback, typename First, typename... Rest>
struct first_error_policy_impl<Fallback, First, Rest...> {
  using type = std::conditional_t<
      policy::error_policy<First>, First,
      typename first_error_policy_impl<Fallback, Rest...>::type>;
};

template <typename Fallback, typename... Policies>
struct first_concurrency_policy_impl {
  using type = Fallback;
};

template <typename Fallback, typename First, typename... Rest>
struct first_concurrency_policy_impl<Fallback, First, Rest...> {
  using type = std::conditional_t<
      policy::concurrency_policy<First>, First,
      typename first_concurrency_policy_impl<Fallback, Rest...>::type>;
};

} // namespace details

template <policy::value_policy ValuePolicy, typename OpTag = void>
struct value_policy_behavior {
  template <typename T>
  static constexpr T add(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs + rhs);
  }

  template <typename T>
  static constexpr T sub(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs - rhs);
  }

  template <typename T>
  static constexpr T mul(T lhs, T rhs) noexcept {
    return static_cast<T>(lhs * rhs);
  }
};

template <policy::type_policy TypePolicy, typename OpTag = void>
struct type_policy_behavior {
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

template <policy::error_policy ErrorPolicy, typename OpTag = void>
struct error_policy_behavior {
  template <typename Result, typename Fn>
  static constexpr Result evaluate(Fn &&fn) noexcept(noexcept(fn())) {
    return static_cast<Result>(std::forward<Fn>(fn)());
  }
};

template <policy::concurrency_policy ConcurrencyPolicy, typename OpTag = void>
struct concurrency_policy_behavior {
  template <typename Fn>
  static constexpr auto execute(Fn &&fn) noexcept(noexcept(fn()))
      -> decltype(fn()) {
    return std::forward<Fn>(fn)();
  }
};

template <typename... Policies>
using resolved_value_policy_t =
  details::first_value_policy_impl<
        std::tuple_element_t<0, policy::common_policies_t<Policies...>>,
        Policies...>::type;

template <typename... Policies>
using resolved_type_policy_t =
  details::first_type_policy_impl<
        std::tuple_element_t<1, policy::common_policies_t<Policies...>>,
        Policies...>::type;

template <typename... Policies>
using resolved_error_policy_t =
  details::first_error_policy_impl<
        std::tuple_element_t<2, policy::common_policies_t<Policies...>>,
        Policies...>::type;

template <typename... Policies>
using resolved_concurrency_policy_t =
  details::first_concurrency_policy_impl<
        std::tuple_element_t<3, policy::common_policies_t<Policies...>>,
        Policies...>::type;

} // namespace mcpplibs::primitives::operations
