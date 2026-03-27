module;
#include <concepts>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <type_traits>

export module mcpplibs.primitives.underlying.impl;

import mcpplibs.primitives.underlying.traits;

export
template <mcpplibs::primitives::std_underlying_type T>
struct mcpplibs::primitives::underlying::traits<T> {
  using value_type = std::remove_cv_t<T>;
  using rep_type = value_type;

  static constexpr bool enabled = true;

  static constexpr category kind = [] {
    if constexpr (std_bool<value_type>) {
      return category::boolean;
    } else if constexpr (std_char<value_type>) {
      return category::character;
    } else if constexpr (std_integer<value_type>) {
      return category::integer;
    } else {
      return category::floating;
    }
  }();

  static constexpr rep_type to_rep(value_type value) noexcept { return value; }

  static constexpr value_type from_rep(rep_type value) noexcept {
    return value;
  }

  static constexpr bool is_valid_rep(rep_type) noexcept { return true; }
};

namespace mcpplibs::primitives::underlying::details {

template <std::integral T>
consteval auto cast_integer_literal(unsigned long long value) -> T {
  if (value > static_cast<unsigned long long>(std::numeric_limits<T>::max())) {
    throw std::out_of_range{"integer literal is out of range for target underlying type"};
  }

  return static_cast<T>(value);
}

template <std::floating_point T>
consteval auto cast_floating_literal(long double value) -> T {
  if (!(value < 0 || value >= 0)) {
    throw std::out_of_range{"floating literal must be finite"};
  }

  auto const lowest = static_cast<long double>(std::numeric_limits<T>::lowest());
  if (auto const max = static_cast<long double>(std::numeric_limits<T>::max());
      value < lowest || value > max) {
    throw std::out_of_range{"floating literal is out of range for target underlying type"};
  }

  return static_cast<T>(value);
}

} // namespace mcpplibs::primitives::underlying::details

export namespace mcpplibs::primitives::literals {

consteval auto operator""_uchar(const char value) -> unsigned char {
  return static_cast<unsigned char>(value);
}

consteval auto operator""_char8(const char8_t value) -> char8_t { return value; }

consteval auto operator""_char16(const char16_t value) -> char16_t { return value; }

consteval auto operator""_char32(const char32_t value) -> char32_t { return value; }

consteval auto operator""_wchar(const wchar_t value) -> wchar_t { return value; }

consteval auto operator""_u8(const unsigned long long value) -> std::uint8_t {
  return underlying::details::cast_integer_literal<std::uint8_t>(value);
}

consteval auto operator""_u16(const unsigned long long value) -> std::uint16_t {
  return underlying::details::cast_integer_literal<std::uint16_t>(value);
}

consteval auto operator""_u32(const unsigned long long value) -> std::uint32_t {
  return underlying::details::cast_integer_literal<std::uint32_t>(value);
}

consteval auto operator""_u64(const unsigned long long value) -> std::uint64_t {
  return underlying::details::cast_integer_literal<std::uint64_t>(value);
}

consteval auto operator""_i8(const unsigned long long value) -> std::int8_t {
  return underlying::details::cast_integer_literal<std::int8_t>(value);
}

consteval auto operator""_i16(const unsigned long long value) -> std::int16_t {
  return underlying::details::cast_integer_literal<std::int16_t>(value);
}

consteval auto operator""_i32(const unsigned long long value) -> std::int32_t {
  return underlying::details::cast_integer_literal<std::int32_t>(value);
}

consteval auto operator""_i64(const unsigned long long value) -> std::int64_t {
  return underlying::details::cast_integer_literal<std::int64_t>(value);
}

consteval auto operator""_f32(const unsigned long long value) -> float {
  return underlying::details::cast_floating_literal<float>(
      static_cast<long double>(value));
}

consteval auto operator""_f32(const long double value) -> float {
  return underlying::details::cast_floating_literal<float>(value);
}

consteval auto operator""_f64(const unsigned long long value) -> double {
  return underlying::details::cast_floating_literal<double>(
      static_cast<long double>(value));
}

consteval auto operator""_f64(const long double value) -> double {
  return underlying::details::cast_floating_literal<double>(value);
}

consteval auto operator""_f80(const unsigned long long value) -> long double {
  return underlying::details::cast_floating_literal<long double>(
      static_cast<long double>(value));
}

consteval auto operator""_f80(const long double value) -> long double {
  return underlying::details::cast_floating_literal<long double>(value);
}

} // namespace mcpplibs::primitives::literals

