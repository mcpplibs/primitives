module;
#include <concepts>
#include <type_traits>

export module mcpplibs.primitives.underlying.traits;

export namespace mcpplibs::primitives {

template <typename T>
concept std_bool = std::same_as<std::remove_cv_t<T>, bool>;

template <typename T>
concept std_char = std::same_as<std::remove_cv_t<T>, char> ||
                   std::same_as<std::remove_cv_t<T>, signed char> ||
                   std::same_as<std::remove_cv_t<T>, unsigned char> ||
                   std::same_as<std::remove_cv_t<T>, wchar_t> ||
                   std::same_as<std::remove_cv_t<T>, char8_t> ||
                   std::same_as<std::remove_cv_t<T>, char16_t> ||
                   std::same_as<std::remove_cv_t<T>, char32_t>;

template <typename T>
concept std_floating = std::floating_point<std::remove_cv_t<T>>;

template <typename T>
concept std_integer =
    std::integral<std::remove_cv_t<T>> && (!std_bool<T>) && (!std_char<T>);

template <typename T>
concept std_numeric = std_integer<T> || std_floating<T>;

template <typename T>
concept std_underlying_type =
    std_bool<T> || std_char<T> || std_integer<T> || std_floating<T>;

namespace underlying {

enum class category {
  boolean,
  character,
  integer,
  floating,
};

template <typename T> struct traits {
  using value_type = void;
  using rep_type = void;

  static constexpr bool enabled = false;

  static constexpr auto kind = static_cast<category>(-1);

  template <typename U> static constexpr rep_type to_rep(U) noexcept {}

  template <typename U> static constexpr value_type from_rep(U) noexcept {}

  template <typename U> static constexpr bool is_valid_rep(U) noexcept {
    return false;
  }
};

template <typename LhsRep, typename RhsRep, typename = void>
struct common_rep_traits {
  using type = void;
  static constexpr bool enabled = false;
};

template <typename LhsRep, typename RhsRep>
struct common_rep_traits<
    LhsRep, RhsRep,
    std::void_t<std::common_type_t<std::remove_cv_t<LhsRep>,
                                   std::remove_cv_t<RhsRep>>>> {
  using type =
      std::common_type_t<std::remove_cv_t<LhsRep>, std::remove_cv_t<RhsRep>>;
  static constexpr bool enabled = true;
};

template <std_underlying_type T>
struct traits<T> {
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

} // namespace underlying
} // namespace mcpplibs::primitives

namespace mcpplibs::primitives::underlying::details {

template <typename T>
concept enabled = traits<std::remove_cv_t<T>>::enabled;

template <typename T>
concept has_rep_type =
    enabled<T> && requires { typename traits<std::remove_cv_t<T>>::rep_type; };

template <typename T>
concept has_category = enabled<T> && requires {
  { traits<std::remove_cv_t<T>>::kind } -> std::convertible_to<category>;
};

template <typename T>
concept has_rep_bridge =
    has_rep_type<T> &&
    requires(std::remove_cv_t<T> value,
             typename traits<std::remove_cv_t<T>>::rep_type rep) {
      {
        traits<std::remove_cv_t<T>>::to_rep(value)
      } -> std::same_as<typename traits<std::remove_cv_t<T>>::rep_type>;
      {
        traits<std::remove_cv_t<T>>::from_rep(rep)
      } -> std::same_as<std::remove_cv_t<T>>;
      { traits<std::remove_cv_t<T>>::is_valid_rep(rep) } -> std::same_as<bool>;
    };

template <typename T>
concept has_std_rep_type =
    has_rep_type<T> &&
    std_underlying_type<typename traits<std::remove_cv_t<T>>::rep_type>;

template <typename T>
concept has_custom_numeric_rep_type =
    has_rep_type<T> &&
    requires(typename traits<std::remove_cv_t<T>>::rep_type a,
             typename traits<std::remove_cv_t<T>>::rep_type b) {
      { a + b };
      { a - b };
      { a * b };
      { a / b };
      { a == b } -> std::convertible_to<bool>;
    };

template <typename T>
concept has_supported_rep_type =
    has_std_rep_type<T> || has_custom_numeric_rep_type<T>;

template <std_underlying_type T>
consteval category category_of_std_underlying_type() {
  if constexpr (std_bool<T>) {
    return category::boolean;
  } else if constexpr (std_char<T>) {
    return category::character;
  } else if constexpr (std_integer<T>) {
    return category::integer;
  } else {
    return category::floating;
  }
}

template <typename T>
concept has_consistent_category =
    has_category<T> && has_supported_rep_type<T> &&
    ((has_std_rep_type<T> &&
      (traits<std::remove_cv_t<T>>::kind ==
       category_of_std_underlying_type<
           typename traits<std::remove_cv_t<T>>::rep_type>())) ||
     (!has_std_rep_type<T> &&
      (traits<std::remove_cv_t<T>>::kind == category::integer ||
       traits<std::remove_cv_t<T>>::kind == category::floating)));

template <typename LhsRep, typename RhsRep>
concept has_common_rep =
    common_rep_traits<std::remove_cv_t<LhsRep>,
                                  std::remove_cv_t<RhsRep>>::enabled &&
    requires {
      typename common_rep_traits<std::remove_cv_t<LhsRep>,
                                             std::remove_cv_t<RhsRep>>::type;
    };

} // namespace mcpplibs::primitives::underlying::details

export namespace mcpplibs::primitives {

template <typename T>
concept underlying_type =
    underlying::details::enabled<T> && underlying::details::has_category<T> &&
    underlying::details::has_rep_bridge<T> &&
    underlying::details::has_supported_rep_type<T> &&
    underlying::details::has_consistent_category<T>;

template <typename T>
concept underlying_operand = underlying_type<std::remove_cvref_t<T>>;

template <typename T>
concept boolean_underlying_type =
    underlying_type<T> && (underlying::traits<std::remove_cv_t<T>>::kind ==
                           underlying::category::boolean);

template <typename T>
concept character_underlying_type =
    underlying_type<T> && (underlying::traits<std::remove_cv_t<T>>::kind ==
                           underlying::category::character);

template <typename T>
concept integer_underlying_type =
    underlying_type<T> && (underlying::traits<std::remove_cv_t<T>>::kind ==
                           underlying::category::integer);

template <typename T>
concept floating_underlying_type =
    underlying_type<T> && (underlying::traits<std::remove_cv_t<T>>::kind ==
                           underlying::category::floating);

template <typename T>
concept numeric_underlying_type =
    integer_underlying_type<T> || floating_underlying_type<T>;

template <typename LhsRep, typename RhsRep>
concept has_common_rep = underlying::details::has_common_rep<LhsRep, RhsRep>;

template <typename LhsRep, typename RhsRep>
using common_rep_t =
    underlying::common_rep_traits<std::remove_cv_t<LhsRep>,
                                           std::remove_cv_t<RhsRep>>::type;
} // namespace mcpplibs::primitives
