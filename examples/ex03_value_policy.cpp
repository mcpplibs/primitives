#include <cstdint>
#include <iostream>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 3: Compare checked / unchecked / saturating value policies.
  using checked_t =
      primitive<std::uint16_t, policy::value::checked, policy::error::expected>;
  using unchecked_t = primitive<std::uint16_t, policy::value::unchecked,
                                policy::error::expected>;
  using saturating_t = primitive<std::uint16_t, policy::value::saturating,
                                 policy::error::expected>;

  // Same overflow input for all three policies.
  auto const lhs_checked = checked_t{65530};
  auto const rhs_checked = checked_t{20};
  auto const checked_result = operations::add(lhs_checked, rhs_checked);

  auto const lhs_unchecked = unchecked_t{65530};
  auto const rhs_unchecked = unchecked_t{20};
  auto const unchecked_result = operations::add(lhs_unchecked, rhs_unchecked);

  auto const lhs_saturating = saturating_t{65530};
  auto const rhs_saturating = saturating_t{20};
  auto const saturating_result =
      operations::add(lhs_saturating, rhs_saturating);

  // Expected outcomes:
  // checked -> error, unchecked -> wrap, saturating -> clamp.
  if (checked_result.has_value()) {
    std::cerr << "checked policy should report overflow\n";
    return 1;
  }
  if (!unchecked_result.has_value() || unchecked_result->value() != 14) {
    std::cerr << "unchecked policy should wrap\n";
    return 1;
  }
  if (!saturating_result.has_value() || saturating_result->value() != 65535) {
    std::cerr << "saturating policy should clamp\n";
    return 1;
  }

  std::cout << "checked=overflow, unchecked=" << unchecked_result->value()
            << ", saturating=" << saturating_result->value() << '\n';
  return 0;
}
