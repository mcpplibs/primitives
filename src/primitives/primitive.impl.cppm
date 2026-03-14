module;
#include <tuple>

export module mcpplibs.primitives.primitive.impl;

// Import policy concept to constrain Policies parameter pack.
export import mcpplibs.primitive.traits.policy;

export namespace mcpplibs::primitives {

template <typename T, primitive::policy::policy_type... Policies>
struct primitive {
  using value_type = T;
  using policies = std::tuple<Policies...>;
  constexpr explicit primitive(T v) noexcept : value(v) {}
  T value;
};

} // namespace mcpplibs::primitives
