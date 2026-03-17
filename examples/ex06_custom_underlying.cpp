#include <iostream>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

// Point 6 / Step 1: Define custom domain value types.
struct UserInteger {
  int value;
};

struct NonNegativeInt {
  int value;
};

// Point 6 / Step 2: Register underlying::traits for UserInteger.
// This type has a full int bridge and accepts all reps.
template <> struct mcpplibs::primitives::underlying::traits<UserInteger> {
  using value_type = UserInteger;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return UserInteger{value};
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

// Point 6 / Step 3: Register underlying::traits for NonNegativeInt.
// This one demonstrates custom rep validation (rep must be non-negative).
template <> struct mcpplibs::primitives::underlying::traits<NonNegativeInt> {
  using value_type = NonNegativeInt;
  using rep_type = int;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value.value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return NonNegativeInt{value};
  }

  static constexpr auto is_valid_rep(rep_type value) noexcept -> bool {
    return value >= 0;
  }
};

int main() {
  // Point 6 / Step 4A: Call operations on UserInteger custom underlying.
  using user_t =
      primitive<UserInteger, policy::value::checked, policy::error::expected>;

  auto const ua = user_t{UserInteger{40}};
  auto const ub = user_t{UserInteger{2}};
  auto const user_result = operations::add(ua, ub);
  if (!user_result.has_value() || user_result->value() != 42) {
    std::cerr << "custom UserInteger underlying failed\n";
    return 1;
  }

  // Point 6 / Step 4B: Demonstrate dispatcher rejecting invalid reps.
  // Here -1 fails is_valid_rep and is mapped to domain_error.
  using nonneg_t = primitive<NonNegativeInt, policy::value::checked,
                             policy::error::expected>;
  auto const na = nonneg_t{NonNegativeInt{-1}};
  auto const nb = nonneg_t{NonNegativeInt{2}};
  auto const nonneg_result = operations::add(na, nb);
  if (nonneg_result.has_value() ||
      nonneg_result.error() != policy::error::kind::domain_error) {
    std::cerr << "invalid rep should be rejected by dispatcher\n";
    return 1;
  }

  std::cout << "custom underlying demo passed\n";
  return 0;
}
