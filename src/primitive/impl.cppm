module;
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

export module mcpplibs.primitives.primitive.impl;

import mcpplibs.primitives.underlying.traits;
import mcpplibs.primitives.policy.traits;
import mcpplibs.primitives.policy.impl;
import mcpplibs.primitives.policy.handler;

export namespace mcpplibs::primitives {

namespace details {

template <policy::policy_type... Policies> struct count_concurrency_policies;

template <> struct count_concurrency_policies<> {
  static constexpr std::size_t value = 0;
};

template <policy::policy_type First, policy::policy_type... Rest>
struct count_concurrency_policies<First, Rest...> {
  static constexpr bool is_match =
      policy::traits<First>::kind == policy::category::concurrency;
  static constexpr std::size_t value =
      count_concurrency_policies<Rest...>::value + (is_match ? 1u : 0u);
};

template <policy::policy_type... Policies> struct resolve_concurrency_policy;

template <> struct resolve_concurrency_policy<> {
  using type = policy::defaults::concurrency;
};

template <policy::policy_type First, policy::policy_type... Rest>
struct resolve_concurrency_policy<First, Rest...> {
  using type = std::conditional_t<
      policy::traits<First>::kind == policy::category::concurrency, First,
      typename resolve_concurrency_policy<Rest...>::type>;
};

template <policy::policy_type... Policies>
using resolve_concurrency_policy_t =
    typename resolve_concurrency_policy<Policies...>::type;

} // namespace details

template <underlying_type T, policy::policy_type... Policies> class primitive {
public:
  using value_type = T;
  using policies = std::tuple<Policies...>;
  using concurrency_policy = details::resolve_concurrency_policy_t<Policies...>;

  static_assert(details::count_concurrency_policies<Policies...>::value <= 1,
                "Multiple concurrency policies are not allowed");

  constexpr explicit primitive(value_type v) noexcept : value_(v) {}
  constexpr value_type &value() noexcept { return value_; }
  [[nodiscard]] constexpr value_type const &value() const noexcept {
    return value_;
  }
  constexpr explicit operator value_type() const noexcept { return value_; }

  [[nodiscard]] auto load() const noexcept -> value_type {
    using access_handler_t =
        policy::concurrency::handler<concurrency_policy, void, value_type,
                                     policy::error::kind>;
    static_assert(
        policy::concurrency::handler_access_available<concurrency_policy,
                                                      value_type>,
        "Selected concurrency policy does not provide primitive "
        "load/store/CAS support");
    return access_handler_t::load(value_);
  }

  auto store(value_type desired) noexcept -> void {
    using access_handler_t =
        policy::concurrency::handler<concurrency_policy, void, value_type,
                                     policy::error::kind>;
    static_assert(
        policy::concurrency::handler_access_available<concurrency_policy,
                                                      value_type>,
        "Selected concurrency policy does not provide primitive "
        "load/store/CAS support");
    access_handler_t::store(value_, desired);
  }

  auto compare_exchange(value_type &expected, value_type desired) noexcept
      -> bool {
    using access_handler_t =
        policy::concurrency::handler<concurrency_policy, void, value_type,
                                     policy::error::kind>;
    static_assert(
        policy::concurrency::handler_access_available<concurrency_policy,
                                                      value_type>,
        "Selected concurrency policy does not provide primitive "
        "load/store/CAS support");
    return access_handler_t::compare_exchange(value_, expected, desired);
  }

private:
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
