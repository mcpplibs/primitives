export module mcpplibs.primitives.operations.impl;

import mcpplibs.primitives.operations.traits;

export namespace mcpplibs::primitives::operations {

struct Addition {};
struct Subtraction {};
struct Multiplication {};
struct Division {};
struct Equal {};
struct NotEqual {};

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

} // namespace mcpplibs::primitives::operations