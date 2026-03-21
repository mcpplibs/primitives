module;

#include <compare>
#include <expected>
#include <type_traits>
#include <utility>

export module mcpplibs.primitives.operations.operators;

import mcpplibs.primitives.operations.traits;
import mcpplibs.primitives.operations.dispatcher;
import mcpplibs.primitives.operations.impl;
import mcpplibs.primitives.primitive.impl;
import mcpplibs.primitives.primitive.traits;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.underlying.traits;

export namespace mcpplibs::primitives::operations {

namespace details {
template <typename CommonRep, typename = void> struct three_way_ordering {
  using type = std::strong_ordering;
};

template <typename CommonRep>
struct three_way_ordering<
    CommonRep,
    std::void_t<decltype(std::declval<CommonRep const &>() <=>
                         std::declval<CommonRep const &>())>> {
  using type = std::remove_cvref_t<decltype(std::declval<CommonRep const &>() <=>
                                            std::declval<CommonRep const &>())>;
};

template <typename CommonRep>
using three_way_ordering_t = three_way_ordering<CommonRep>::type;

template <typename Ordering, typename CommonRep>
constexpr auto decode_three_way_code(CommonRep const &code) -> Ordering {
  if (code == static_cast<CommonRep>(0)) {
    return Ordering::less;
  }
  if (code == static_cast<CommonRep>(2)) {
    return Ordering::greater;
  }

  if constexpr (std::is_same_v<Ordering, std::partial_ordering>) {
    if (code == static_cast<CommonRep>(3)) {
      return std::partial_ordering::unordered;
    }
    return std::partial_ordering::equivalent;
  }

  if constexpr (std::is_same_v<Ordering, std::strong_ordering>) {
    return std::strong_ordering::equal;
  }

  return Ordering::equivalent;
}

} // namespace details

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
using primitive_dispatch_result_t = std::expected<meta::make_primitive_t<
        typename dispatcher_meta<OpTag, Lhs, Rhs, ErrorPayload>::common_rep,
        typename meta::traits<Lhs>::policies>,
    ErrorPayload>;

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
using three_way_dispatch_result_t = std::expected<
    details::three_way_ordering_t<
        typename dispatcher_meta<ThreeWayCompare, Lhs, Rhs,
                                 ErrorPayload>::common_rep>,
    ErrorPayload>;

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload> {
  using result_primitive =
      primitive_dispatch_result_t<OpTag, Lhs, Rhs,
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

template <primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto three_way_compare(Lhs const &lhs, Rhs const &rhs)
    -> three_way_dispatch_result_t<Lhs, Rhs, ErrorPayload> {
  using common_rep =
      dispatcher_meta<ThreeWayCompare, Lhs, Rhs,
                               ErrorPayload>::common_rep;
  using ordering =
      three_way_dispatch_result_t<Lhs, Rhs, ErrorPayload>::value_type;

  auto const raw = dispatch<ThreeWayCompare, Lhs, Rhs, ErrorPayload>(lhs, rhs);
  if (!raw.has_value()) {
    return std::unexpected(raw.error());
  }

  return details::decode_three_way_code<ordering, common_rep>(*raw);
}

template <operation OpTag, primitive_instance Lhs, primitive_instance Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload> {
  using lhs_traits = meta::traits<Lhs>;
  using lhs_value_type = lhs_traits::value_type;
  using lhs_value_policy = lhs_traits::value_policy;
  using lhs_rep = underlying::traits<lhs_value_type>::rep_type;
  using out_primitive = primitive_dispatch_result_t<OpTag, Lhs, Rhs,
                                           ErrorPayload>::value_type;
  using common_rep = meta::traits<out_primitive>::value_type;

  auto out = apply<OpTag, Lhs, Rhs, ErrorPayload>(lhs, rhs);
  if (!out.has_value()) {
    return std::unexpected(out.error());
  }

  auto const assigned_common = out->load();
  if constexpr (std::same_as<lhs_value_policy, policy::value::checked> &&
                std::integral<lhs_rep> && std::integral<common_rep>) {
    if (auto const kind =
            policy::details::narrow_integral_error<lhs_rep>(assigned_common);
        kind.has_value()) {
      return std::unexpected(
          policy::details::to_error_payload<ErrorPayload>(*kind));
    }
  }

  auto const assigned_rep = static_cast<lhs_rep>(assigned_common);
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
constexpr auto operator<=>(Lhs const &lhs, Rhs const &rhs)
    -> operations::three_way_dispatch_result_t<Lhs, Rhs> {
  return operations::three_way_compare(lhs, rhs);
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
