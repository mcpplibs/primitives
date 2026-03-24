module;

#include <expected>
#include <optional>

export module mcpplibs.primitives.conversion.traits;

export namespace mcpplibs::primitives::conversion::risk {

enum class kind : unsigned char {
  none = 0,
  overflow,
  underflow,
  domain_error,
  precision_loss,
  sign_loss,
  invalid_type_combination,
};

template <typename Value>
struct traits {
  kind kind = kind::none;
  std::optional<Value> source_value{};
  std::optional<Value> converted_value{};
};

} // namespace mcpplibs::primitives::conversion::risk

export namespace mcpplibs::primitives::conversion {

template <typename Value>
using cast_result = std::expected<Value, risk::kind>;

} // namespace mcpplibs::primitives::conversion
