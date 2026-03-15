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

} // namespace mcpplibs::primitives::policy
