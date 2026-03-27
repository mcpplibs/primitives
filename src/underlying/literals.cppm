module;

#include <concepts>
#include <cstddef> // NOLINT
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

export module mcpplibs.primitives.underlying.literals;

import mcpplibs.primitives.algorithms.limits;
import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.underlying;

namespace mcpplibs::primitives::underlying::details {

template <conversion::risk::kind Kind>
consteval auto throw_literal_risk() -> void {
  if constexpr (Kind == conversion::risk::kind::overflow ||
                Kind == conversion::risk::kind::underflow) {
    throw std::out_of_range{
        "numeric literal is out of range for target underlying type"};
  } else if constexpr (Kind == conversion::risk::kind::precision_loss) {
    throw std::invalid_argument{
        "numeric literal loses precision for target underlying type"};
  } else if constexpr (Kind == conversion::risk::kind::domain_error) {
    throw std::invalid_argument{
        "numeric literal must be finite for target underlying type"};
  } else {
    throw std::invalid_argument{
        "numeric literal is not representable for target underlying type"};
  }
}

template <std::floating_point T>
consteval auto ordered(T value) -> bool {
  return (value < static_cast<T>(0)) || (value >= static_cast<T>(0));
}

template <std::floating_point T>
consteval auto finite(T value) -> bool {
  return ordered(value) &&
         value >= algorithms::lowest_value<T>() &&
         value <= algorithms::max_value<T>();
}

template <typename To, typename From>
consteval auto out_of_floating_range(From value) -> bool {
  using value_type = std::remove_cv_t<To>;
  auto const normalized = static_cast<long double>(value);
  auto const lowest =
      static_cast<long double>(algorithms::lowest_value<value_type>());
  auto const max = static_cast<long double>(algorithms::max_value<value_type>());
  return normalized < lowest || normalized > max;
}

template <std::integral To>
consteval auto checked_integral_literal(unsigned long long value) -> To {
  using value_type = std::remove_cv_t<To>;
  constexpr auto max_value =
      static_cast<unsigned long long>(algorithms::max_value<value_type>());

  if (value > max_value) {
    throw_literal_risk<conversion::risk::kind::overflow>();
  }

  return static_cast<value_type>(value);
}

template <std::floating_point To, std_numeric From>
consteval auto checked_floating_literal(From value) -> To {
  using source_type = std::remove_cv_t<From>;
  using value_type = std::remove_cv_t<To>;

  if constexpr (std_floating<source_type>) {
    if (!ordered(value)) {
      throw_literal_risk<conversion::risk::kind::domain_error>();
    }
  }

  if (out_of_floating_range<value_type>(value)) {
    if constexpr (std::signed_integral<source_type> || std_floating<source_type>) {
      if (value < static_cast<source_type>(0)) {
        throw_literal_risk<conversion::risk::kind::underflow>();
      }
    }
    throw_literal_risk<conversion::risk::kind::overflow>();
  }

  auto const converted = static_cast<value_type>(value);

  if constexpr (std_floating<value_type>) {
    if (!finite(converted)) {
      if constexpr (std::signed_integral<source_type> || std_floating<source_type>) {
        if (value < static_cast<source_type>(0)) {
          throw_literal_risk<conversion::risk::kind::underflow>();
        }
      }
      throw_literal_risk<conversion::risk::kind::overflow>();
    }
  }

  return converted;
}

template <std::floating_point To, std_numeric From>
consteval auto exact_floating_literal(From value) -> To {
  using source_type = std::remove_cv_t<From>;
  using value_type = std::remove_cv_t<To>;

  auto const converted = checked_floating_literal<value_type>(value);

  if constexpr (std::integral<source_type>) {
    if (static_cast<source_type>(converted) != value) {
      throw_literal_risk<conversion::risk::kind::precision_loss>();
    }
  } else {
    auto const roundtrip = static_cast<source_type>(converted);
    if (!ordered(roundtrip)) {
      throw_literal_risk<conversion::risk::kind::domain_error>();
    }
    if (roundtrip != value) {
      if (converted == static_cast<value_type>(0) &&
          value != static_cast<source_type>(0)) {
        throw_literal_risk<conversion::risk::kind::underflow>();
      }
      throw_literal_risk<conversion::risk::kind::precision_loss>();
    }
  }

  return converted;
}

template <char... Cs>
consteval auto parse_unsigned_decimal_literal() -> unsigned long long {
  constexpr char input[] {Cs..., '\0'};
  constexpr auto max_value = std::numeric_limits<unsigned long long>::max();

  unsigned long long value {};
  for (std::size_t i = 0; i < sizeof...(Cs); ++i) {
    auto const ch = input[i];
    if (ch < '0' || ch > '9') {
      throw std::invalid_argument{"invalid integer literal"};
    }

    auto const digit = static_cast<unsigned long long>(ch - '0');
    if (value > max_value / 10ULL ||
        (value == max_value / 10ULL && digit > max_value % 10ULL)) {
      throw std::out_of_range{"integer literal is out of range"};
    }

    value = value * 10ULL + digit;
  }

  return value;
}

template <std::integral To, char... Cs>
consteval auto literal_integral() -> To {
  return checked_integral_literal<To>(parse_unsigned_decimal_literal<Cs...>());
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

template <char... Cs>
consteval auto operator""_u8() -> std::uint8_t {
  return underlying::details::literal_integral<std::uint8_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_u16() -> std::uint16_t {
  return underlying::details::literal_integral<std::uint16_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_u32() -> std::uint32_t {
  return underlying::details::literal_integral<std::uint32_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_u64() -> std::uint64_t {
  return underlying::details::literal_integral<std::uint64_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_size() -> std::size_t {
  return underlying::details::literal_integral<std::size_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_diff() -> std::ptrdiff_t {
  return underlying::details::literal_integral<std::ptrdiff_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_i8() -> std::int8_t {
  return underlying::details::literal_integral<std::int8_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_i16() -> std::int16_t {
  return underlying::details::literal_integral<std::int16_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_i32() -> std::int32_t {
  return underlying::details::literal_integral<std::int32_t, Cs...>();
}

template <char... Cs>
consteval auto operator""_i64() -> std::int64_t {
  return underlying::details::literal_integral<std::int64_t, Cs...>();
}

consteval auto operator""_f32(const unsigned long long value) -> float {
  return underlying::details::checked_floating_literal<float>(value);
}

consteval auto operator""_f32(const long double value) -> float {
  return underlying::details::checked_floating_literal<float>(value);
}

consteval auto operator""_f32e(const unsigned long long value) -> float {
  return underlying::details::exact_floating_literal<float>(value);
}

consteval auto operator""_f32e(const long double value) -> float {
  return underlying::details::exact_floating_literal<float>(value);
}

consteval auto operator""_f64(const unsigned long long value) -> double {
  return underlying::details::checked_floating_literal<double>(value);
}

consteval auto operator""_f64(const long double value) -> double {
  return underlying::details::checked_floating_literal<double>(value);
}

consteval auto operator""_f64e(const unsigned long long value) -> double {
  return underlying::details::exact_floating_literal<double>(value);
}

consteval auto operator""_f64e(const long double value) -> double {
  return underlying::details::exact_floating_literal<double>(value);
}

consteval auto operator""_f80(const unsigned long long value) -> long double {
  return underlying::details::checked_floating_literal<long double>(value);
}

consteval auto operator""_f80(const long double value) -> long double {
  return underlying::details::checked_floating_literal<long double>(value);
}

consteval auto operator""_f80e(const unsigned long long value) -> long double {
  return underlying::details::exact_floating_literal<long double>(value);
}

consteval auto operator""_f80e(const long double value) -> long double {
  return underlying::details::exact_floating_literal<long double>(value);
}

} // namespace mcpplibs::primitives::literals
