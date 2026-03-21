module;
#include <tuple>

export module mcpplibs.primitives.primitive.traits;

import mcpplibs.primitives.primitive.impl;
import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.policy.utility;
import mcpplibs.primitives.underlying.traits;

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

template <typename T> struct traits;

template <underlying_type T, policy::policy_type... Policies>
struct traits<primitive<T, Policies...>> {
  using value_type = T;
  using policies = std::tuple<Policies...>;
  using value_policy =
      policy::resolve_policy_t<policy_category::value, Policies...>;
  using type_policy =
      policy::resolve_policy_t<policy_category::type, Policies...>;
  using error_policy =
      policy::resolve_policy_t<policy_category::error, Policies...>;
  using concurrency_policy =
      policy::resolve_policy_t<policy_category::concurrency, Policies...>;
};

} // namespace mcpplibs::primitives::meta
