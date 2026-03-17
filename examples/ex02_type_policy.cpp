#include <iostream>
#include <type_traits>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

int main() {
  // Point 2: Compare different type_policy behaviors.
  // Case A: strict policy rejects mixed underlying reps at compile time.
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

  // Case B: compatible policy allows mixed arithmetic reps and negotiates a
  // common type.
  using compatible_meta = operations::dispatcher_meta<
      operations::Addition,
      primitive<int, policy::value::checked, policy::type::compatible,
                policy::error::expected>,
      primitive<long long, policy::value::checked, policy::type::compatible,
                policy::error::expected>,
      policy::error::kind>;

  static_assert(
      std::is_same_v<typename compatible_meta::common_rep, long long>);

  std::cout << "type_policy demo passed\n";
  return 0;
}
