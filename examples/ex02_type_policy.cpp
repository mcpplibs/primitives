/*
 * Example: ex02_type_policy
 *
 * Purpose:
 * Compare type negotiation and underlying-construction behavior between strict
 * and compatible type policies.
 *
 * Expected results:
 * - strict policy rejects mixed reps (int + long long) at compile-time
 *   negotiation, yielding common_rep=void.
 * - strict policy also rejects cross-underlying construction
 *   (primitive<long long> from int).
 * - compatible policy accepts mixed arithmetic reps and negotiates
 *   common_rep=long long.
 * - compatible policy allows same-category cross-underlying construction
 *   (primitive<long long> from int).
 * - Program prints a success message and exits with code 0.
 */

#include <iostream>
#include <type_traits>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 2A: strict policy rejects mixed reps in dispatcher negotiation.
  using strict_i32 = primitive<int, policy::value::checked,
                               policy::type::strict, policy::error::expected>;
  using strict_i64 = primitive<long long, policy::value::checked,
                               policy::type::strict, policy::error::expected>;

  using strict_meta =
      operations::dispatcher_meta<operations::Addition, strict_i32, strict_i64,
                                  policy::error::kind>;

  static_assert(std::is_same_v<typename strict_meta::common_rep, void>);
  static_assert(
      !policy::type::handler<policy::type::strict, operations::Addition, int,
                             long long>::allowed);

  // Point 2B: strict policy also applies to underlying construction.
  static_assert(std::is_constructible_v<strict_i64, long long>);
  static_assert(!std::is_constructible_v<strict_i64, int>);

  // Point 2C: compatible policy allows mixed arithmetic reps and negotiates a
  // common representation type.
  using compatible_i64 =
      primitive<long long, policy::value::checked, policy::type::compatible,
                policy::error::expected>;

  using compatible_meta = operations::dispatcher_meta<
      operations::Addition,
      primitive<int, policy::value::checked, policy::type::compatible,
                policy::error::expected>,
      compatible_i64>;

  static_assert(
      std::is_same_v<typename compatible_meta::common_rep, long long>);
  static_assert(std::is_constructible_v<compatible_i64, int>);
  static_assert(!std::is_constructible_v<compatible_i64, double>);

  auto const from_underlying = compatible_i64{42};
  auto const rhs = compatible_i64{2LL};
  auto const compatible_sum = operations::add(from_underlying, rhs);
  if (!compatible_sum.has_value() || compatible_sum->value() != 44LL) {
    std::cerr << "compatible underlying construction should participate in add\n";
    return 1;
  }

  std::cout << "type_policy demo passed\n";
  return 0;
}
