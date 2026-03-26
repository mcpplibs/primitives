module;

#include <type_traits>

export module mcpplibs.primitives.conversion.primitive;

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.conversion.mixing;
import mcpplibs.primitives.primitive;

export namespace mcpplibs::primitives::conversion {

template <meta::primitive_type Dest, meta::primitive_type Src>
constexpr auto unchecked_cast(Src const &value) noexcept
    -> std::remove_cvref_t<Dest> {
  return conversion::unchecked_cast<std::remove_cvref_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, meta::primitive_type Src>
constexpr auto checked_cast(Src const &value)
    -> cast_result<std::remove_cvref_t<Dest>> {
  return conversion::checked_cast<std::remove_cvref_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, meta::primitive_type Src>
constexpr auto saturating_cast(Src const &value) noexcept
    -> std::remove_cvref_t<Dest> {
  return conversion::saturating_cast<std::remove_cvref_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, meta::primitive_type Src>
constexpr auto truncating_cast(Src const &value) noexcept
    -> std::remove_cvref_t<Dest> {
  return conversion::truncating_cast<std::remove_cvref_t<Dest>>(value.load());
}

template <meta::primitive_type Dest, meta::primitive_type Src>
constexpr auto exact_cast(Src const &value)
    -> cast_result<std::remove_cvref_t<Dest>> {
  return conversion::exact_cast<std::remove_cvref_t<Dest>>(value.load());
}

} // namespace mcpplibs::primitives::conversion
