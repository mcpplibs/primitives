module;
#include <cstdint>
#include <tuple>

export module mcpplibs.primitives.primitive.impl;

import mcpplibs.primitives.underlying.traits;
import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.handler;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.policy.utility;

export namespace mcpplibs::primitives {

template <underlying_type T, policy::policy_type... Policies> class primitive {
public:
  using value_type = T;
  using policies = std::tuple<Policies...>;

private:
  using type_policy_t =
      policy::resolve_policy_t<policy::category::type, Policies...>;
  using value_rep_type = underlying::traits<value_type>::rep_type;

  template <underlying_type Source, underlying_type Target>
  static constexpr bool policy_allows_underlying_bridge_v = [] {
    using source_rep_type =
        underlying::traits<std::remove_cv_t<Source>>::rep_type;
    using target_rep_type =
        underlying::traits<std::remove_cv_t<Target>>::rep_type;
    if constexpr (std::same_as<type_policy_t, policy::type::strict>) {
      return false;
    } else if constexpr (std::same_as<type_policy_t, policy::type::compatible>) {
      return underlying::traits<std::remove_cv_t<Source>>::kind ==
                 underlying::traits<std::remove_cv_t<Target>>::kind &&
             has_common_rep<source_rep_type, target_rep_type> &&
             !std::same_as<common_rep_t<source_rep_type, target_rep_type>, void>;
    } else if constexpr (std::same_as<type_policy_t,
                                      policy::type::transparent>) {
      return has_common_rep<source_rep_type, target_rep_type> &&
             !std::same_as<common_rep_t<source_rep_type, target_rep_type>, void>;
    } else {
      return false;
    }
  }();

  template <underlying_type U>
  static constexpr bool cross_underlying_constructible_v =
      policy_allows_underlying_bridge_v<U, value_type>;

  template <underlying_type U>
  static constexpr bool cross_underlying_exchangeable_v =
      cross_underlying_constructible_v<U> &&
      policy_allows_underlying_bridge_v<value_type, U>;

  template <underlying_type Target, underlying_type Source>
  static constexpr auto convert_underlying_(Source source) noexcept -> Target {
    using source_value_type = std::remove_cv_t<Source>;
    using source_rep_type = underlying::traits<source_value_type>::rep_type;
    using target_rep_type =
        underlying::traits<std::remove_cv_t<Target>>::rep_type;

    auto const source_rep = underlying::traits<source_value_type>::to_rep(source);
    auto const target_rep = static_cast<target_rep_type>(
        static_cast<source_rep_type>(source_rep));
    return underlying::traits<std::remove_cv_t<Target>>::from_rep(target_rep);
  }

public:
  template <underlying_type U>
    requires std::same_as<U, value_type>
  constexpr explicit primitive(U u) noexcept : value_(u) {}

  template <underlying_type U>
    requires(!std::same_as<U, value_type> &&
             cross_underlying_constructible_v<U>)
  constexpr explicit primitive(U u) noexcept
      : value_(convert_underlying_<value_type>(u)) {}

  constexpr primitive(primitive const &other) noexcept {
    if consteval {
      value_ = other.value_;
    } else {
      value_ = other.load();
    }
  }

  template <underlying_type U>
    requires(!std::same_as<U, value_type>)
  explicit constexpr primitive(primitive<U, Policies...> const &other) noexcept
      requires(cross_underlying_constructible_v<U>)
      : value_(convert_underlying_<value_type>(other.load())) {}

  constexpr auto operator=(primitive const &other) noexcept -> primitive & {
    if (this == &other) {
      return *this;
    }

    if consteval {
      value_ = other.value_;
    } else {
      store(other.load());
    }
    return *this;
  }

  template <underlying_type U>
    requires(!std::same_as<U, value_type>)
  constexpr auto
  operator=(primitive<U, Policies...> const &other) noexcept -> primitive &
      requires(cross_underlying_constructible_v<U>) {
    store(convert_underlying_<value_type>(other.load()));
    return *this;
  }

  constexpr primitive(primitive &&other) noexcept {
    if consteval {
      value_ = other.value_;
    } else {
      value_ = other.load();
    }
  }

  template <underlying_type U>
    requires(!std::same_as<U, value_type>)
  explicit constexpr primitive(primitive<U, Policies...> &&other) noexcept
      requires(cross_underlying_constructible_v<U>)
      : value_(convert_underlying_<value_type>(other.load())) {}

  constexpr auto operator=(primitive &&other) noexcept -> primitive & {
    if (this == &other) {
      return *this;
    }

    if consteval {
      value_ = other.value_;
    } else {
      store(other.load());
    }
    return *this;
  }

  template <underlying_type U>
    requires(!std::same_as<U, value_type>)
  constexpr auto operator=(primitive<U, Policies...> &&other) noexcept
      -> primitive &
      requires(cross_underlying_constructible_v<U>) {
    store(convert_underlying_<value_type>(other.load()));
    return *this;
  }

  constexpr value_type &value() noexcept { return value_; }

  [[nodiscard]] constexpr value_type const &value() const noexcept {
    return value_;
  }

  constexpr explicit operator value_type() const noexcept { return value_; }

  [[nodiscard]] constexpr auto load() const noexcept -> value_type {
    if consteval {
      return value_;
    }
    require_access_handler_();
    return access_handler_t::load(value_);
  }

  template <underlying_type U>
    requires std::same_as<U, value_type>
  constexpr auto store(U desired) noexcept -> void {
    if consteval {
      value_ = desired;
    } else {
      require_access_handler_();
      access_handler_t::store(value_, desired);
    }
  }

  template <underlying_type U>
    requires(!std::same_as<U, value_type> &&
             cross_underlying_constructible_v<U>)
  constexpr auto store(U desired) noexcept -> void {
    store(convert_underlying_<value_type>(desired));
  }

  template <underlying_type U>
    requires std::same_as<U, value_type>
  constexpr auto compare_exchange(U &expected, U desired) noexcept -> bool {
    if consteval {
      if (value_ != expected) {
        expected = value_;
        return false;
      }

      value_ = desired;
      return true;
    }
    require_access_handler_();
    return access_handler_t::compare_exchange(value_, expected, desired);
  }

  template <underlying_type U>
    requires(!std::same_as<U, value_type> && cross_underlying_exchangeable_v<U>)
  constexpr auto compare_exchange(U &expected, U desired) noexcept -> bool {
    auto expected_native = convert_underlying_<value_type>(expected);
    auto const desired_native = convert_underlying_<value_type>(desired);

    auto const exchanged = compare_exchange(expected_native, desired_native);
    if (!exchanged) {
      expected = convert_underlying_<U>(expected_native);
    }
    return exchanged;
  }

private:
  using concurrency_policy_t = policy::resolve_policy_t<
      policy::category::concurrency, Policies...>;

  using access_handler_t =
      policy::concurrency::handler<concurrency_policy_t, void, value_type,
                                   policy::error::kind>;

  static constexpr auto require_access_handler_() noexcept -> void {
    static_assert(
        policy::concurrency::handler_access_available<concurrency_policy_t,
                                                      value_type>,
        "Selected concurrency policy does not provide primitive "
        "load/store/CAS support");
  }

  value_type value_;
};

namespace types {
template <policy::policy_type... Policies>
using Bool = primitive<bool, Policies...>;

template <policy::policy_type... Policies>
using UChar = primitive<unsigned char, Policies...>;
template <policy::policy_type... Policies>
using Char8 = primitive<char8_t, Policies...>;
template <policy::policy_type... Policies>
using Char16 = primitive<char16_t, Policies...>;
template <policy::policy_type... Policies>
using Char32 = primitive<char32_t, Policies...>;
template <policy::policy_type... Policies>
using WChar = primitive<wchar_t, Policies...>;

template <policy::policy_type... Policies>
using U8 = primitive<std::uint8_t, Policies...>;
template <policy::policy_type... Policies>
using U16 = primitive<std::uint16_t, Policies...>;
template <policy::policy_type... Policies>
using U32 = primitive<std::uint32_t, Policies...>;
template <policy::policy_type... Policies>
using U64 = primitive<std::uint64_t, Policies...>;
template <policy::policy_type... Policies>
using I8 = primitive<std::int8_t, Policies...>;
template <policy::policy_type... Policies>
using I16 = primitive<std::int16_t, Policies...>;
template <policy::policy_type... Policies>
using I32 = primitive<std::int32_t, Policies...>;
template <policy::policy_type... Policies>
using I64 = primitive<std::int64_t, Policies...>;

template <policy::policy_type... Policies>
using F32 = primitive<float, Policies...>;
template <policy::policy_type... Policies>
using F64 = primitive<double, Policies...>;
template <policy::policy_type... Policies>
using F80 = primitive<long double, Policies...>;
} // namespace types

} // namespace mcpplibs::primitives
