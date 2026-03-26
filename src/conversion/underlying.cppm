module;

#include <cmath>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <type_traits>

export module mcpplibs.primitives.conversion.underlying;

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.underlying.traits;

namespace mcpplibs::primitives::conversion::details {

template <typename DestRep, typename SrcRep>
concept statically_castable = requires(SrcRep value) {
  static_cast<std::remove_cvref_t<DestRep>>(value);
};

template <typename DestRep, typename SrcRep>
concept builtin_numeric_pair =
    std_numeric<DestRep> && std_numeric<SrcRep>;

template <underlying_type T>
using underlying_rep_t = underlying::traits<std::remove_cv_t<T>>::rep_type;

template <typename Rep, typename Candidate>
concept builtin_numeric_proxy_candidate =
    std_numeric<Candidate> && has_common_rep<Rep, Candidate> &&
    !std::same_as<common_rep_t<Rep, Candidate>, void> &&
    std::same_as<common_rep_t<Rep, Candidate>, std::remove_cv_t<Candidate>> &&
    statically_castable<Candidate, Rep>;

template <typename Rep>
struct integer_builtin_proxy {
  using type = std::conditional_t<
      builtin_numeric_proxy_candidate<Rep, short>, short,
      std::conditional_t<
          builtin_numeric_proxy_candidate<Rep, unsigned short>, unsigned short,
          std::conditional_t<
              builtin_numeric_proxy_candidate<Rep, int>, int,
              std::conditional_t<
                  builtin_numeric_proxy_candidate<Rep, unsigned int>,
                  unsigned int,
                  std::conditional_t<
                      builtin_numeric_proxy_candidate<Rep, long>, long,
                      std::conditional_t<
                          builtin_numeric_proxy_candidate<Rep, unsigned long>,
                          unsigned long,
                          std::conditional_t<
                              builtin_numeric_proxy_candidate<Rep, long long>,
                              long long,
                              std::conditional_t<
                                  builtin_numeric_proxy_candidate<
                                      Rep, unsigned long long>,
                                  unsigned long long, void>>>>>>>>;
};

template <typename Rep>
using integer_builtin_proxy_t = integer_builtin_proxy<Rep>::type;

template <typename Rep>
struct floating_builtin_proxy {
  using type = std::conditional_t<
      builtin_numeric_proxy_candidate<Rep, float>, float,
      std::conditional_t<
          builtin_numeric_proxy_candidate<Rep, double>, double,
          std::conditional_t<
              builtin_numeric_proxy_candidate<Rep, long double>, long double,
              void>>>;
};

template <typename Rep>
using floating_builtin_proxy_t = floating_builtin_proxy<Rep>::type;

template <std_integer DestRep, std_integer SrcRep>
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

template <std_integer DestRep, std_floating SrcRep>
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

template <std_floating DestRep, std_integer SrcRep>
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

template <std_floating DestRep, std_floating SrcRep>
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

template <numeric_underlying_type Dest, numeric_underlying_type Src>
constexpr auto numeric_underlying_risk(Src value)
    -> std::optional<risk::kind> {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = underlying_rep_t<src_type>;
  using dest_rep_type = underlying_rep_t<dest_type>;
  using src_builtin_rep_type = std::conditional_t<
      integer_underlying_type<src_type>, integer_builtin_proxy_t<src_rep_type>,
      floating_builtin_proxy_t<src_rep_type>>;
  using dest_builtin_rep_type = std::conditional_t<
      integer_underlying_type<dest_type>,
      integer_builtin_proxy_t<dest_rep_type>,
      floating_builtin_proxy_t<dest_rep_type>>;

  if constexpr (std::same_as<src_builtin_rep_type, void> ||
                std::same_as<dest_builtin_rep_type, void>) {
    static_cast<void>(value);
    return risk::kind::invalid_type_combination;
  } else {
    auto const source_rep = underlying::traits<src_type>::to_rep(value);
    return numeric_risk<dest_builtin_rep_type>(
        static_cast<src_builtin_rep_type>(source_rep));
  }
}

template <numeric_underlying_type DestRep, numeric_underlying_type SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto unchecked_rep_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  using dest_type = std::remove_cvref_t<DestRep>;
  return static_cast<dest_type>(value);
}

template <numeric_underlying_type DestRep, numeric_underlying_type SrcRep>
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

template <numeric_underlying_type DestRep, numeric_underlying_type SrcRep>
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

template <numeric_underlying_type DestRep, numeric_underlying_type SrcRep>
  requires statically_castable<DestRep, SrcRep>
constexpr auto truncating_rep_cast(SrcRep value) noexcept
    -> std::remove_cvref_t<DestRep> {
  using dest_type = std::remove_cvref_t<DestRep>;
  using src_type = std::remove_cvref_t<SrcRep>;

  if constexpr (std_integer<dest_type> && std_floating<src_type>) {
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

template <numeric_underlying_type DestRep, numeric_underlying_type SrcRep>
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

template <underlying_type Dest, underlying_type Src, typename RepCaster>
constexpr auto cast_underlying_value(Src value, RepCaster rep_caster) -> Dest {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = underlying::traits<src_type>::rep_type;
  using dest_rep_type = underlying::traits<dest_type>::rep_type;

  auto const source_rep = underlying::traits<src_type>::to_rep(value);
  auto const target_rep = rep_caster.template operator()<dest_rep_type>(
      static_cast<src_rep_type>(source_rep));
  return underlying::traits<dest_type>::from_rep(target_rep);
}

template <underlying_type Dest, underlying_type Src, typename RepCaster>
constexpr auto cast_underlying_result(Src value, RepCaster rep_caster)
    -> cast_result<Dest> {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = underlying::traits<src_type>::rep_type;
  using dest_rep_type = underlying::traits<dest_type>::rep_type;

  auto const source_rep = underlying::traits<src_type>::to_rep(value);
  auto const target_rep = rep_caster.template operator()<dest_rep_type>(
      static_cast<src_rep_type>(source_rep));
  if (!target_rep.has_value()) {
    return std::unexpected(target_rep.error());
  }

  return underlying::traits<dest_type>::from_rep(*target_rep);
}

} // namespace mcpplibs::primitives::conversion::details

export namespace mcpplibs::primitives::conversion {

template <integer_underlying_type DestRep, integer_underlying_type SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_underlying_risk<DestRep>(value);
}

template <integer_underlying_type DestRep, floating_underlying_type SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_underlying_risk<DestRep>(value);
}

template <floating_underlying_type DestRep, integer_underlying_type SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_underlying_risk<DestRep>(value);
}

template <floating_underlying_type DestRep, floating_underlying_type SrcRep>
constexpr auto numeric_risk(SrcRep value)
    -> std::optional<risk::kind> {
  return details::numeric_underlying_risk<DestRep>(value);
}

template <underlying_type Dest, underlying_type Src>
constexpr auto unchecked_cast(Src value) noexcept -> Dest {
  return details::cast_underlying_value<Dest>(
      value, []<typename DestRep, typename SrcRep>(SrcRep rep) {
        return details::unchecked_rep_cast<DestRep>(rep);
      });
}

template <underlying_type Dest, underlying_type Src>
constexpr auto checked_cast(Src value) -> cast_result<Dest> {
  return details::cast_underlying_result<Dest>(
      value, []<typename DestRep, typename SrcRep>(SrcRep rep) {
        return details::checked_rep_cast<DestRep>(rep);
      });
}

template <underlying_type Dest, underlying_type Src>
constexpr auto saturating_cast(Src value) noexcept -> Dest {
  return details::cast_underlying_value<Dest>(
      value, []<typename DestRep, typename SrcRep>(SrcRep rep) {
        return details::saturating_rep_cast<DestRep>(rep);
      });
}

template <underlying_type Dest, underlying_type Src>
constexpr auto truncating_cast(Src value) noexcept -> Dest {
  return details::cast_underlying_value<Dest>(
      value, []<typename DestRep, typename SrcRep>(SrcRep rep) {
        return details::truncating_rep_cast<DestRep>(rep);
      });
}

template <underlying_type Dest, underlying_type Src>
constexpr auto exact_cast(Src value) -> cast_result<Dest> {
  return details::cast_underlying_result<Dest>(
      value, []<typename DestRep, typename SrcRep>(SrcRep rep) {
        return details::exact_rep_cast<DestRep>(rep);
      });
}

} // namespace mcpplibs::primitives::conversion
