module;
#include <tuple>

export module mcpplibs.primitives.primitive.impl;

import mcpplibs.primitives.traits.policy;
import mcpplibs.primitives.traits.underlying;

export namespace mcpplibs::primitives {

template <underlying_type T, policy::policy_type... Policies> class primitive {
public:
  using value_type = T;
  using policies = std::tuple<Policies...>;
  constexpr explicit primitive(T v) noexcept : value_(v) {}
  constexpr value_type &value() noexcept { return value_; }
  constexpr value_type const &value() const noexcept { return value_; }

private:
  T value_;
};

} // namespace mcpplibs::primitives
