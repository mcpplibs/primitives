export module mcpplibs.primitives.operations.traits;

export namespace mcpplibs::primitives::operations {

enum class dimension : unsigned char {
  unary = 1,
  binary = 2,
};

enum class capability : unsigned char {
  none = 0,
  arithmetic = 1 << 0,
  comparison = 1 << 1,
  bitwise = 1 << 2,
};

constexpr auto has_capability(capability mask, capability cap) noexcept
    -> bool {
  return (static_cast<unsigned char>(mask) & static_cast<unsigned char>(cap)) !=
         0;
}

template <typename OpTag> struct traits {
  using op_tag = OpTag;

  // Operation metadata (default-disabled).
  // capability_mask is the single source of truth for capability declaration.
  static constexpr bool enabled = false;
  static constexpr auto arity = static_cast<dimension>(0);
  static constexpr auto capability_mask = capability::none;
};

template <typename OpTag>
concept operation = traits<OpTag>::enabled;

template <typename OpTag>
concept unary_operation =
    operation<OpTag> && traits<OpTag>::arity == dimension::unary;

template <typename OpTag>
concept binary_operation =
    operation<OpTag> && traits<OpTag>::arity == dimension::binary;

template <typename OpTag, capability Cap>
inline constexpr bool op_has_capability_v =
    operation<OpTag> && has_capability(traits<OpTag>::capability_mask, Cap);

template <typename OpTag>
concept arithmetic_operation =
    op_has_capability_v<OpTag, capability::arithmetic>;

template <typename OpTag>
concept comparison_operation =
    op_has_capability_v<OpTag, capability::comparison>;

template <typename OpTag>
concept bitwise_operation = op_has_capability_v<OpTag, capability::bitwise>;

// Validates that an enabled operation has a non-none capability_mask declared.
template <typename OpTag>
inline constexpr bool op_capability_valid_v =
    !operation<OpTag> || traits<OpTag>::capability_mask != capability::none;

} // namespace mcpplibs::primitives::operations