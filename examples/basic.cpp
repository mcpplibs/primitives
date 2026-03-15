#include <iostream>

import mcpplibs.primitives;

int main() {
  using namespace mcpplibs::primitives;
  using namespace mcpplibs::primitives::policy;

  std::cout << "=== mcpplibs.primitives traits & policy example ===\n";
  std::cout << std::boolalpha;
  std::cout << "int is std_integer: " << std_integer<int> << "\n";
  std::cout << "double is std_floating: " << std_floating<double> << "\n";
  std::cout << "int is underlying_type: " << underlying_type<int> << "\n";

  std::cout
      << "default value policy is unchecked_value: "
      << std::is_same_v<default_value, checked_value> << "\n";

  std::cout << "checked_value is a policy_type: "
            << policy_type<checked_value> << "\n";
  std::cout << "checked_value category == value: "
            << (policy::traits<checked_value>::kind == category::value)
            << "\n";

  return 0;
}
