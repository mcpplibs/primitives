module;
#include <cstdint>
#include <tuple>

export module mcpplibs.primitives.primitive.impl;

import mcpplibs.primitives.underlying.traits;
import mcpplibs.primitives.policy.traits;

export namespace mcpplibs::primitives {

template <underlying_type T, policy::policy_type... Policies> class primitive {
public:
  using value_type = T;
  using policies = std::tuple<Policies...>;
  constexpr explicit primitive(value_type v) noexcept : value_(v) {}
  constexpr value_type &value() noexcept { return value_; }
  [[nodiscard]] constexpr value_type const &value() const noexcept { return value_; }
  constexpr explicit operator value_type() const noexcept { return value_; }

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
    }

} // namespace mcpplibs::primitives
