/*
 * Example: ex03_value_policy
 *
 * Purpose:
 * Show how checked, unchecked, and saturating value policies behave under
 * the same overflow input, including mixed primitive/underlying binary
 * operations.
 *
 * Expected results:
 * - checked: reports overflow as an error in both primitive+primitive and
 *   primitive+underlying paths.
 * - unchecked: wraps according to native arithmetic semantics in both paths.
 * - saturating: clamps to the representable upper bound in both paths.
 * - Program prints observed values and exits with code 0.
 */

#include <cstdint>
#include <iostream>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 3: Compare checked / unchecked / saturating value policies.
  using rep_t = std::uint16_t;

  using checked_t =
      primitive<rep_t, policy::value::checked, policy::error::expected>;
  using unchecked_t = primitive<rep_t, policy::value::unchecked,
                                policy::error::expected>;
  using saturating_t = primitive<rep_t, policy::value::saturating,
                                 policy::error::expected>;

  // Same overflow input for all three policies.
  auto const lhs_checked = checked_t{static_cast<rep_t>(65530)};
  auto const rhs_checked = checked_t{static_cast<rep_t>(20)};
  auto const checked_result = operations::add(lhs_checked, rhs_checked);
  auto const checked_mixed_result =
      operations::add(lhs_checked, static_cast<rep_t>(20));

  auto const lhs_unchecked = unchecked_t{static_cast<rep_t>(65530)};
  auto const rhs_unchecked = unchecked_t{static_cast<rep_t>(20)};
  auto const unchecked_result = operations::add(lhs_unchecked, rhs_unchecked);
  auto const unchecked_mixed_result =
      operations::add(lhs_unchecked, static_cast<rep_t>(20));

  auto const lhs_saturating = saturating_t{static_cast<rep_t>(65530)};
  auto const rhs_saturating = saturating_t{static_cast<rep_t>(20)};
  auto const saturating_result =
      operations::add(lhs_saturating, rhs_saturating);
  auto const saturating_mixed_result =
      operations::add(lhs_saturating, static_cast<rep_t>(20));

  // Expected outcomes:
  // checked -> error, unchecked -> wrap, saturating -> clamp.
  if (checked_result.has_value() || checked_mixed_result.has_value()) {
    std::cerr << "checked policy should report overflow in both paths\n";
    return 1;
  }
  if (!unchecked_result.has_value() ||
      unchecked_result->value() != static_cast<rep_t>(14) ||
      !unchecked_mixed_result.has_value() ||
      unchecked_mixed_result->value() != static_cast<rep_t>(14)) {
    std::cerr << "unchecked policy should wrap in both paths\n";
    return 1;
  }
  if (!saturating_result.has_value() ||
      saturating_result->value() != static_cast<rep_t>(65535) ||
      !saturating_mixed_result.has_value() ||
      saturating_mixed_result->value() != static_cast<rep_t>(65535)) {
    std::cerr << "saturating policy should clamp in both paths\n";
    return 1;
  }

  std::cout << "checked=overflow, checked_mixed=overflow"
            << ", unchecked=" << unchecked_result->value()
            << ", unchecked_mixed=" << unchecked_mixed_result->value()
            << ", saturating=" << saturating_result->value()
            << ", saturating_mixed=" << saturating_mixed_result->value()
            << '\n';
  return 0;
}
