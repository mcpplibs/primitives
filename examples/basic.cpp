#include <iostream>

import mcpplibs.primitive;

int main() {
  using namespace mcpplibs::primitive;
  using namespace mcpplibs::primitive::policy;

  std::cout << "=== mcpplibs.primitive traits & policy example ===\n";
  std::cout << std::boolalpha;
  std::cout << "int is std_integer: " << std_integer<int> << "\n";
  std::cout << "double is std_floating: " << std_floating<double> << "\n";
  std::cout << "int is underlying_type: " << underlying_type<int> << "\n";

  std::cout << "default value policy is unchecked_value: "
            << std::is_same_v<policy::default_policies::value,
                              policy::unchecked_value> << "\n";

  std::cout << "checked_value is a policy_type: "
            << policy_type<checked_value> << "\n";
  std::cout << "checked_value category == value: "
            << (policy::traits<checked_value>::kind == policy::category::value)
            << "\n";

  return 0;
}
