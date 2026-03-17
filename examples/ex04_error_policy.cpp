#include <iostream>
#include <stdexcept>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 4: Compare error_policy behavior under divide-by-zero.
  using expected_t =
      primitive<int, policy::value::checked, policy::error::expected>;
  using throwing_t =
      primitive<int, policy::value::checked, policy::error::throwing>;

  // Case A: expected policy returns error payload.
  auto const e_lhs = expected_t{42};
  auto const e_rhs = expected_t{0};
  auto const expected_result = operations::div(e_lhs, e_rhs);
  if (expected_result.has_value() ||
      expected_result.error() != policy::error::kind::divide_by_zero) {
    std::cerr << "expected policy did not return divide_by_zero\n";
    return 1;
  }

  // Case B: throwing policy raises exception.
  auto const t_lhs = throwing_t{42};
  auto const t_rhs = throwing_t{0};

  bool caught = false;
  try {
    (void)operations::div(t_lhs, t_rhs);
  } catch (std::runtime_error const &) {
    caught = true;
  }

  if (!caught) {
    std::cerr << "throwing policy did not throw\n";
    return 1;
  }

  std::cout << "error_policy demo passed\n";
  return 0;
}
