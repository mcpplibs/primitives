module;
#include <cstddef>
#include <type_traits>

export module mcpplibs.primitives.policy.utility;

import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.impl;

namespace mcpplibs::primitives::policy::details {
using policy_category = category;

template <policy_category C, policy_type... Policies>
struct count_matching_impl;

template <policy_category C> struct count_matching_impl<C> {
  static constexpr std::size_t value = 0;
};

template <policy_category C, policy_type First, policy_type... Rest>
struct count_matching_impl<C, First, Rest...> {
  static constexpr std::size_t rest = count_matching_impl<C, Rest...>::value;
  static constexpr bool is_match =
      traits<First>::enabled && traits<First>::kind == C;
  static constexpr std::size_t value = rest + (is_match ? 1u : 0u);
};

template <policy_category C, policy_type... Policies>
constexpr std::size_t count_matching_v =
    count_matching_impl<C, Policies...>::value;

template <policy_category C, policy_type... Policies>
struct find_first_impl;

template <policy_category C> struct find_first_impl<C> {
  using type = void;
};

template <policy_category C, policy_type First, policy_type... Rest>
struct find_first_impl<C, First, Rest...> {
  static constexpr bool is_match =
      traits<First>::enabled && traits<First>::kind == C;
  using type = std::conditional_t<is_match, First,
                                  typename find_first_impl<C, Rest...>::type>;
};

template <policy_category C, policy_type... Policies>
using find_first_t = find_first_impl<C, Policies...>::type;

template <policy_category C, policy_type... Policies>
struct resolve_policy_impl {
  static_assert(count_matching_v<C, Policies...> <= 1,
                "Multiple policies provided for the same category");

  using found = find_first_t<C, Policies...>;
  using type = std::conditional_t<
      std::is_same_v<found, void>,
      std::conditional_t<
          C == policy_category::value, defaults::value,
          std::conditional_t<C == policy_category::type, defaults::type,
                             std::conditional_t<C == policy_category::error,
                                                defaults::error,
                                                defaults::concurrency>>>,
      found>;
};

} // namespace mcpplibs::primitives::policy::details

export namespace mcpplibs::primitives::policy {
using policy_category = category;

template <policy_category C, policy_type... Policies>
using resolve_policy_t = details::resolve_policy_impl<C, Policies...>::type;
} // namespace mcpplibs::primitives::policy
