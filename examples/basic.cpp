#include <iostream>

import mcpplibs.primitive;

int main() {
  std::cout << "=== mcpplibs.primitive traits example ===\n";
  std::cout << std::boolalpha;
  std::cout << "int is std_integer: "
            << mcpplibs::primitive::std_integer<int> << "\n";
  std::cout << "double is std_floating: "
            << mcpplibs::primitive::std_floating<double> << "\n";
  std::cout << "int is underlying_type: "
            << mcpplibs::primitive::underlying_type<int> << "\n";
  return 0;
}
