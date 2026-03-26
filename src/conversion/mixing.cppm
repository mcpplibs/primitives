module;

#include <expected>
#include <type_traits>

export module mcpplibs.primitives.conversion.mixing;

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.conversion.underlying;
import mcpplibs.primitives.primitive;
import mcpplibs.primitives.underlying;

namespace mcpplibs::primitives::conversion::details {

template <meta::primitive_type Primitive>
using primitive_value_t =
  meta::traits<std::remove_cvref_t<Primitive>>::value_type;

template <meta::primitive_type Dest, underlying_type Src, typename Caster>
constexpr auto cast_to_primitive_value(Src value, Caster caster) noexcept
    -> std::remove_cvref_t<Dest> {
  using dest_type = std::remove_cvref_t<Dest>;
  using dest_value_type = primitive_value_t<dest_type>;

  return dest_type{caster.template operator()<dest_value_type>(value)};
}

template <meta::primitive_type Dest, underlying_type Src, typename Caster>
constexpr auto cast_to_primitive_result(Src value, Caster caster)
    -> cast_result<std::remove_cvref_t<Dest>> {
  using dest_type = std::remove_cvref_t<Dest>;
  using dest_value_type = primitive_value_t<dest_type>;

  auto const converted = caster.template operator()<dest_value_type>(value);
  if (!converted.has_value()) {
    return std::unexpected(converted.error());
  }

  return dest_type{*converted};
}

} // namespace mcpplibs::primitives::conversion::details

export namespace mcpplibs::primitives::conversion {

template <meta::primitive_type Dest, underlying_type Src>
constexpr auto unchecked_cast(Src value) noexcept -> std::remove_cvref_t<Dest> {
  return details::cast_to_primitive_value<Dest>(
      value, []<underlying_type DestValue, underlying_type SrcValue>(
                 SrcValue source) {
        return conversion::unchecked_cast<DestValue>(source);
      });
}

template <underlying_type Dest, meta::primitive_type Src>
constexpr auto unchecked_cast(Src const &value) noexcept -> std::remove_cv_t<Dest> {
  return conversion::unchecked_cast<std::remove_cv_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, underlying_type Src>
constexpr auto checked_cast(Src value)
    -> cast_result<std::remove_cvref_t<Dest>> {
  return details::cast_to_primitive_result<Dest>(
      value, []<underlying_type DestValue, underlying_type SrcValue>(
                 SrcValue source) {
        return conversion::checked_cast<DestValue>(source);
      });
}

template <underlying_type Dest, meta::primitive_type Src>
constexpr auto checked_cast(Src const &value)
    -> cast_result<std::remove_cv_t<Dest>> {
  return conversion::checked_cast<std::remove_cv_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, underlying_type Src>
constexpr auto saturating_cast(Src value) noexcept
    -> std::remove_cvref_t<Dest> {
  return details::cast_to_primitive_value<Dest>(
      value, []<underlying_type DestValue, underlying_type SrcValue>(
                 SrcValue source) {
        return conversion::saturating_cast<DestValue>(source);
      });
}

template <underlying_type Dest, meta::primitive_type Src>
constexpr auto saturating_cast(Src const &value) noexcept
    -> std::remove_cv_t<Dest> {
  return conversion::saturating_cast<std::remove_cv_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, underlying_type Src>
constexpr auto truncating_cast(Src value) noexcept
    -> std::remove_cvref_t<Dest> {
  return details::cast_to_primitive_value<Dest>(
      value, []<underlying_type DestValue, underlying_type SrcValue>(
                 SrcValue source) {
        return conversion::truncating_cast<DestValue>(source);
      });
}

template <underlying_type Dest, meta::primitive_type Src>
constexpr auto truncating_cast(Src const &value) noexcept
    -> std::remove_cv_t<Dest> {
  return conversion::truncating_cast<std::remove_cv_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, underlying_type Src>
constexpr auto exact_cast(Src value)
    -> cast_result<std::remove_cvref_t<Dest>> {
  return details::cast_to_primitive_result<Dest>(
      value, []<underlying_type DestValue, underlying_type SrcValue>(
                 SrcValue source) {
        return conversion::exact_cast<DestValue>(source);
      });
}

template <underlying_type Dest, meta::primitive_type Src>
constexpr auto exact_cast(Src const &value)
    -> cast_result<std::remove_cv_t<Dest>> {
  return conversion::exact_cast<std::remove_cv_t<Dest>>(value.load());
}

} // namespace mcpplibs::primitives::conversion
