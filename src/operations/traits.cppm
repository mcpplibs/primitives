export module mcpplibs.primitives.operations.traits;

export namespace mcpplibs::primitives::operations {

enum class dimension : unsigned char {
  unary = 1,
  binary = 2,
};

template <typename OpTag> struct traits {
  using op_tag = OpTag;

  // Operation metadata hooks (default-disabled).
  static constexpr bool enabled = false;
  static constexpr auto arity = static_cast<dimension>(0);
};

template <typename OpTag>
concept operation = traits<OpTag>::enabled;

template <typename OpTag>
concept unary_operation =
    operation<OpTag> && traits<OpTag>::arity == dimension::unary;

template <typename OpTag>
concept binary_operation =
    operation<OpTag> && traits<OpTag>::arity == dimension::binary;

} // namespace mcpplibs::primitives::operations