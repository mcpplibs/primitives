module;
#include <tuple>
#include <type_traits>

export module mcpplibs.primitives.primitive.traits;

// Import the policy taxonomy so we can inspect policy::traits<>.
export import mcpplibs.primitive.traits.policy;

// Public default policy aliases — these must be visible to internal details.
export namespace mcpplibs::primitives {
using default_value_policy = ::mcpplibs::primitive::policy::unchecked_value;
using default_type_policy = ::mcpplibs::primitive::policy::transparent_type;
using default_error_policy = ::mcpplibs::primitive::policy::throw_error;
using default_concurrency_policy = ::mcpplibs::primitive::policy::single_thread;
}

// Internal implementation details — not exported.
namespace mcpplibs::primitives::traits::details {
using policy_category = ::mcpplibs::primitive::policy::category;

template <policy_category C,
          ::mcpplibs::primitive::policy::policy_type... Policies>
struct count_matching_impl;

template <policy_category C> struct count_matching_impl<C> {
  static constexpr std::size_t value = 0;
};

template <policy_category C, ::mcpplibs::primitive::policy::policy_type First,
          ::mcpplibs::primitive::policy::policy_type... Rest>
struct count_matching_impl<C, First, Rest...> {
  static constexpr std::size_t rest = count_matching_impl<C, Rest...>::value;
  static constexpr bool is_match =
      ::mcpplibs::primitive::policy::traits<First>::enabled &&
      (::mcpplibs::primitive::policy::traits<First>::kind == C);
  static constexpr std::size_t value = rest + (is_match ? 1u : 0u);
};

template <policy_category C,
          ::mcpplibs::primitive::policy::policy_type... Policies>
constexpr std::size_t count_matching_v =
    count_matching_impl<C, Policies...>::value;

template <policy_category C,
          ::mcpplibs::primitive::policy::policy_type... Policies>
struct find_first_impl;

template <policy_category C> struct find_first_impl<C> {
  using type = void;
};

template <policy_category C, ::mcpplibs::primitive::policy::policy_type First,
          ::mcpplibs::primitive::policy::policy_type... Rest>
struct find_first_impl<C, First, Rest...> {
  static constexpr bool is_match =
      ::mcpplibs::primitive::policy::traits<First>::enabled &&
      (::mcpplibs::primitive::policy::traits<First>::kind == C);
  using type = std::conditional_t<is_match, First,
                                  typename find_first_impl<C, Rest...>::type>;
};

template <policy_category C, primitive::policy::policy_type... Policies>
using find_first_t = find_first_impl<C, Policies...>::type;

template <policy_category C, primitive::policy::policy_type... Policies>
struct resolve_policy_impl {
  static_assert(count_matching_v<C, Policies...> <= 1,
                "Multiple policies provided for the same category");

  using found = find_first_t<C, Policies...>;
  using type = std::conditional_t<
      std::is_same_v<found, void>,
      std::conditional_t<
          C == policy_category::value, primitives::default_value_policy,
          std::conditional_t<
              C == policy_category::type, primitives::default_type_policy,
              std::conditional_t<C == policy_category::error,
                                 primitives::default_error_policy,
                                 primitives::default_concurrency_policy>>>,
      found>;
};

template <policy_category C,
          ::mcpplibs::primitive::policy::policy_type... Policies>
using resolve_policy_t = typename resolve_policy_impl<C, Policies...>::type;

} // namespace mcpplibs::primitives::traits::details

// Public API exported from this module.
export namespace mcpplibs::primitives::traits {
    using policy_category = ::mcpplibs::primitive::policy::category;

    template <policy_category C,
              ::mcpplibs::primitive::policy::policy_type... Policies>
    using resolve_policy_t =
        ::mcpplibs::primitives::traits::details::resolve_policy_t<C, Policies...>;

    template <typename P> struct primitive_traits; // customization point for users

    // Forward-declare exported `primitive` (defined in impl module).
    template <typename T, ::mcpplibs::primitive::policy::policy_type... Policies>
    struct primitive;

    template <typename T, ::mcpplibs::primitive::policy::policy_type... Policies>
    struct primitive_traits<primitive<T, Policies...>> {
        using value_type = T;
        using policies = std::tuple<Policies...>;
        using value_policy = resolve_policy_t<policy_category::value, Policies...>;
        using type_policy = resolve_policy_t<policy_category::type, Policies...>;
        using error_policy = resolve_policy_t<policy_category::error, Policies...>;
        using concurrency_policy =
            resolve_policy_t<policy_category::concurrency, Policies...>;
    };

} // namespace traits::traits

