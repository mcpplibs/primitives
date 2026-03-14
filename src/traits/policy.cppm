module;
#include <type_traits>

export module mcpplibs.primitive.traits.policy;

export namespace mcpplibs::primitive {

namespace policy {

enum class category { value, type, error, concurrency };

template <typename P> struct traits {
  static constexpr bool enabled = false;
  static constexpr category kind = static_cast<category>(-1);
};

struct checked_value {};
struct unchecked_value {};

struct strict_type {};
struct relaxed_type {};

struct throw_error {};
struct expected_error {};

struct single_thread {};
struct atomic {};

template <> struct traits<checked_value> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::value;
};

template <> struct traits<unchecked_value> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::value;
};

template <> struct traits<strict_type> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::type;
};

template <> struct traits<relaxed_type> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::type;
};

template <> struct traits<throw_error> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::error;
};

template <> struct traits<expected_error> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::error;
};

template <> struct traits<single_thread> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::concurrency;
};

template <> struct traits<atomic> {
  static constexpr bool enabled = true;
  static constexpr category kind = category::concurrency;
};

template <typename P>
concept policy_type = traits<P>::enabled;

struct default_policies {
  using value = unchecked_value;
  using type = relaxed_type;
  using error = throw_error;
  using concurrency = single_thread;
};

} // namespace policy

} // namespace mcpplibs::primitive
