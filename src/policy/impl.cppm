module;

export module mcpplibs.primitives.policy.impl;

import mcpplibs.primitives.policy.traits;

export namespace mcpplibs::primitives::policy {

template <> struct traits<checked_value> {
  using policy_type = checked_value;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<unchecked_value> {
  using policy_type = unchecked_value;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<saturating_value> {
  using policy_type = saturating_value;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::value;
};

template <> struct traits<strict_type> {
  using policy_type = strict_type;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<category_compatible_type> {
  using policy_type = category_compatible_type;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<transparent_type> {
  using policy_type = transparent_type;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::type;
};

template <> struct traits<throw_error> {
  using policy_type = throw_error;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<expected_error> {
  using policy_type = expected_error;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<terminate_error> {
  using policy_type = terminate_error;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::error;
};

template <> struct traits<single_thread> {
  using policy_type = single_thread;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
};

template <> struct traits<atomic> {
  using policy_type = atomic;
  static constexpr bool enabled = true;
  static constexpr auto kind = category::concurrency;
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
