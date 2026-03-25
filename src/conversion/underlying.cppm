module;

#include <cmath>
#include <concepts>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <type_traits>

export module mcpplibs.primitives.conversion.underlying;

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.underlying.traits;

namespace mcpplibs::primitives::conversion::details {

template <typename T>
concept integral_like = std::integral<std::remove_cvref_t<T>>;

template <typename T>
concept floating_like = std::floating_point<std::remove_cvref_t<T>>;

template <typename T>
concept numeric_like = integral_like<T> || floating_like<T>;

template <typename T>
concept custom_numeric_like =
    requires(std::remove_cvref_t<T> a, std::remove_cvref_t<T> b) {
      { a + b };
      { a - b };
      { a * b };
      { a / b };
      { a == b } -> std::convertible_to<bool>;
    };

template <typename T>
concept numeric_cast_operand = numeric_like<T> || custom_numeric_like<T>;

template <typename DestRep, typename SrcRep>
concept statically_castable = requires(SrcRep value) {
  static_cast<std::remove_cvref_t<DestRep>>(value);
};

template <typename DestRep, typename SrcRep>
concept builtin_numeric_pair = numeric_like<DestRep> && numeric_like<SrcRep>;

template <integral_like DestRep, integral_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (std::is_signed_v<src_type>) {
    auto const signed_value = static_cast<std::intmax_t>(value);
    if constexpr (std::is_signed_v<dest_type>) {
      if (signed_value <
          static_cast<std::intmax_t>(std::numeric_limits<dest_type>::min())) {
        return risk::kind::underflow;
      }
      if (signed_value >
          static_cast<std::intmax_t>(std::numeric_limits<dest_type>::max())) {
        return risk::kind::overflow;
      }
      return std::nullopt;
    } else {
      if (signed_value < 0) {
        return risk::kind::underflow;
      }

      if (static_cast<std::uintmax_t>(signed_value) >
          static_cast<std::uintmax_t>(std::numeric_limits<dest_type>::max())) {
        return risk::kind::overflow;
      }
      return std::nullopt;
    }
  } else {
    auto const unsigned_value = static_cast<std::uintmax_t>(value);
    if (unsigned_value >
        static_cast<std::uintmax_t>(std::numeric_limits<dest_type>::max())) {
      return risk::kind::overflow;
    }
    return std::nullopt;
  }
}

template <integral_like DestRep, floating_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if (std::isnan(value)) {
    return risk::kind::domain_error;
  }
  if (std::isinf(value)) {
    return value < static_cast<src_type>(0) ? risk::kind::underflow
                                            : risk::kind::overflow;
  }

  auto const normalized = static_cast<long double>(value);
  auto const min_value =
      static_cast<long double>(std::numeric_limits<dest_type>::lowest());
  auto const max_value =
      static_cast<long double>(std::numeric_limits<dest_type>::max());

  if (normalized < min_value) {
    return risk::kind::underflow;
  }
  if (normalized > max_value) {
    return risk::kind::overflow;
  }
  return std::nullopt;
}

template <floating_like DestRep, integral_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  auto const converted = static_cast<dest_type>(value);
  if (std::isinf(converted)) {
    return value < static_cast<src_type>(0) ? risk::kind::underflow
                                            : risk::kind::overflow;
  }

  auto const roundtrip = static_cast<src_type>(converted);
  if (roundtrip != value) {
    return risk::kind::precision_loss;
  }

  return std::nullopt;
}

template <floating_like DestRep, floating_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  using dest_type = std::remove_cvref_t<DestRep>;

  if (std::isnan(value)) {
    return std::nullopt;
  }
  if (std::isinf(value)) {
    return std::nullopt;
  }

  auto const normalized = static_cast<long double>(value);
  auto const min_value =
      static_cast<long double>(std::numeric_limits<dest_type>::lowest());
  auto const max_value =
      static_cast<long double>(std::numeric_limits<dest_type>::max());

  if (normalized < min_value) {
    return risk::kind::underflow;
  }
  if (normalized > max_value) {
    return risk::kind::overflow;
  }

  auto const converted = static_cast<dest_type>(value);
  auto const roundtrip = static_cast<std::remove_cvref_t<SrcRep>>(converted);
  if (roundtrip != value) {
    return risk::kind::precision_loss;
  }

  return std::nullopt;
}

template <numeric_cast_operand DestRep, numeric_cast_operand SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto unchecked_rep_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  using dest_type = std::remove_cvref_t<DestRep>;
  return static_cast<dest_type>(value);
}

template <numeric_cast_operand DestRep, numeric_cast_operand SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto checked_rep_cast(SrcRep value)
    -> cast_result<std::remove_cvref_t<DestRep>> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (builtin_numeric_pair<dest_type, src_type>) {
    if (auto const kind = numeric_risk<dest_type>(value); kind.has_value()) {
      return std::unexpected(*kind);
    }
  }

  return static_cast<dest_type>(value);
}

template <numeric_cast_operand DestRep, numeric_cast_operand SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto saturating_rep_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (builtin_numeric_pair<dest_type, src_type>) {
    if (auto const kind = numeric_risk<dest_type>(value); kind.has_value()) {
      if (*kind == risk::kind::overflow) {
        return std::numeric_limits<dest_type>::max();
      }
      if (*kind == risk::kind::underflow) {
        return std::numeric_limits<dest_type>::lowest();
      }
      if (*kind == risk::kind::domain_error) {
        return dest_type{};
      }
    }
  }

  return static_cast<dest_type>(value);
}

template <numeric_cast_operand DestRep, numeric_cast_operand SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto truncating_rep_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (integral_like<dest_type> && floating_like<src_type>) {
    if (std::isnan(value)) {
      return dest_type{};
    }
    if (std::isinf(value)) {
      return value < static_cast<src_type>(0)
                 ? std::numeric_limits<dest_type>::lowest()
                 : std::numeric_limits<dest_type>::max();
    }

    auto const normalized = static_cast<long double>(value);
    auto const min_value =
        static_cast<long double>(std::numeric_limits<dest_type>::lowest());
    auto const max_value =
        static_cast<long double>(std::numeric_limits<dest_type>::max());

    if (normalized < min_value) {
      return std::numeric_limits<dest_type>::lowest();
    }
    if (normalized > max_value) {
      return std::numeric_limits<dest_type>::max();
    }
  }

  return static_cast<dest_type>(value);
}

template <numeric_cast_operand DestRep, numeric_cast_operand SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto exact_rep_cast(SrcRep value)
    -> cast_result<std::remove_cvref_t<DestRep>> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (!builtin_numeric_pair<dest_type, src_type>) {
    return std::unexpected(risk::kind::invalid_type_combination);
  } else {
    if (auto const kind = numeric_risk<dest_type>(value); kind.has_value()) {
      return std::unexpected(*kind);
    }
    return static_cast<dest_type>(value);
  }
}

} // namespace mcpplibs::primitives::conversion::details

export namespace mcpplibs::primitives::conversion::underlying {

template <details::integral_like DestRep, details::integral_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_risk<DestRep>(value);
}

template <details::integral_like DestRep, details::floating_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_risk<DestRep>(value);
}

template <details::floating_like DestRep, details::integral_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_risk<DestRep>(value);
}

template <details::floating_like DestRep, details::floating_like SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_risk<DestRep>(value);
}

template <details::numeric_cast_operand DestRep,
          details::numeric_cast_operand SrcRep>
  requires details::statically_castable<DestRep, SrcRep> &&
           !(underlying_type<std::remove_cvref_t<DestRep>> &&
             underlying_type<std::remove_cvref_t<SrcRep>>)
constexpr auto unchecked_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  return details::unchecked_rep_cast<DestRep>(value);
}

template <details::numeric_cast_operand DestRep,
          details::numeric_cast_operand SrcRep>
  requires details::statically_castable<DestRep, SrcRep> &&
           !(underlying_type<std::remove_cvref_t<DestRep>> &&
             underlying_type<std::remove_cvref_t<SrcRep>>)
constexpr auto checked_cast(SrcRep value)
    -> cast_result<std::remove_cvref_t<DestRep>> {
  return details::checked_rep_cast<DestRep>(value);
}

template <details::numeric_cast_operand DestRep,
          details::numeric_cast_operand SrcRep>
  requires details::statically_castable<DestRep, SrcRep> &&
           !(underlying_type<std::remove_cvref_t<DestRep>> &&
             underlying_type<std::remove_cvref_t<SrcRep>>)
constexpr auto saturating_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  return details::saturating_rep_cast<DestRep>(value);
}

template <details::numeric_cast_operand DestRep,
          details::numeric_cast_operand SrcRep>
  requires details::statically_castable<DestRep, SrcRep> &&
           !(underlying_type<std::remove_cvref_t<DestRep>> &&
             underlying_type<std::remove_cvref_t<SrcRep>>)
constexpr auto truncating_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  return details::truncating_rep_cast<DestRep>(value);
}

template <details::numeric_cast_operand DestRep,
          details::numeric_cast_operand SrcRep>
  requires details::statically_castable<DestRep, SrcRep> &&
           !(underlying_type<std::remove_cvref_t<DestRep>> &&
             underlying_type<std::remove_cvref_t<SrcRep>>)
constexpr auto exact_cast(SrcRep value)
    -> cast_result<std::remove_cvref_t<DestRep>> {
  return details::exact_rep_cast<DestRep>(value);
}

template <underlying_type Dest, underlying_type Src>
constexpr auto unchecked_cast(Src value) noexcept -> Dest {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = primitives::underlying::traits<src_type>::rep_type;
  using dest_rep_type = primitives::underlying::traits<dest_type>::rep_type;

  auto const source_rep = primitives::underlying::traits<src_type>::to_rep(value);
  auto const target_rep =
      details::unchecked_rep_cast<dest_rep_type>(static_cast<src_rep_type>(source_rep));
  return primitives::underlying::traits<dest_type>::from_rep(target_rep);
}

template <underlying_type Dest, underlying_type Src>
constexpr auto checked_cast(Src value) -> cast_result<Dest> {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = primitives::underlying::traits<src_type>::rep_type;
  using dest_rep_type = primitives::underlying::traits<dest_type>::rep_type;

  auto const source_rep = primitives::underlying::traits<src_type>::to_rep(value);
  auto const target_rep =
      details::checked_rep_cast<dest_rep_type>(static_cast<src_rep_type>(source_rep));
  if (!target_rep.has_value()) {
    return std::unexpected(target_rep.error());
  }

  return primitives::underlying::traits<dest_type>::from_rep(*target_rep);
}

template <underlying_type Dest, underlying_type Src>
constexpr auto saturating_cast(Src value) noexcept -> Dest {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = primitives::underlying::traits<src_type>::rep_type;
  using dest_rep_type = primitives::underlying::traits<dest_type>::rep_type;

  auto const source_rep = primitives::underlying::traits<src_type>::to_rep(value);
  auto const target_rep =
      details::saturating_rep_cast<dest_rep_type>(static_cast<src_rep_type>(source_rep));
  return primitives::underlying::traits<dest_type>::from_rep(target_rep);
}

template <underlying_type Dest, underlying_type Src>
constexpr auto truncating_cast(Src value) noexcept -> Dest {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = primitives::underlying::traits<src_type>::rep_type;
  using dest_rep_type = primitives::underlying::traits<dest_type>::rep_type;

  auto const source_rep = primitives::underlying::traits<src_type>::to_rep(value);
  auto const target_rep =
      details::truncating_rep_cast<dest_rep_type>(static_cast<src_rep_type>(source_rep));
  return primitives::underlying::traits<dest_type>::from_rep(target_rep);
}

template <underlying_type Dest, underlying_type Src>
constexpr auto exact_cast(Src value) -> cast_result<Dest> {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = primitives::underlying::traits<src_type>::rep_type;
  using dest_rep_type = primitives::underlying::traits<dest_type>::rep_type;

  auto const source_rep = primitives::underlying::traits<src_type>::to_rep(value);
  auto const target_rep =
      details::exact_rep_cast<dest_rep_type>(static_cast<src_rep_type>(source_rep));
  if (!target_rep.has_value()) {
    return std::unexpected(target_rep.error());
  }

  return primitives::underlying::traits<dest_type>::from_rep(*target_rep);
}

} // namespace mcpplibs::primitives::conversion::underlying
