#include <cstdint>
#include <iostream>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 1: Use built-in primitive type aliases and framework operators.
  using namespace mcpplibs::primitives::types;
  using namespace mcpplibs::primitives::operators;

  using value_t = I32<>;

  // Prepare operands.
  auto const a = value_t{40};
  auto const b = value_t{2};

  // Run the four arithmetic operators routed through dispatcher.
  auto const sum = a + b;
  auto const diff = a - b;
  auto const prod = a * b;
  auto const quot = a / b;

  // Validate all dispatches succeeded.
  if (!sum.has_value() || !diff.has_value() || !prod.has_value() ||
      !quot.has_value()) {
    std::cerr << "default arithmetic failed\n";
    return 1;
  }

  std::cout << "sum=" << sum->value() << ", diff=" << diff->value()
            << ", prod=" << prod->value() << ", quot=" << quot->value() << '\n';
  return 0;
}
