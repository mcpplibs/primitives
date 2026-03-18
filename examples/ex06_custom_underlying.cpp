/*
 * Example: ex06_custom_underlying
 *
 * Purpose:
 * Demonstrate custom underlying integration, including rep bridge,
 * rep validation, and extensible common_rep negotiation.
 *
 * Expected results:
 * - UserInteger path succeeds and computes 40 + 2 = 42.
 * - NonNegativeInt path rejects invalid rep and returns domain_error.
 * - TaggedLhs/TaggedRhs path uses custom common_rep_traits to negotiate
 *   TaggedCommonRep, then computes 40 + 2 = 42.
 * - Program prints a success message and exits with code 0.
 */

#include <iostream>
#include <type_traits>

import mcpplibs.primitives;

using namespace mcpplibs::primitives;

// Point 6 / Step 1: Define custom domain value types.
struct UserInteger {
  int value;
};

struct NonNegativeInt {
  int value;
};

struct TaggedLhs {
  int value;

  constexpr explicit operator long long() const noexcept {
    return static_cast<long long>(value);
  }
};

struct TaggedRhs {
  int value;

  constexpr explicit operator long long() const noexcept {
    return static_cast<long long>(value);
  }
};

struct TaggedCommonRep {
  long long value;

  friend constexpr auto operator+(TaggedCommonRep lhs,
                                  TaggedCommonRep rhs) noexcept
      -> TaggedCommonRep {
    return TaggedCommonRep{lhs.value + rhs.value};
  }

  friend constexpr auto operator-(TaggedCommonRep lhs,
                                  TaggedCommonRep rhs) noexcept
      -> TaggedCommonRep {
    return TaggedCommonRep{lhs.value - rhs.value};
  }

  friend constexpr auto operator*(TaggedCommonRep lhs,
                                  TaggedCommonRep rhs) noexcept
      -> TaggedCommonRep {
    return TaggedCommonRep{lhs.value * rhs.value};
  }

  friend constexpr auto operator/(TaggedCommonRep lhs,
                                  TaggedCommonRep rhs) noexcept
      -> TaggedCommonRep {
    return TaggedCommonRep{lhs.value / rhs.value};
  }

  friend constexpr auto operator==(TaggedCommonRep lhs,
                                   TaggedCommonRep rhs) noexcept -> bool {
    return lhs.value == rhs.value;
  }
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

template <> struct mcpplibs::primitives::underlying::traits<TaggedLhs> {
  using value_type = TaggedLhs;
  using rep_type = TaggedLhs;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <> struct mcpplibs::primitives::underlying::traits<TaggedRhs> {
  using value_type = TaggedRhs;
  using rep_type = TaggedRhs;

  static constexpr bool enabled = true;
  static constexpr auto kind = category::integer;

  static constexpr auto to_rep(value_type value) noexcept -> rep_type {
    return value;
  }

  static constexpr auto from_rep(rep_type value) noexcept -> value_type {
    return value;
  }

  static constexpr auto is_valid_rep(rep_type) noexcept -> bool { return true; }
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<TaggedLhs,
                                                           TaggedRhs> {
  using type = TaggedCommonRep;
  static constexpr bool enabled = true;
};

template <>
struct mcpplibs::primitives::underlying::common_rep_traits<TaggedRhs,
                                                           TaggedLhs> {
  using type = TaggedCommonRep;
  static constexpr bool enabled = true;
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

  // Point 6 / Step 4C: Demonstrate extensible common_rep negotiation.
  using lhs_t = primitive<TaggedLhs, policy::value::checked,
                          policy::type::transparent, policy::error::expected>;
  using rhs_t = primitive<TaggedRhs, policy::value::checked,
                          policy::type::transparent, policy::error::expected>;

  using transparent_handler =
      policy::type::handler<policy::type::transparent, operations::Addition,
                            TaggedLhs, TaggedRhs>;
  static_assert(transparent_handler::allowed);
  static_assert(
      std::same_as<typename transparent_handler::common_rep, TaggedCommonRep>);

  auto const lhs = lhs_t{TaggedLhs{40}};
  auto const rhs = rhs_t{TaggedRhs{2}};
  auto const mixed_result = operations::add(lhs, rhs);
  if (!mixed_result.has_value() || mixed_result->value().value != 42) {
    std::cerr << "custom common_rep negotiation failed\n";
    return 1;
  }

  std::cout << "custom underlying demo passed\n";
  return 0;
}
