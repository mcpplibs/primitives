module;
#include <type_traits>

export module mcpplibs.primitives.traits.policy;

export namespace mcpplibs::primitives::policy {

enum class category { value, type, error, concurrency };

template <typename P> struct traits {
  using policy_type = void;
  static constexpr bool enabled = false;
  static constexpr category kind = static_cast<category>(-1);
};

struct checked_value {};
struct unchecked_value {};
struct saturating_value {};

struct strict_type {};
struct category_compatible_type {};
struct transparent_type {};

struct throw_error {};
struct expected_error {};
struct terminate_error {};

struct single_thread {};
struct atomic {};

template <> struct traits<checked_value> {
  using policy_type = checked_value;
  static constexpr bool enabled = true;
  static constexpr category kind = category::value;
};

template <> struct traits<unchecked_value> {
  using policy_type = unchecked_value;
  static constexpr bool enabled = true;
  static constexpr category kind = category::value;
};

template <> struct traits<saturating_value> {
  using policy_type = saturating_value;
  static constexpr bool enabled = true;
  static constexpr category kind = category::value;
};

template <> struct traits<strict_type> {
  using policy_type = strict_type;
  static constexpr bool enabled = true;
  static constexpr category kind = category::type;
};

template <> struct traits<category_compatible_type> {
  using policy_type = category_compatible_type;
  static constexpr bool enabled = true;
  static constexpr category kind = category::type;
};

template <> struct traits<transparent_type> {
  using policy_type = transparent_type;
  static constexpr bool enabled = true;
  static constexpr category kind = category::type;
};

template <> struct traits<throw_error> {
  using policy_type = throw_error;
  static constexpr bool enabled = true;
  static constexpr category kind = category::error;
};

template <> struct traits<expected_error> {
  using policy_type = expected_error;
  static constexpr bool enabled = true;
  static constexpr category kind = category::error;
};

template <> struct traits<terminate_error> {
  using policy_type = terminate_error;
  static constexpr bool enabled = true;
  static constexpr category kind = category::error;
};

template <> struct traits<single_thread> {
  using policy_type = single_thread;
  static constexpr bool enabled = true;
  static constexpr category kind = category::concurrency;
};

template <> struct traits<atomic> {
  using policy_type = atomic;
  static constexpr bool enabled = true;
  static constexpr category kind = category::concurrency;
};

template <typename P>
concept policy_type = traits<P>::enabled;

template <typename P>
concept value_policy = policy_type<P> && (traits<P>::kind == category::value);

template <typename P>
concept type_policy = policy_type<P> && (traits<P>::kind == category::type);

template <typename P>
concept error_policy = policy_type<P> && (traits<P>::kind == category::error);

template <typename P>
concept concurrency_policy =
    policy_type<P> && (traits<P>::kind == category::concurrency);

} // namespace mcpplibs::primitives::policy
