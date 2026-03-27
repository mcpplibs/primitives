/*
 * Example: ex10_custom_operation
 *
 * Purpose:
 * Demonstrate how to register new operation tags, declare operation traits,
 * and provide runtime op_binding specializations.
 *
 * Expected results:
 * - Custom operations Average, GreaterThan, and BitAnd dispatch successfully.
 * - For inputs 10 and 6, outputs are avg=8, gt=1, bitand=2.
 * - Program prints computed values and exits with code 0.
 */

#include <iostream>

import mcpplibs.primitives;
import mcpplibs.primitives.operations.invoker;

using namespace mcpplibs::primitives;

namespace demo_ops {
// Point 10 / Step 1: Define custom operation tags.
struct Average {};
struct GreaterThan {};
struct BitAnd {};
} // namespace demo_ops

// Point 10 / Step 2: Register operation traits for
// arithmetic/comparison/bitwise.
template <> struct mcpplibs::primitives::operations::traits<demo_ops::Average> {
  using op_tag = demo_ops::Average;
  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::arithmetic;
};

template <>
struct mcpplibs::primitives::operations::traits<demo_ops::GreaterThan> {
  using op_tag = demo_ops::GreaterThan;
  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::comparison;
};

template <> struct mcpplibs::primitives::operations::traits<demo_ops::BitAnd> {
  using op_tag = demo_ops::BitAnd;
  static constexpr bool enabled = true;
  static constexpr auto arity = dimension::binary;
  static constexpr auto capability_mask = capability::bitwise;
};

// Point 10 / Step 3: Provide runtime op_binding for each new operation.
// Complex point: these specializations plug directly into run_value dispatch.
template <typename CommonRep>
struct mcpplibs::primitives::operations::runtime::op_binding<
    demo_ops::Average, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    out.has_value = true;
    out.value = static_cast<CommonRep>((lhs + rhs) / 2);
    return out;
  }
};

template <typename CommonRep>
struct mcpplibs::primitives::operations::runtime::op_binding<
    demo_ops::GreaterThan, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    out.has_value = true;
    out.value = static_cast<CommonRep>(lhs > rhs);
    return out;
  }
};

template <typename CommonRep>
struct mcpplibs::primitives::operations::runtime::op_binding<
    demo_ops::BitAnd, policy::value::checked, CommonRep> {
  static constexpr bool enabled = true;

  static constexpr auto apply(CommonRep lhs, CommonRep rhs)
      -> policy::value::decision<CommonRep> {
    policy::value::decision<CommonRep> out{};
    out.has_value = true;
    out.value = static_cast<CommonRep>(lhs & rhs);
    return out;
  }
};

int main() {
  // Point 10 / Step 4: Invoke each custom operation through operations::apply.
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const a = value_t{10};
  auto const b = value_t{6};

  auto const avg = operations::apply<demo_ops::Average>(a, b);
  auto const gt = operations::apply<demo_ops::GreaterThan>(a, b);
  auto const band = operations::apply<demo_ops::BitAnd>(a, b);

  if (!avg.has_value() || !gt.has_value() || !band.has_value()) {
    std::cerr << "custom operation dispatch failed\n";
    return 1;
  }

  std::cout << "avg=" << avg->value() << ", gt=" << gt->value()
            << ", bitand=" << band->value() << '\n';
  return 0;
}
