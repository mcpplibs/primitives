module;
#include <tuple>
#include <type_traits>

export module mcpplibs.primitives.primitive.traits;

import mcpplibs.primitives.primitive.impl;
import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.underlying.traits;

// Internal implementation details - not exported.
namespace mcpplibs::primitives::traits::details {
using policy_category = policy::category;

template <policy_category C, policy::policy_type... Policies>
struct count_matching_impl;

template <policy_category C> struct count_matching_impl<C> {
  static constexpr std::size_t value = 0;
};

template <policy_category C, policy::policy_type First,
          policy::policy_type... Rest>
struct count_matching_impl<C, First, Rest...> {
  static constexpr std::size_t rest = count_matching_impl<C, Rest...>::value;
  static constexpr bool is_match =
      policy::traits<First>::enabled && policy::traits<First>::kind == C;
  static constexpr std::size_t value = rest + (is_match ? 1u : 0u);
};

template <policy_category C, policy::policy_type... Policies>
constexpr std::size_t count_matching_v =
    count_matching_impl<C, Policies...>::value;

template <policy_category C, policy::policy_type... Policies>
struct find_first_impl;

template <policy_category C> struct find_first_impl<C> {
  using type = void;
};

template <policy_category C, policy::policy_type First,
          policy::policy_type... Rest>
struct find_first_impl<C, First, Rest...> {
  static constexpr bool is_match =
      policy::traits<First>::enabled && policy::traits<First>::kind == C;
  using type = std::conditional_t<is_match, First,
                                  typename find_first_impl<C, Rest...>::type>;
};

template <policy_category C, policy::policy_type... Policies>
using find_first_t = find_first_impl<C, Policies...>::type;

template <policy_category C, policy::policy_type... Policies>
struct resolve_policy_impl {
  static_assert(count_matching_v<C, Policies...> <= 1,
                "Multiple policies provided for the same category");

  using found = find_first_t<C, Policies...>;
  using type = std::conditional_t<
      std::is_same_v<found, void>,
      std::conditional_t<
          C == policy_category::value, policy::defaults::value,
          std::conditional_t<
              C == policy_category::type, policy::defaults::type,
              std::conditional_t<C == policy_category::error,
                                 policy::defaults::error,
                                 policy::defaults::concurrency>>>,
      found>;
};

template <policy_category C, policy::policy_type... Policies>
using resolve_policy_t = resolve_policy_impl<C, Policies...>::type;

} // namespace mcpplibs::primitives::traits::details

// Public API exported from this module.
export namespace mcpplibs::primitives::traits {
using policy_category = policy::category;

template <policy_category C, policy::policy_type... Policies>
using resolve_policy_t = details::resolve_policy_t<C, Policies...>;

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

template <typename T> struct primitive_traits;

template <underlying_type T, policy::policy_type... Policies>
struct primitive_traits<primitive<T, Policies...>> {
  using value_type = T;
  using policies = std::tuple<Policies...>;
  using value_policy = resolve_policy_t<policy_category::value, Policies...>;
  using type_policy = resolve_policy_t<policy_category::type, Policies...>;
  using error_policy = resolve_policy_t<policy_category::error, Policies...>;
  using concurrency_policy =
      resolve_policy_t<policy_category::concurrency, Policies...>;
};

} // namespace mcpplibs::primitives::traits
