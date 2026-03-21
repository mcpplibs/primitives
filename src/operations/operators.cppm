module;

#include <expected>

export module mcpplibs.primitives.operations.operators;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.dispatcher;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.primitive.impl;
import mcpplibs.primitives.primitive.traits;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.underlying.traits;

export namespace mcpplibs::primitives::operations {

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
using primitive_dispatch_result_t = std::expected<
    typename mcpplibs::primitives::traits::make_primitive_t<
        typename dispatcher_meta<OpTag, Lhs, Rhs, ErrorPayload>::common_rep,
        typename mcpplibs::primitives::traits::primitive_traits<Lhs>::policies>,
    ErrorPayload>;

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload> {
  using result_primitive =
      typename primitive_dispatch_result_t<OpTag, Lhs, Rhs,
                                           ErrorPayload>::value_type;

  auto const raw = dispatch<OpTag, Lhs, Rhs, ErrorPayload>(lhs, rhs);
  if (!raw.has_value()) {
    return std::unexpected(raw.error());
  }

  return result_primitive{*raw};
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto add(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Addition, Lhs, Rhs, ErrorPayload> {
  return apply<Addition, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto sub(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Subtraction, Lhs, Rhs, ErrorPayload> {
  return apply<Subtraction, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mul(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Multiplication, Lhs, Rhs, ErrorPayload> {
  return apply<Multiplication, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto div(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Division, Lhs, Rhs, ErrorPayload> {
  return apply<Division, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto equal(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Equal, Lhs, Rhs, ErrorPayload> {
  return apply<Equal, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto not_equal(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<NotEqual, Lhs, Rhs, ErrorPayload> {
  return apply<NotEqual, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload> {
  using lhs_value_type =
      typename mcpplibs::primitives::traits::primitive_traits<Lhs>::value_type;
  using lhs_rep = typename underlying::traits<lhs_value_type>::rep_type;

  auto out = apply<OpTag, Lhs, Rhs, ErrorPayload>(lhs, rhs);
  if (!out.has_value()) {
    return std::unexpected(out.error());
  }

  auto const assigned_rep = static_cast<lhs_rep>(out->load());
  lhs.store(underlying::traits<lhs_value_type>::from_rep(assigned_rep));
  return out;
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto add_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Addition, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Addition, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto sub_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Subtraction, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Subtraction, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mul_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Multiplication, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Multiplication, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto div_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Division, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Division, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

} // namespace mcpplibs::primitives::operations

export namespace mcpplibs::primitives::operators {

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator+(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Addition, Lhs, Rhs> {
  return operations::add(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator-(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Subtraction, Lhs,
                                               Rhs> {
  return operations::sub(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator*(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Multiplication, Lhs,
                                               Rhs> {
  return operations::mul(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator/(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Division, Lhs, Rhs> {
  return operations::div(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator==(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Equal, Lhs, Rhs> {
  return operations::equal(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator!=(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::NotEqual, Lhs, Rhs> {
  return operations::not_equal(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator+=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Addition, Lhs, Rhs> {
  return operations::add_assign(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator-=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Subtraction, Lhs,
                                               Rhs> {
  return operations::sub_assign(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator*=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Multiplication, Lhs,
                                               Rhs> {
  return operations::mul_assign(lhs, rhs);
}

template <operations::primitive_instance Lhs,
          operations::primitive_instance Rhs>
constexpr auto operator/=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Division, Lhs, Rhs> {
  return operations::div_assign(lhs, rhs);
}

} // namespace mcpplibs::primitives::operators
