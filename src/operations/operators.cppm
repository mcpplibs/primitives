module;
#include <type_traits>
export module mcpplibs.primitives.operations.operators;

import mcpplibs.primitives.underlying;
import mcpplibs.primitives.policy;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.operations.impl;

export namespace mcpplibs::primitives::operators {
template <typename T, policy::policy_type... Policies>
  requires std_numeric<T>
constexpr auto operator~(const primitive<T, Policies...> &p) {
  // Placeholder: unary bitwise NOT not yet forwarded to operations layer.
  // TODO: forward to operations::bit_not when implemented.
  return primitives::primitive<T, Policies...>(~p.value());
}

template <typename T, policy::policy_type... Policies>
  requires std_numeric<T>
constexpr auto operator+(const primitive<T, Policies...> &p) {
  // Placeholder: unary plus not yet forwarded to operations layer.
  // TODO: forward to operations::unary_plus when implemented.
  return primitives::primitive<T, Policies...>(+p.value());
}

template <typename T, policy::policy_type... Policies>
  requires std_numeric<T>
constexpr auto operator-(const primitive<T, Policies...> &p) {
  // Placeholder: unary minus not yet forwarded to operations layer.
  // TODO: forward to operations::unary_neg when implemented.
  return primitives::primitive<T, Policies...>(-p.value());
}

template <typename LHS, typename RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
  requires std_numeric<LHS> && std_numeric<RHS>
constexpr auto operator+(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  return operations::add(lhs, rhs);
}

template <typename LHS, typename RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
  requires std_numeric<LHS> && std_numeric<RHS>
constexpr auto operator-(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  return operations::sub(lhs, rhs);
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator*(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  return operations::mul(lhs, rhs);
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator/(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  // Placeholder: division not yet forwarded to operations layer.
  // TODO: forward to operations::div when implemented (handle policies).
  using result_type = std::common_type_t<LHS, RHS>;
  return primitives::primitive<result_type, PoliciesLHS..., PoliciesRHS...>(
      static_cast<result_type>(lhs.value()) /
      static_cast<result_type>(rhs.value()));
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator%(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  // Placeholder: modulo not yet forwarded to operations layer.
  // TODO: forward to operations::mod when implemented (handle policies).
  using result_type = std::common_type_t<LHS, RHS>;
  return primitives::primitive<result_type, PoliciesLHS..., PoliciesRHS...>(
      static_cast<result_type>(lhs.value()) %
      static_cast<result_type>(rhs.value()));
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator<<(const primitive<LHS, PoliciesLHS...> &lhs,
                          const primitive<RHS, PoliciesRHS...> &rhs) {
  // Placeholder: left shift not yet forwarded to operations layer.
  // TODO: forward to operations::shl when implemented (handle policies).
  using result_type = std::common_type_t<LHS, RHS>;
  return primitives::primitive<result_type, PoliciesLHS..., PoliciesRHS...>(
      static_cast<result_type>(lhs.value())
      << static_cast<result_type>(rhs.value()));
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator>>(const primitive<LHS, PoliciesLHS...> &lhs,
                          const primitive<RHS, PoliciesRHS...> &rhs) {
  // Placeholder: right shift not yet forwarded to operations layer.
  // TODO: forward to operations::shr when implemented (handle policies).
  using result_type = std::common_type_t<LHS, RHS>;
  return primitives::primitive<result_type, PoliciesLHS..., PoliciesRHS...>(
      static_cast<result_type>(lhs.value()) >>
      static_cast<result_type>(rhs.value()));
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator&(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  // Placeholder: bitwise AND not yet forwarded to operations layer.
  // TODO: forward to operations::bit_and when implemented (handle policies).
  using result_type = std::common_type_t<LHS, RHS>;
  return primitives::primitive<result_type, PoliciesLHS..., PoliciesRHS...>(
      static_cast<result_type>(lhs.value()) &
      static_cast<result_type>(rhs.value()));
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator|(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  // Placeholder: bitwise OR not yet forwarded to operations layer.
  // TODO: forward to operations::bit_or when implemented (handle policies).
  using result_type = std::common_type_t<LHS, RHS>;
  return primitives::primitive<result_type, PoliciesLHS..., PoliciesRHS...>(
      static_cast<result_type>(lhs.value()) |
      static_cast<result_type>(rhs.value()));
}

template <std_numeric LHS, std_numeric RHS, policy::policy_type... PoliciesLHS,
          policy::policy_type... PoliciesRHS>
constexpr auto operator^(const primitive<LHS, PoliciesLHS...> &lhs,
                         const primitive<RHS, PoliciesRHS...> &rhs) {
  // Placeholder: bitwise XOR not yet forwarded to operations layer.
  // TODO: forward to operations::bit_xor when implemented (handle policies).
  using result_type = std::common_type_t<LHS, RHS>;
  return primitives::primitive<result_type, PoliciesLHS..., PoliciesRHS...>(
      static_cast<result_type>(lhs.value()) ^
      static_cast<result_type>(rhs.value()));
}
} // namespace mcpplibs::primitives::operators