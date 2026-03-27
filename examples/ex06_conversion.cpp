/*
 * Example: ex06_conversion
 *
 * Purpose:
 * Demonstrate the conversion helpers for checked, saturating, truncating, and
 * exact casts across both underlying values and primitives.
 *
 * Expected results:
 * - checked_cast succeeds for in-range primitive-to-underlying conversion.
 * - checked_cast reports underflow for negative-to-unsigned conversion.
 * - saturating_cast clamps out-of-range input.
 * - truncating_cast drops the fractional part when targeting an integral
 *   primitive.
 * - exact_cast preserves exact values and reports precision loss otherwise.
 * - Program prints a success message and exits with code 0.
 */

#include <cstdint>
#include <iostream>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  using namespace mcpplibs::primitives::literals;

  using expected_i32 =
      types::I32<policy::value::checked, policy::error::expected>;
  using expected_f32 =
      types::F32<policy::value::checked, policy::error::expected>;

  constexpr auto source =
      with<policy::value::checked, policy::error::expected>(255_i32);

  constexpr auto checked_u16 = conversion::checked_cast<std::uint16_t>(source);
  constexpr auto checked_bad = conversion::checked_cast<std::uint16_t>(-1_i32);
  constexpr auto saturated_u16 =
      conversion::saturating_cast<std::uint16_t>(100000_i32);
  auto const truncated_i32 =
      conversion::truncating_cast<expected_i32>(12.75_f64);
  auto const exact_f32 = conversion::exact_cast<expected_f32>(16777216_i32);
  auto const exact_bad = conversion::exact_cast<expected_f32>(16777217_i32);
  constexpr auto risk = conversion::numeric_risk<std::uint16_t>(-1_i32);

  if (!checked_u16.has_value() || checked_bad.has_value() ||
      !exact_f32.has_value() || exact_bad.has_value() || !risk.has_value()) {
    std::cerr << "conversion demo expected success path missing\n";
    return 1;
  }

  if (*checked_u16 != static_cast<std::uint16_t>(255) ||
      checked_bad.error() != conversion::risk::kind::underflow ||
      saturated_u16 != static_cast<std::uint16_t>(65535) ||
      truncated_i32.load() != 12 ||
      exact_f32->load() != 16777216.0F ||
      exact_bad.error() != conversion::risk::kind::precision_loss ||
      *risk != conversion::risk::kind::underflow) {
    std::cerr << "conversion demo produced unexpected values\n";
    return 1;
  }

  std::cout << "conversion demo passed\n";
  return 0;
}
