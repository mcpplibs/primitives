module;

#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
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
import mcpplibs.primitives.underlying;


namespace mcpplibs::primitives::operations::details {
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

enum class assign_risk : unsigned char {
  overflow,
  underflow,
  domain_error,
  precision_loss,
};

template <typename DestRep, typename SrcRep>
concept statically_castable = requires(SrcRep value) {
  static_cast<std::remove_cvref_t<DestRep>>(value);
};

template <std_integer DestRep, std_integer SrcRep>
constexpr auto numeric_risk(SrcRep value) -> std::optional<assign_risk> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (std::is_signed_v<src_type>) {
    auto const signed_value = static_cast<std::intmax_t>(value);
    if constexpr (std::is_signed_v<dest_type>) {
      if (signed_value <
          static_cast<std::intmax_t>(std::numeric_limits<dest_type>::min())) {
        return assign_risk::underflow;
      }
      if (signed_value >
          static_cast<std::intmax_t>(std::numeric_limits<dest_type>::max())) {
        return assign_risk::overflow;
      }
    } else {
      if (signed_value < 0) {
        return assign_risk::underflow;
      }
      if (static_cast<std::uintmax_t>(signed_value) >
          static_cast<std::uintmax_t>(std::numeric_limits<dest_type>::max())) {
        return assign_risk::overflow;
      }
    }
  } else {
    auto const unsigned_value = static_cast<std::uintmax_t>(value);
    if (unsigned_value >
        static_cast<std::uintmax_t>(std::numeric_limits<dest_type>::max())) {
      return assign_risk::overflow;
    }
  }

  return std::nullopt;
}

template <std_integer DestRep, std_floating SrcRep>
constexpr auto numeric_risk(SrcRep value) -> std::optional<assign_risk> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if (std::isnan(value)) {
    return assign_risk::domain_error;
  }
  if (std::isinf(value)) {
    return value < static_cast<src_type>(0) ? assign_risk::underflow
                                            : assign_risk::overflow;
  }

  auto const normalized = static_cast<long double>(value);
  auto const min_value =
      static_cast<long double>(std::numeric_limits<dest_type>::lowest());
  auto const max_value =
      static_cast<long double>(std::numeric_limits<dest_type>::max());

  if (normalized < min_value) {
    return assign_risk::underflow;
  }
  if (normalized > max_value) {
    return assign_risk::overflow;
  }
  return std::nullopt;
}

template <typename DestRep, typename SrcRep>
constexpr auto numeric_risk(SrcRep value) -> std::optional<assign_risk> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (std_integer<dest_type> && std_integer<src_type>) {
    return numeric_risk<dest_type, src_type>(value);
  } else if constexpr (std_integer<dest_type> && std_floating<src_type>) {
    return numeric_risk<dest_type, src_type>(value);
  } else {
    return std::nullopt;
  }
}

template <typename DestRep, typename SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto saturating_rep_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (std_integer<dest_type> && std_numeric<src_type>) {
    if (auto const kind = numeric_risk<dest_type>(value); kind.has_value()) {
      if (*kind == assign_risk::overflow) {
        return std::numeric_limits<dest_type>::max();
      }
      if (*kind == assign_risk::underflow) {
        return std::numeric_limits<dest_type>::lowest();
      }
      if (*kind == assign_risk::domain_error) {
        return dest_type{};
      }
    }
  }

  return static_cast<dest_type>(value);
}

constexpr auto to_policy_error_kind(assign_risk kind)
    -> policy::error::kind {
  switch (kind) {
  case assign_risk::overflow:
    return policy::error::kind::overflow;
  case assign_risk::underflow:
    return policy::error::kind::underflow;
  case assign_risk::domain_error:
    return policy::error::kind::domain_error;
  default:
    return policy::error::kind::unspecified;
  }
}

template <typename ErrorPayload>
constexpr auto to_error_payload(policy::error::kind kind) -> ErrorPayload {
  if constexpr (std::same_as<ErrorPayload, policy::error::kind>) {
    return kind;
  } else {
    static_cast<void>(kind);
    return ErrorPayload{};
  }
}

} // namespace mcpplibs::primitives::operations::details



export namespace mcpplibs::primitives::operations {

template <operation OpTag, meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
using primitive_dispatch_result_t = std::expected<meta::make_primitive_t<
        typename dispatcher_meta<OpTag, Lhs, Rhs, ErrorPayload>::common_rep,
        typename meta::traits<Lhs>::policies>,
    ErrorPayload>;

template <meta::primitive_type Primitive, underlying_operand Underlying>
using mixed_bridge_primitive_t = meta::make_primitive_t<
    std::remove_cvref_t<Underlying>, typename meta::traits<Primitive>::policies>;

template <operation OpTag, meta::primitive_type Primitive,
          underlying_operand Underlying,
          typename ErrorPayload = policy::error::kind>
using mixed_primitive_dispatch_result_t =
    primitive_dispatch_result_t<OpTag, Primitive,
                                mixed_bridge_primitive_t<Primitive, Underlying>,
                                ErrorPayload>;

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
using three_way_dispatch_result_t = std::expected<
    details::three_way_ordering_t<
        typename dispatcher_meta<ThreeWayCompare, Lhs, Rhs,
                                 ErrorPayload>::common_rep>,
    ErrorPayload>;

template <meta::primitive_type Primitive, underlying_operand Underlying,
          typename ErrorPayload = policy::error::kind>
using mixed_three_way_dispatch_result_t =
    three_way_dispatch_result_t<Primitive,
                                mixed_bridge_primitive_t<Primitive, Underlying>,
                                ErrorPayload>;

template <operation OpTag, underlying_operand Underlying,
          meta::primitive_type Primitive,
          typename ErrorPayload = policy::error::kind>
using flipped_mixed_primitive_dispatch_result_t = primitive_dispatch_result_t<
    OpTag, mixed_bridge_primitive_t<Primitive, Underlying>, Primitive,
    ErrorPayload>;

template <underlying_operand Underlying, meta::primitive_type Primitive,
          typename ErrorPayload = policy::error::kind>
using flipped_mixed_three_way_dispatch_result_t = three_way_dispatch_result_t<
    mixed_bridge_primitive_t<Primitive, Underlying>, Primitive, ErrorPayload>;

template <operation OpTag, meta::primitive_type Operand,
          typename ErrorPayload = policy::error::kind>
using unary_dispatch_result_t =
    primitive_dispatch_result_t<OpTag, Operand, Operand, ErrorPayload>;

template <operation OpTag, meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload>
constexpr auto apply_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload>;

template <operation OpTag, meta::primitive_type Lhs, meta::primitive_type Rhs,
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

template <operation OpTag, meta::primitive_type Operand,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply(Operand const &operand)
    -> unary_dispatch_result_t<OpTag, Operand, ErrorPayload> {
  return apply<OpTag, Operand, Operand, ErrorPayload>(operand, operand);
}

template <operation OpTag, meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload> {
  using bridge_rhs_t = mixed_bridge_primitive_t<Lhs, Rhs>;
  auto const bridge_rhs = bridge_rhs_t{rhs};
  return apply<OpTag, Lhs, bridge_rhs_t, ErrorPayload>(lhs, bridge_rhs);
}

template <operation OpTag, underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<OpTag, Lhs, Rhs,
                                                 ErrorPayload> {
  using bridge_lhs_t = mixed_bridge_primitive_t<Rhs, Lhs>;
  auto const bridge_lhs = bridge_lhs_t{lhs};
  return apply<OpTag, bridge_lhs_t, Rhs, ErrorPayload>(bridge_lhs, rhs);
}

// Unary operations
template <meta::primitive_type Operand, typename ErrorPayload = policy::error::kind>
constexpr auto increment(Operand &operand)
    -> unary_dispatch_result_t<Increment, Operand, ErrorPayload> {
  return apply_assign<Increment, Operand, Operand, ErrorPayload>(operand,
                                                                 operand);
}

template <meta::primitive_type Operand, typename ErrorPayload = policy::error::kind>
constexpr auto decrement(Operand &operand)
    -> unary_dispatch_result_t<Decrement, Operand, ErrorPayload> {
  return apply_assign<Decrement, Operand, Operand, ErrorPayload>(operand,
                                                                 operand);
}

template <meta::primitive_type Operand, typename ErrorPayload = policy::error::kind>
constexpr auto bit_not(Operand const &operand)
    -> unary_dispatch_result_t<BitwiseNot, Operand, ErrorPayload> {
  return apply<BitwiseNot, Operand, ErrorPayload>(operand);
}

template <meta::primitive_type Operand, typename ErrorPayload = policy::error::kind>
constexpr auto unary_plus(Operand const &operand)
    -> unary_dispatch_result_t<UnaryPlus, Operand, ErrorPayload> {
  return apply<UnaryPlus, Operand, ErrorPayload>(operand);
}

template <meta::primitive_type Operand, typename ErrorPayload = policy::error::kind>
constexpr auto unary_minus(Operand const &operand)
    -> unary_dispatch_result_t<UnaryMinus, Operand, ErrorPayload> {
  return apply<UnaryMinus, Operand, ErrorPayload>(operand);
}

// Binary arithmetic operations
template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto add(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Addition, Lhs, Rhs, ErrorPayload> {
  return apply<Addition, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto add(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<Addition, Lhs, Rhs, ErrorPayload> {
  return apply<Addition, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto add(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<Addition, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<Addition, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto sub(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Subtraction, Lhs, Rhs, ErrorPayload> {
  return apply<Subtraction, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto sub(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<Subtraction, Lhs, Rhs, ErrorPayload> {
  return apply<Subtraction, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto sub(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<Subtraction, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<Subtraction, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mul(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Multiplication, Lhs, Rhs, ErrorPayload> {
  return apply<Multiplication, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mul(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<Multiplication, Lhs, Rhs,
                                         ErrorPayload> {
  return apply<Multiplication, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mul(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<Multiplication, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<Multiplication, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto div(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Division, Lhs, Rhs, ErrorPayload> {
  return apply<Division, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mod(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Modulus, Lhs, Rhs, ErrorPayload> {
  return apply<Modulus, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mod(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<Modulus, Lhs, Rhs, ErrorPayload> {
  return apply<Modulus, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mod(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<Modulus, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<Modulus, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

// Binary bitwise operations
template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_left(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<LeftShift, Lhs, Rhs, ErrorPayload> {
  return apply<LeftShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_left(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<LeftShift, Lhs, Rhs, ErrorPayload> {
  return apply<LeftShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_left(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<LeftShift, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<LeftShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_right(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<RightShift, Lhs, Rhs, ErrorPayload> {
  return apply<RightShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_right(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<RightShift, Lhs, Rhs, ErrorPayload> {
  return apply<RightShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_right(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<RightShift, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<RightShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_and(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<BitwiseAnd, Lhs, Rhs, ErrorPayload> {
  return apply<BitwiseAnd, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_and(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<BitwiseAnd, Lhs, Rhs, ErrorPayload> {
  return apply<BitwiseAnd, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_and(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<BitwiseAnd, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<BitwiseAnd, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_or(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<BitwiseOr, Lhs, Rhs, ErrorPayload> {
  return apply<BitwiseOr, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_or(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<BitwiseOr, Lhs, Rhs, ErrorPayload> {
  return apply<BitwiseOr, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_or(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<BitwiseOr, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<BitwiseOr, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_xor(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<BitwiseXor, Lhs, Rhs, ErrorPayload> {
  return apply<BitwiseXor, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_xor(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<BitwiseXor, Lhs, Rhs, ErrorPayload> {
  return apply<BitwiseXor, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_xor(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<BitwiseXor, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<BitwiseXor, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto div(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<Division, Lhs, Rhs, ErrorPayload> {
  return apply<Division, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto div(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<Division, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<Division, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto equal(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Equal, Lhs, Rhs, ErrorPayload> {
  return apply<Equal, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto equal(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<Equal, Lhs, Rhs, ErrorPayload> {
  return apply<Equal, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto equal(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<Equal, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<Equal, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto not_equal(Lhs const &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<NotEqual, Lhs, Rhs, ErrorPayload> {
  return apply<NotEqual, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto not_equal(Lhs const &lhs, Rhs const &rhs)
    -> mixed_primitive_dispatch_result_t<NotEqual, Lhs, Rhs, ErrorPayload> {
  return apply<NotEqual, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto not_equal(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_primitive_dispatch_result_t<NotEqual, Lhs, Rhs,
                                                 ErrorPayload> {
  return apply<NotEqual, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
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

template <meta::primitive_type Lhs, underlying_operand Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto three_way_compare(Lhs const &lhs, Rhs const &rhs)
    -> mixed_three_way_dispatch_result_t<Lhs, Rhs, ErrorPayload> {
  using bridge_rhs_t = mixed_bridge_primitive_t<Lhs, Rhs>;
  auto const bridge_rhs = bridge_rhs_t{rhs};
  return three_way_compare<Lhs, bridge_rhs_t, ErrorPayload>(lhs, bridge_rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto three_way_compare(Lhs const &lhs, Rhs const &rhs)
    -> flipped_mixed_three_way_dispatch_result_t<Lhs, Rhs, ErrorPayload> {
  using bridge_lhs_t = mixed_bridge_primitive_t<Rhs, Lhs>;
  auto const bridge_lhs = bridge_lhs_t{lhs};
  return three_way_compare<bridge_lhs_t, Rhs, ErrorPayload>(bridge_lhs, rhs);
}

template <operation OpTag, meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto apply_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload> {
  using lhs_traits = meta::traits<Lhs>;
  using lhs_value_type = lhs_traits::value_type;
  using lhs_value_policy = lhs_traits::value_policy;
  using lhs_rep = underlying::traits<lhs_value_type>::rep_type;
  using assign_meta = dispatcher_meta<OpTag, Lhs, Rhs, ErrorPayload>;
  using common_value_type = assign_meta::common_rep;
  using common_rep = underlying::traits<common_value_type>::rep_type;

  auto out = apply<OpTag, Lhs, Rhs, ErrorPayload>(lhs, rhs);
  if (!out.has_value()) {
    return std::unexpected(out.error());
  }

  auto const assigned_common = out->load();
  auto const assigned_common_rep =
      underlying::traits<common_value_type>::to_rep(assigned_common);
  if constexpr (std::same_as<lhs_value_policy, policy::value::checked> &&
                std_integer<lhs_rep> && std_numeric<common_rep>) {
    if (auto const kind =
            details::numeric_risk<lhs_rep>(assigned_common_rep);
        kind.has_value()) {
      return std::unexpected(
          details::to_error_payload<ErrorPayload>(
              details::to_policy_error_kind(*kind)));
    }
  }

  auto const assigned_rep =
      details::saturating_rep_cast<lhs_rep>(assigned_common_rep);
  lhs.store(underlying::traits<lhs_value_type>::from_rep(assigned_rep));
  return out;
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto add_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Addition, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Addition, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto sub_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Subtraction, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Subtraction, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mul_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Multiplication, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Multiplication, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto div_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Division, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Division, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto mod_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<Modulus, Lhs, Rhs, ErrorPayload> {
  return apply_assign<Modulus, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_left_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<LeftShift, Lhs, Rhs, ErrorPayload> {
  return apply_assign<LeftShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto shift_right_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<RightShift, Lhs, Rhs, ErrorPayload> {
  return apply_assign<RightShift, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_and_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<BitwiseAnd, Lhs, Rhs, ErrorPayload> {
  return apply_assign<BitwiseAnd, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_or_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<BitwiseOr, Lhs, Rhs, ErrorPayload> {
  return apply_assign<BitwiseOr, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

template <meta::primitive_type Lhs, meta::primitive_type Rhs,
          typename ErrorPayload = policy::error::kind>
constexpr auto bit_xor_assign(Lhs &lhs, Rhs const &rhs)
    -> primitive_dispatch_result_t<BitwiseXor, Lhs, Rhs, ErrorPayload> {
  return apply_assign<BitwiseXor, Lhs, Rhs, ErrorPayload>(lhs, rhs);
}

} // namespace mcpplibs::primitives::operations

export namespace mcpplibs::primitives::operators {

// Unary operators
template <meta::primitive_type Operand>
constexpr auto operator++(Operand &operand)
    -> operations::unary_dispatch_result_t<operations::Increment, Operand> {
  return operations::increment(operand);
}

template <meta::primitive_type Operand>
constexpr auto operator++(Operand &operand, int)
    -> operations::unary_dispatch_result_t<operations::Increment, Operand> {
  auto const before = operand;
  auto const stepped = operations::increment(operand);
  if (!stepped.has_value()) {
    return std::unexpected(stepped.error());
  }
  return before;
}

template <meta::primitive_type Operand>
constexpr auto operator--(Operand &operand)
    -> operations::unary_dispatch_result_t<operations::Decrement, Operand> {
  return operations::decrement(operand);
}

template <meta::primitive_type Operand>
constexpr auto operator--(Operand &operand, int)
    -> operations::unary_dispatch_result_t<operations::Decrement, Operand> {
  auto const before = operand;
  auto const stepped = operations::decrement(operand);
  if (!stepped.has_value()) {
    return std::unexpected(stepped.error());
  }
  return before;
}

template <meta::primitive_type Operand>
constexpr auto operator+(Operand const &operand)
    -> operations::unary_dispatch_result_t<operations::UnaryPlus, Operand> {
  return operations::unary_plus(operand);
}

template <meta::primitive_type Operand>
constexpr auto operator-(Operand const &operand)
    -> operations::unary_dispatch_result_t<operations::UnaryMinus, Operand> {
  return operations::unary_minus(operand);
}

template <meta::primitive_type Operand>
constexpr auto operator~(Operand const &operand)
    -> operations::unary_dispatch_result_t<operations::BitwiseNot, Operand> {
  return operations::bit_not(operand);
}

// Binary arithmetic operators
template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator+(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Addition, Lhs, Rhs> {
  return operations::add(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator+(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::Addition, Lhs,
                                                     Rhs> {
  return operations::add(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator+(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::Addition, Lhs, Rhs> {
  return operations::add(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator-(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Subtraction, Lhs,
                                               Rhs> {
  return operations::sub(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator-(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::Subtraction,
                                                     Lhs, Rhs> {
  return operations::sub(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator-(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::Subtraction, Lhs, Rhs> {
  return operations::sub(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator*(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Multiplication, Lhs,
                                               Rhs> {
  return operations::mul(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator*(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::Multiplication,
                                                     Lhs, Rhs> {
  return operations::mul(lhs, rhs);
}

template <underlying_operand Lhs,
          meta::primitive_type Rhs>
constexpr auto operator*(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::Multiplication, Lhs, Rhs> {
  return operations::mul(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator/(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Division, Lhs, Rhs> {
  return operations::div(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator/(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::Division, Lhs,
                                                     Rhs> {
  return operations::div(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator/(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::Division, Lhs, Rhs> {
  return operations::div(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator%(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Modulus, Lhs, Rhs> {
  return operations::mod(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator%(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::Modulus, Lhs,
                                                     Rhs> {
  return operations::mod(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator%(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::Modulus, Lhs, Rhs> {
  return operations::mod(lhs, rhs);
}

// Binary bitwise operators
template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator<<(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::LeftShift, Lhs, Rhs> {
  return operations::shift_left(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator<<(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::LeftShift, Lhs,
                                                     Rhs> {
  return operations::shift_left(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator<<(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::LeftShift, Lhs, Rhs> {
  return operations::shift_left(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::RightShift, Lhs, Rhs> {
  return operations::shift_right(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::RightShift, Lhs,
                                                     Rhs> {
  return operations::shift_right(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator>>(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::RightShift, Lhs, Rhs> {
  return operations::shift_right(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator&(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::BitwiseAnd, Lhs, Rhs> {
  return operations::bit_and(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator&(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::BitwiseAnd, Lhs,
                                                     Rhs> {
  return operations::bit_and(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator&(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::BitwiseAnd, Lhs, Rhs> {
  return operations::bit_and(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator|(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::BitwiseOr, Lhs, Rhs> {
  return operations::bit_or(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator|(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::BitwiseOr, Lhs,
                                                     Rhs> {
  return operations::bit_or(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator|(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::BitwiseOr, Lhs, Rhs> {
  return operations::bit_or(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator^(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::BitwiseXor, Lhs, Rhs> {
  return operations::bit_xor(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator^(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::BitwiseXor, Lhs,
                                                     Rhs> {
  return operations::bit_xor(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator^(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::BitwiseXor, Lhs, Rhs> {
  return operations::bit_xor(lhs, rhs);
}

// Binary comparison operators

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator==(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Equal, Lhs, Rhs> {
  return operations::equal(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator==(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::Equal, Lhs,
                                                     Rhs> {
  return operations::equal(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator==(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<operations::Equal,
                                                             Lhs, Rhs> {
  return operations::equal(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator!=(Lhs const &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::NotEqual, Lhs, Rhs> {
  return operations::not_equal(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator!=(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_primitive_dispatch_result_t<operations::NotEqual, Lhs,
                                                     Rhs> {
  return operations::not_equal(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator!=(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_primitive_dispatch_result_t<
        operations::NotEqual, Lhs, Rhs> {
  return operations::not_equal(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator<=>(Lhs const &lhs, Rhs const &rhs)
    -> operations::three_way_dispatch_result_t<Lhs, Rhs> {
  return operations::three_way_compare(lhs, rhs);
}

template <meta::primitive_type Lhs, underlying_operand Rhs>
constexpr auto operator<=>(Lhs const &lhs, Rhs const &rhs)
    -> operations::mixed_three_way_dispatch_result_t<Lhs, Rhs> {
  return operations::three_way_compare(lhs, rhs);
}

template <underlying_operand Lhs, meta::primitive_type Rhs>
constexpr auto operator<=>(Lhs const &lhs, Rhs const &rhs)
    -> operations::flipped_mixed_three_way_dispatch_result_t<Lhs, Rhs> {
  return operations::three_way_compare(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator+=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Addition, Lhs, Rhs> {
  return operations::add_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator-=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Subtraction, Lhs,
                                               Rhs> {
  return operations::sub_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator*=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Multiplication, Lhs,
                                               Rhs> {
  return operations::mul_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator/=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Division, Lhs, Rhs> {
  return operations::div_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator%=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::Modulus, Lhs, Rhs> {
  return operations::mod_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator<<=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::LeftShift, Lhs, Rhs> {
  return operations::shift_left_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator>>=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::RightShift, Lhs, Rhs> {
  return operations::shift_right_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator&=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::BitwiseAnd, Lhs, Rhs> {
  return operations::bit_and_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator|=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::BitwiseOr, Lhs, Rhs> {
  return operations::bit_or_assign(lhs, rhs);
}

template <meta::primitive_type Lhs,
          meta::primitive_type Rhs>
constexpr auto operator^=(Lhs &lhs, Rhs const &rhs)
    -> operations::primitive_dispatch_result_t<operations::BitwiseXor, Lhs, Rhs> {
  return operations::bit_xor_assign(lhs, rhs);
}

} // namespace mcpplibs::primitives::operators

