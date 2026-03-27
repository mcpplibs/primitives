# primitives API Documentation

> Module entry: `import mcpplibs.primitives;`

## Module Entry

Top-level import:

```cpp
import mcpplibs.primitives;
```

This module exports:

- `mcpplibs.primitives.underlying`
- `mcpplibs.primitives.policy`
- `mcpplibs.primitives.primitive`
- `mcpplibs.primitives.operations`
- `mcpplibs.primitives.algorithms`
- `mcpplibs.primitives.conversion`

## Namespace Overview

- `mcpplibs::primitives`: core types, concepts, and `primitive`.
- `mcpplibs::primitives::underlying`: underlying traits and categories.
- `mcpplibs::primitives::policy`: policy tags, defaults, and protocols.
- `mcpplibs::primitives::operations`: functional dispatch API.
- `mcpplibs::primitives::conversion`: numeric risk checks and cast helpers.
- `mcpplibs::primitives::algorithms`: limits and hash helpers.
- `mcpplibs::primitives::operators`: operator overload entry.
- `mcpplibs::primitives::literals`: checked underlying literal suffixes.
- `mcpplibs::primitives::meta`: primitive metadata traits.
- `mcpplibs::primitives::types`: convenience aliases for common underlying types.

## Core Concepts and Metadata

### underlying concepts

- `underlying_type<T>`
- `boolean_underlying_type<T>`
- `character_underlying_type<T>`
- `integer_underlying_type<T>`
- `floating_underlying_type<T>`
- `numeric_underlying_type<T>`
- `has_common_rep<LhsRep, RhsRep>`
- `common_rep_t<LhsRep, RhsRep>`

### policy concepts

- `policy::policy_type<P>`
- `policy::value_policy<P>`
- `policy::type_policy<P>`
- `policy::error_policy<P>`
- `policy::concurrency_policy<P>`
- `policy::resolve_policy_t<category, Policies...>`

### primitive metadata

- `meta::traits<primitive<...>>`  
  exposes `value_type / policies / value_policy / type_policy / error_policy / concurrency_policy`
- `meta::make_primitive_t<T, PoliciesTuple>`

## `primitive<T, Policies...>`

```cpp
template <underlying_type T, policy::policy_type... Policies>
class primitive;
```

### Member types

- `value_type = T`
- `policies = std::tuple<Policies...>`

### Construction and assignment

- `explicit primitive(T)`: same-underlying construction.
- `explicit primitive(U)`: cross-underlying construction (subject to type policy).
- Copy/move construction and assignment are supported for the same policy set.
- Cross-underlying primitive construction/assignment is supported when allowed by type policy.

### Value and concurrency access APIs

- `value()` / `value() const`
- `explicit operator value_type() const`
- `load() -> value_type`
- `store(desired)`
- `compare_exchange(expected, desired) -> bool`

Notes:

- `store/compare_exchange` always support same-underlying types.
- Cross-underlying `store/compare_exchange` are available only when type policy allows.
- Access APIs are provided by `policy::concurrency::handler<..., void, ...>`.

### Type policy impact on cross-underlying behavior

- `policy::type::strict`: only identical types are allowed.
- `policy::type::compatible`: requires same underlying category and a valid `common_rep`.
- `policy::type::transparent`: only requires a valid `common_rep`.

### Convenience aliases (`types`)

- Integers: `U8/U16/U32/U64`, `Size`, `Diff`, `I8/I16/I32/I64`
- Floating-point: `F32/F64/F80`
- Bool/chars: `Bool/UChar/Char8/Char16/Char32/WChar`

Example:

```cpp
using value_t = mcpplibs::primitives::types::I32<
    mcpplibs::primitives::policy::value::checked,
    mcpplibs::primitives::policy::error::expected>;
```

### Factory helpers and literals

- `with(value)` builds a `primitive` with default policies from an underlying value.
- `with<Policies...>(value)` applies an explicit policy set during construction.
- `with(policies_tuple, value)` reuses an existing policies tuple.
- `mcpplibs::primitives::literals` provides checked suffixes such as
  `_u8/_u16/_u32/_u64/_size/_diff`, `_i8/_i16/_i32/_i64`,
  `_f32/_f32e/_f64/_f64e/_f80/_f80e`, and character suffixes
  `_uchar/_char8/_char16/_char32/_wchar`.

Example:

```cpp
using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::literals;

auto value = with<policy::value::checked, policy::error::expected>(42_i32);
```

## `operations` API

The `operations` namespace provides function-style APIs with unified `std::expected` results.

### Common result aliases

- `primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload>`
- `mixed_primitive_dispatch_result_t<OpTag, Primitive, Underlying, ErrorPayload>`
- `three_way_dispatch_result_t<Lhs, Rhs, ErrorPayload>`

### Unary operations

- `increment`
- `decrement`
- `bit_not`
- `unary_plus`
- `unary_minus`

### Binary arithmetic

- `add`
- `sub`
- `mul`
- `div`
- `mod`

### Binary bitwise

- `shift_left`
- `shift_right`
- `bit_and`
- `bit_or`
- `bit_xor`

### Comparisons

- `equal`
- `not_equal`
- `three_way_compare`

### Compound assignments

- `add_assign/sub_assign/mul_assign/div_assign/mod_assign`
- `shift_left_assign/shift_right_assign`
- `bit_and_assign/bit_or_assign/bit_xor_assign`

### Mixed operand support

Most binary operations support:

- `primitive op primitive`
- `primitive op underlying`
- `underlying op primitive`

## `operators` overloads

To use `+ - * / % ...` syntax, import:

```cpp
using namespace mcpplibs::primitives::operators;
```

Operator results are still `std::expected<...>`, not raw values.

## `conversion` API

The `conversion` namespace provides reusable casts for underlying values,
primitives, and mixed source/destination pairs.

### Common result aliases and risk model

- `cast_result<T> = std::expected<T, conversion::risk::kind>`
- `conversion::risk::kind`:
  `none`, `overflow`, `underflow`, `domain_error`, `precision_loss`,
  `sign_loss`, `invalid_type_combination`

### Conversion helpers

- `numeric_risk<Dest>(value)`
- `unchecked_cast<Dest>(value)`
- `checked_cast<Dest>(value)`
- `saturating_cast<Dest>(value)`
- `truncating_cast<Dest>(value)`
- `exact_cast<Dest>(value)`

Notes:

- `checked_cast` and `exact_cast` return `cast_result<T>`.
- `saturating_cast` clamps overflow/underflow for numeric destinations.
- `truncating_cast` is useful for floating-to-integral conversions.
- Overloads exist for underlying -> underlying, primitive -> primitive,
  primitive -> underlying, and underlying -> primitive paths.

## `algorithms` API

The `algorithms` namespace provides reusable limits metadata and hashing helpers
for built-in and supported custom underlyings.

### Limits

- `limits<T>`
- `limited_type<T>`
- `min_value<T>()`
- `lowest_value<T>()`
- `max_value<T>()`
- `epsilon_value<T>()`
- `infinity_value<T>()`
- `quiet_nan_value<T>()`

### Hashing

- `hash<T>`
- `hash_result_t<T>`
- `hashable<T>`
- `hash_value(value)`

## policy API

### Built-in policy tags

- Value: `policy::value::{checked, unchecked, saturating}`
- Type: `policy::type::{strict, compatible, transparent}`
- Error: `policy::error::{throwing, expected, terminate}`
- Concurrency: `policy::concurrency::{none, fenced, fenced_relaxed, fenced_acq_rel, fenced_seq_cst}`

### Defaults

- `policy::defaults::value = policy::value::checked`
- `policy::defaults::type = policy::type::strict`
- `policy::defaults::error = policy::error::throwing`
- `policy::defaults::concurrency = policy::concurrency::none`

### Error kinds

`policy::error::kind`:

- `none`
- `invalid_type_combination`
- `overflow`
- `underflow`
- `divide_by_zero`
- `domain_error`
- `unspecified`

## underlying Extension Points

### 1) Register `underlying::traits<T>`

Required members:

- `value_type`
- `rep_type`
- `enabled`
- `kind`
- `to_rep(value)`
- `from_rep(rep)`
- `is_valid_rep(rep)`

### 2) Customize common-rep negotiation

Specialize `underlying::common_rep_traits<LhsRep, RhsRep>` to override the default `std::common_type_t` behavior.

## Result and Error Model

- Most operations/operators APIs return `std::expected<...>`.
- With `policy::error::expected`, failures are returned as `unexpected(policy::error::kind)`.
- With `policy::error::throwing`, runtime failures throw exceptions (while API signatures still use `std::expected`).

## Minimal Example

```cpp
import std;
import mcpplibs.primitives;

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::operators;

int main() {
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const out = value_t{40} + value_t{2};
  if (!out.has_value()) {
    return 1;
  }
  return out->value() == 42 ? 0 : 1;
}
```
