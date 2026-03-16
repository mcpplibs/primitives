module;
#include <tuple>
#include <type_traits>

export module mcpplibs.primitives.policy.traits;

// Primary template and category enum for policy traits. Concrete policy tags
// and specializations live in the implementation submodule.

export namespace mcpplibs::primitives::policy {

enum class category { value, type, error, concurrency };

// Built-in policy tags (defined here so `common_policies` can live in traits).
struct checked_value {};
struct unchecked_value {};
struct saturating_value {};

struct strict_type {};
struct category_compatible_type {};
struct transparent_type {};

struct throw_error {};
struct expected_error {};
struct terminate_error {};

struct single_thread {};
struct atomic {};

template <typename P> struct traits {
  using policy_type = void;
  static constexpr bool enabled = false;
  static constexpr auto kind = static_cast<category>(-1);
};

template <typename P>
concept policy_type = traits<P>::enabled;

template <typename P>
concept value_policy = policy_type<P> && (traits<P>::kind == category::value);

template <typename P>
concept type_policy = policy_type<P> && (traits<P>::kind == category::type);

template <typename P>
concept error_policy = policy_type<P> && (traits<P>::kind == category::error);

template <typename P>
concept concurrency_policy =
    policy_type<P> && (traits<P>::kind == category::concurrency);

// Customizable priority hooks — users can specialize these templates to
// change how the library decides which policy wins when multiple are
// provided. Specializations live in implementation or user code.
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

// Library default priority ordering for built-in tags. Users can specialize
// these hooks in their own code to change global priority ordering.
template <> struct priority_value<void> {
  using type = std::tuple<checked_value, saturating_value, unchecked_value>;
};

template <> struct priority_type<void> {
  using type =
      std::tuple<strict_type, category_compatible_type, transparent_type>;
};

template <> struct priority_error<void> {
  using type = std::tuple<throw_error, expected_error, terminate_error>;
};

template <> struct priority_concurrency<void> {
  using type = std::tuple<atomic, single_thread>;
};

// Default hooks for library defaults. Implementations should specialize
// these in the implementation submodule to bind to concrete tags.
// Library defaults (built-in tags).
using default_value = checked_value;
using default_type = strict_type;
using default_error = throw_error;
using default_concurrency = single_thread;

// Internal helpers used by the merging algorithm.
namespace details {
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

} // namespace details

// Primary merging metafunction. Consumers can use `common_policies_t<...>`.
template <typename... Ps> struct common_policies {
  using value_policy = details::pick_first_from_priority_impl<
      default_value, priority_value<>::type, Ps...>::type;

  using type_policy = details::pick_first_from_priority_impl<
      default_type, priority_type<>::type, Ps...>::type;

  using error_policy = details::pick_first_from_priority_impl<
      default_error, priority_error<>::type, Ps...>::type;

  using concurrency_policy = details::pick_first_from_priority_impl<
      default_concurrency, priority_concurrency<>::type, Ps...>::type;

  using type =
      std::tuple<value_policy, type_policy, error_policy, concurrency_policy>;
};

template <typename... Ps>
using common_policies_t = common_policies<Ps...>::type;

} // namespace mcpplibs::primitives::policy
