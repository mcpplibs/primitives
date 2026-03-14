module;
#include <tuple>

export module mcpplibs.primitives.primitive.impl;

import mcpplibs.primitives.traits.policy;
import mcpplibs.primitives.traits.underlying;

export namespace mcpplibs::primitives {

template <underlying_type T,
          policy::policy_type... Policies>
template <underlying_type T,
          policy::policy_type... Policies>
  using value_type = T;
  using policies = std::tuple<Policies...>;
  constexpr explicit primitive(T v) noexcept : value(v) {}
  T value;
};

} // namespace mcpplibs::primitives
