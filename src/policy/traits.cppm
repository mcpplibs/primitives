module;

export module mcpplibs.primitives.policy.traits;

// Primary template and category enum for policy traits. Concrete policy tags
// and specializations live in the implementation submodule.

export namespace mcpplibs::primitives::policy {

enum class category { value, type, error, concurrency };

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

} // namespace mcpplibs::primitives::policy
