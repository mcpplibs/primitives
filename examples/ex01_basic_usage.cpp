/*
 * Example: ex01_basic_usage
 *
 * Purpose:
 * Combine checked literals, primitive factory helpers, and built-in framework
 * operators in one compact end-to-end example.
 *
 * Expected results:
 * - `with(...)` builds primitives directly from checked underlying literals.
 * - Arithmetic, unary, bitwise, comparison, mixed, and compound operators all
 *   dispatch successfully.
 * - The printed values match: sum=42, neg=-2, mod=2, shl=40, band=8, final=43.
 * - Program exits with code 0; otherwise prints an error and exits non-zero.
 */

#include <iostream>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  using namespace mcpplibs::primitives::literals;
  using namespace mcpplibs::primitives::operators;

  constexpr auto base =
      with<policy::value::checked, policy::error::expected>(40_i32);
  constexpr auto step =
      with<policy::value::checked, policy::error::expected>(2_i32);
  constexpr auto mask =
      with<policy::value::checked, policy::error::expected>(10_i32);

  constexpr auto sum = 2_i32 + base;
  if constexpr (!sum.has_value()) {
    std::cerr << "basic usage dispatch failed\n";
    return 1;
  }

  constexpr auto neg = -step;
  constexpr auto mod = *sum % 5_i32;
  constexpr auto shl = mask << step;
  constexpr auto band = base & 12_i32;
  constexpr auto const eq = *sum == 42_i32;
  constexpr auto const cmp = *sum <=> 41_i32;

  auto counter =
      with<policy::value::checked, policy::error::expected>(1_i32);
  auto const pre_inc = ++counter;
  constexpr auto bonus =
      with<policy::value::checked, policy::error::expected>(41_i32);
  auto const add_assign = (counter += bonus);

  if (!pre_inc.has_value() || !add_assign.has_value()) {
    std::cerr << "basic usage dispatch failed\n";
    return 1;
  }

  if (pre_inc->value() != 2 ||
      add_assign->value() != 43 || counter.load() != 43) {
    std::cerr << "basic usage produced unexpected values\n";
    return 1;
  }

  std::cout << "sum=" << sum->value() << ", neg=" << neg->value()
            << ", mod=" << mod->value() << ", shl=" << shl->value()
            << ", band=" << band->value() << ", final=" << counter.load()
            << '\n';

  std::cout << "basic_usage demo passed\n";
  return 0;
}
