export module mcpplibs.primitives.operations.impl;

import mcpplibs.primitives.operations.traits;

export namespace mcpplibs::primitives::operations {

// Unary operations
struct Increment {};
struct Decrement {};
struct BitwiseNot {};
struct UnaryPlus {};
struct UnaryMinus {};

// Binary operations
struct Addition {};
struct Subtraction {};
struct Multiplication {};
struct Division {};
struct Modulus {};
struct LeftShift {};
struct RightShift {};
struct BitwiseAnd {};
struct BitwiseOr {};
struct BitwiseXor {};

// Comparison operations
struct Equal {};
struct NotEqual {};
struct ThreeWayCompare {};
struct LessThan {};
struct GreaterThan {};
struct LessThanOrEqual {};
struct GreaterThanOrEqual {};

template <> struct traits<Increment> {
  using op_tag = Increment;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::unary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<Decrement> {
  using op_tag = Decrement;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::unary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<BitwiseNot> {
  using op_tag = BitwiseNot;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::unary;
  static constexpr auto capability_mask = capability::bitwise;
};

template <> struct traits<UnaryPlus> {
  using op_tag = UnaryPlus;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::unary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<UnaryMinus> {
  using op_tag = UnaryMinus;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::unary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<Addition> {
  using op_tag = Addition;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<Subtraction> {
  using op_tag = Subtraction;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<Multiplication> {
  using op_tag = Multiplication;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<Division> {
  using op_tag = Division;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<Modulus> {
  using op_tag = Modulus;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <> struct traits<LeftShift> {
  using op_tag = LeftShift;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::bitwise;
};

template <> struct traits<RightShift> {
  using op_tag = RightShift;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::bitwise;
};

template <> struct traits<BitwiseAnd> {
  using op_tag = BitwiseAnd;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::bitwise;
};

template <> struct traits<BitwiseOr> {
  using op_tag = BitwiseOr;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::bitwise;
};

template <> struct traits<BitwiseXor> {
  using op_tag = BitwiseXor;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::bitwise;
};

template <> struct traits<Equal> {
  using op_tag = Equal;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

template <> struct traits<NotEqual> {
  using op_tag = NotEqual;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

template <> struct traits<ThreeWayCompare> {
  using op_tag = ThreeWayCompare;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

template <> struct traits<LessThan> {
  using op_tag = LessThan;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

template <> struct traits<GreaterThan> {
  using op_tag = GreaterThan;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

template <> struct traits<LessThanOrEqual> {
  using op_tag = LessThanOrEqual;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

template <> struct traits<GreaterThanOrEqual> {
  using op_tag = GreaterThanOrEqual;

  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

} // namespace mcpplibs::primitives::operations
