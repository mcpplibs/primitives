/*
 * Example: ex07_algorithms
 *
 * Purpose:
 * Demonstrate the built-in algorithms helpers for numeric limits metadata,
 * special values, and hashing.
 *
 * Expected results:
 * - limits<> mirrors std::numeric_limits for supported built-in underlying types.
 * - helpers expose lowest/max/epsilon/infinity/quiet_nan values.
 * - hash_value() matches std::hash for the same underlying literal value.
 * - Program prints a success message and exits with code 0.
 */

#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  using namespace mcpplibs::primitives::literals;

  using int_limits = algorithms::limits<decltype(42_i32)>;
  using float_limits = algorithms::limits<double>;

  static_assert(int_limits::enabled);
  static_assert(float_limits::has_infinity);
  static_assert(algorithms::hashable<std::int32_t>);

  constexpr auto min_i32 = algorithms::min_value<std::int32_t>();
  constexpr auto max_i32 = algorithms::max_value<std::int32_t>();
  constexpr auto lowest_f64 = algorithms::lowest_value<double>();
  constexpr auto epsilon_f64 = algorithms::epsilon_value<double>();
  constexpr auto infinity_f64 = algorithms::infinity_value<double>();
  constexpr auto nan_f64 = algorithms::quiet_nan_value<double>();
  auto const hash_literal = algorithms::hash_value(42_i32);
  auto const hash_builtin = algorithms::hash_value(std::int32_t{42});

  if (min_i32 != std::numeric_limits<std::int32_t>::min() ||
      max_i32 != std::numeric_limits<std::int32_t>::max() ||
      lowest_f64 != std::numeric_limits<double>::lowest() ||
      epsilon_f64 != std::numeric_limits<double>::epsilon() ||
      !std::isinf(infinity_f64) || !std::isnan(nan_f64) ||
      hash_literal != hash_builtin ||
      int_limits::digits != std::numeric_limits<std::int32_t>::digits ||
      float_limits::kind != underlying::category::floating) {
    std::cerr << "algorithms demo produced unexpected values\n";
    return 1;
  }

  std::cout << "algorithms demo passed\n";
  return 0;
}
