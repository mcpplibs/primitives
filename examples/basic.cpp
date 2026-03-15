#include <iostream>
#include <limits>

import mcpplibs.primitives;

int main() {
  using namespace mcpplibs::primitives;
  using namespace mcpplibs::primitives::policy;
  using namespace mcpplibs::primitives::operators;
  using namespace mcpplibs::primitives::types;

  // Operators
  I32<> a{3};
  I32<> b{4};
  auto c = a + b; // primitive<int>
  std::cout << "3 + 4 = " << static_cast<int>(c) << "\n";

  // Error handling
  I32<checked_value> lhs{1};
  I32<checked_value> rhs{std::numeric_limits<std::int32_t>::max()};

  try {
    auto res = lhs + rhs;
  } catch (const std::exception &e) {
    std::cout << "An exception occurred: " << e.what() << "\n";
  }

  I64<checked_value, expected_error> lhs2{1};
  I64<checked_value, expected_error> rhs2{std::numeric_limits<std::int64_t>::max()};

  auto res2 = lhs2 + rhs2;

  // Saturating
  I32<saturating_value> s1{std::numeric_limits<std::int32_t>::max()};
  I32<saturating_value> s2{1};
  auto sat = s1 + s2; // saturating -> stays max
  std::cout << "saturating max + 1 = " << static_cast<std::int32_t>(sat)
            << "\n";

  // Mixed-type addition
  using expected_type = I64<category_compatible_type>;
  expected_type L1{5};
  I32<category_compatible_type> L2{6};
  auto mix = L1 + L2; // common_type -> I64
  std::cout << "5 + 6 = " << mix.value() << "\n";
  std::cout << std::boolalpha;
  std::cout << "Does type of mix is I64<category_compatible_type>: " << std::same_as<decltype(mix), expected_type> << std::endl;

  return 0;
}
