module;

#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

export module mcpplibs.primitives.conversion.underlying;

import mcpplibs.primitives.conversion.traits;
import mcpplibs.primitives.underlying.traits;

namespace mcpplibs::primitives::conversion::details {

template <typename DestRep, typename SrcRep>
constexpr auto narrow_integral_error(SrcRep value)
    -> std::optional<risk::kind> {
  using dest_type = std::remove_cv_t<DestRep>;
  using src_type = std::remove_cv_t<SrcRep>;

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

template <typename DestRep, typename SrcRep>
constexpr auto narrow_numeric_error(SrcRep value) -> std::optional<risk::kind> {
  using dest_type = std::remove_cv_t<DestRep>;
  using src_type = std::remove_cv_t<SrcRep>;

  if constexpr (std::integral<dest_type> && std::integral<src_type>) {
    return narrow_integral_error<dest_type>(value);
  } else if constexpr (std::integral<dest_type> &&
                       std::floating_point<src_type>) {
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
  } else {
    static_cast<void>(value);
    return std::nullopt;
  }
}

template <typename DestRep, typename SrcRep>
constexpr auto safe_numeric_cast(SrcRep value) noexcept -> DestRep {
  using dest_type = std::remove_cv_t<DestRep>;
  using src_type = std::remove_cv_t<SrcRep>;

  if constexpr (std::integral<dest_type> && std::floating_point<src_type>) {
    if (auto const kind = narrow_numeric_error<dest_type>(value);
        kind.has_value()) {
      if (*kind == risk::kind::overflow) {
        return std::numeric_limits<dest_type>::max();
      }
      if (*kind == risk::kind::underflow) {
        return std::numeric_limits<dest_type>::lowest();
      }
      return dest_type{};
    }
  }

  return static_cast<dest_type>(value);
}

} // namespace mcpplibs::primitives::conversion::details

export namespace mcpplibs::primitives::conversion::underlying {

template <typename DestRep, typename SrcRep>
constexpr auto narrow_integral_error(SrcRep value)
    -> std::optional<risk::kind> {
  return details::narrow_integral_error<DestRep>(value);
}

template <typename DestRep, typename SrcRep>
constexpr auto narrow_numeric_error(SrcRep value) -> std::optional<risk::kind> {
  return details::narrow_numeric_error<DestRep>(value);
}

template <typename DestRep, typename SrcRep>
constexpr auto safe_numeric_cast(SrcRep value) noexcept -> DestRep {
  return details::safe_numeric_cast<DestRep>(value);
}

template <underlying_type Dest, underlying_type Src>
constexpr auto cast_unchecked(Src value) noexcept -> Dest {
  using src_type = std::remove_cv_t<Src>;
  using dest_type = std::remove_cv_t<Dest>;
  using src_rep_type = primitives::underlying::traits<src_type>::rep_type;
  using dest_rep_type =
      primitives::underlying::traits<dest_type>::rep_type;

  auto const source_rep =
      primitives::underlying::traits<src_type>::to_rep(value);
  auto const target_rep =
      safe_numeric_cast<dest_rep_type>(static_cast<src_rep_type>(source_rep));
  return primitives::underlying::traits<dest_type>::from_rep(
      target_rep);
}

} // namespace mcpplibs::primitives::conversion::underlying
