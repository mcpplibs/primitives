module;
#include <tuple>

export module mcpplibs.primitives.primitive.impl;

import mcpplibs.primitives.policy;
import mcpplibs.primitives.underlying;

export namespace mcpplibs::primitives {

template <underlying_type T, policy::policy_type... Policies> class primitive {
public:
  using value_type = T;
  using policies = std::tuple<Policies...>;
  constexpr explicit primitive(value_type v) noexcept : value_(v) {}
  constexpr value_type &value() noexcept { return value_; }
  [[nodiscard]] constexpr value_type const &value() const noexcept { return value_; }
  constexpr explicit operator value_type() const noexcept { return value_; }

private:
  value_type value_;
};

} // namespace mcpplibs::primitives
