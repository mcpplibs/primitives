module;
#include <tuple>
#include <type_traits>

export module mcpplibs.primitives.primitive.traits;

import mcpplibs.primitives.primitive.impl;
import mcpplibs.primitives.traits.policy;
import mcpplibs.primitives.traits.underlying;


// Internal implementation details — not exported.
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
          C == policy_category::value, policy::default_value,
          std::conditional_t<C == policy_category::type, policy::default_type,
                             std::conditional_t<C == policy_category::error,
                                                policy::default_error,
                                                policy::default_concurrency>>>,
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

template <typename P> struct primitive_traits; // customization point for users

// Forward-declare exported `primitive` (defined in impl module).
template <typename T, policy::policy_type... Policies> struct primitive;

template <typename T, policy::policy_type... Policies>
using default_policies =
    std::tuple<
        policy::default_value,
        policy::default_type,
        policy::default_error,
        policy::default_concurrency
    >;
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
