module;
#include <tuple>
#include <type_traits>

export module mcpplibs.primitives.policy.utility;

import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.impl;

namespace mcpplibs::primitives::policy::details {

template <typename T, typename... Ts> struct contains : std::false_type {};
template <typename T, typename Head, typename... Tail>
struct contains<T, Head, Tail...>
    : std::conditional_t<std::is_same_v<T, Head>, std::true_type,
                         contains<T, Tail...>> {};

template <typename Default, typename PriorityTuple, typename... Ps>
struct pick_first_from_priority_impl;

template <typename Default, typename FirstPriority, typename... RestPriorities,
          typename... Ps>
struct pick_first_from_priority_impl<
    Default, std::tuple<FirstPriority, RestPriorities...>, Ps...> {
  using type = std::conditional_t<
      contains<FirstPriority, Ps...>::value, FirstPriority,
      typename pick_first_from_priority_impl<
          Default, std::tuple<RestPriorities...>, Ps...>::type>;
};

template <typename Default, typename... Ps>
struct pick_first_from_priority_impl<Default, std::tuple<>, Ps...> {
  using type = Default;
};

} // namespace mcpplibs::primitives::policy::details

export namespace mcpplibs::primitives::policy {

// Users can specialize these templates to customize policy selection order.
template <typename = void> struct priority_value {
  using type = std::tuple<>;
};
template <typename = void> struct priority_type {
  using type = std::tuple<>;
};
template <typename = void> struct priority_error {
  using type = std::tuple<>;
};
template <typename = void> struct priority_concurrency {
  using type = std::tuple<>;
};

template <> struct priority_value<void> {
  using type = std::tuple<value::checked, value::saturating, value::unchecked>;
};

template <> struct priority_type<void> {
  using type = std::tuple<type::strict, type::compatible, type::transparent>;
};

template <> struct priority_error<void> {
  using type = std::tuple<error::throwing, error::expected, error::terminate>;
};

template <> struct priority_concurrency<void> {
  using type = std::tuple<concurrency::fenced, concurrency::none>;
};

template <typename... Ps> struct common_policies {
  using value_policy = details::pick_first_from_priority_impl<
      defaults::value, priority_value<>::type, Ps...>::type;

  using type_policy = details::pick_first_from_priority_impl<
      defaults::type, priority_type<>::type, Ps...>::type;

  using error_policy = details::pick_first_from_priority_impl<
      defaults::error, priority_error<>::type, Ps...>::type;

  using concurrency_policy = details::pick_first_from_priority_impl<
      defaults::concurrency, priority_concurrency<>::type, Ps...>::type;

  using type =
      std::tuple<value_policy, type_policy, error_policy, concurrency_policy>;
};

template <typename... Ps>
using common_policies_t = common_policies<Ps...>::type;

} // namespace mcpplibs::primitives::policy
