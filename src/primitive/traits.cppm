module;
#include <tuple>
#include <type_traits>

export module mcpplibs.primitives.primitive.traits;

import mcpplibs.primitives.primitive.impl;
import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.policy.utility;
import mcpplibs.primitives.underlying.traits;

namespace mcpplibs::primitives::meta::details {

template <typename T> struct primitive_traits_impl;

template <underlying_type T, policy::policy_type... Policies>
struct primitive_traits_impl<primitive<T, Policies...>> {
  using value_type = T;
  using policies = std::tuple<Policies...>;
  using value_policy =
      policy::resolve_policy_t<policy::category::value, Policies...>;
  using type_policy =
      policy::resolve_policy_t<policy::category::type, Policies...>;
  using error_policy =
      policy::resolve_policy_t<policy::category::error, Policies...>;
  using concurrency_policy =
      policy::resolve_policy_t<policy::category::concurrency, Policies...>;
};

} // namespace mcpplibs::primitives::meta::details

// Public API exported from this module.
export namespace mcpplibs::primitives::meta {
using policy_category = policy::category;

template <typename T, typename PoliciesTuple> struct make_primitive;

template <underlying_type T, policy::policy_type... Policies>
struct make_primitive<T, std::tuple<Policies...>> {
  using type = primitive<T, Policies...>;
};

template <underlying_type T, typename PoliciesTuple>
using make_primitive_t = make_primitive<T, PoliciesTuple>::type;

using default_policies =
    std::tuple<policy::defaults::value, policy::defaults::type,
               policy::defaults::error, policy::defaults::concurrency>;

template <typename T>
using traits = details::primitive_traits_impl<std::remove_cvref_t<T>>;

template <typename T>
concept primitive_type = requires {
  typename traits<T>::value_type;
  typename traits<T>::policies;
  typename traits<T>::value_policy;
  typename traits<T>::type_policy;
  typename traits<T>::error_policy;
  typename traits<T>::concurrency_policy;
};

template <typename T>
concept boolean =
    primitive_type<T> &&
    (underlying::traits<typename traits<T>::value_type>::kind ==
     underlying::category::boolean);

template <typename T>
concept character =
    primitive_type<T> &&
    (underlying::traits<typename traits<T>::value_type>::kind ==
     underlying::category::character);

template <typename T>
concept integer =
    primitive_type<T> &&
    (underlying::traits<typename traits<T>::value_type>::kind ==
     underlying::category::integer);

template <typename T>
concept floating =
    primitive_type<T> &&
    (underlying::traits<typename traits<T>::value_type>::kind ==
     underlying::category::floating);

template <typename T>
concept numeric = integer<T> || floating<T>;

template <typename T>
concept primitive_like =
    primitive_type<T> || underlying_type<std::remove_cvref_t<T>>;

template <typename T>
concept boolean_like =
    boolean<T> || boolean_underlying_type<std::remove_cvref_t<T>>;

template <typename T>
concept character_like =
    character<T> || character_underlying_type<std::remove_cvref_t<T>>;

template <typename T>
concept integer_like =
    integer<T> || integer_underlying_type<std::remove_cvref_t<T>>;

template <typename T>
concept floating_like =
    floating<T> || floating_underlying_type<std::remove_cvref_t<T>>;

template <typename T>
concept numeric_like =
    numeric<T> || numeric_underlying_type<std::remove_cvref_t<T>>;

} // namespace mcpplibs::primitives::meta
