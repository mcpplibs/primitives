# Quick Start

## Minimal Program

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

Key point: arithmetic and operator APIs typically return `std::expected<...>`.

## First Policy Setup

Recommended starting combination:

- `policy::value::checked`
- `policy::error::expected`
- default `type::strict`
- default `concurrency::none`

This gives safe arithmetic and explicit error handling without requiring exceptions.

## Learn by Examples

Run examples in this order:

1. `ex01_default_arithmetic`
2. `ex02_type_policy`
3. `ex03_value_policy`
4. `ex04_error_policy`
5. `ex05_concurrency_policy`
6. `ex06_custom_underlying`
7. `ex07_custom_policy`
8. `ex08_custom_operation`

Command:

```bash
xmake run ex03_value_policy
```

## Common Usage Snippets

### Default primitive

```cpp
using default_i32 = mcpplibs::primitives::primitive<int>;
```

### Mixed primitive/underlying operation

```cpp
using value_t = mcpplibs::primitives::primitive<
    int,
    mcpplibs::primitives::policy::type::compatible,
    mcpplibs::primitives::policy::error::expected>;

auto const lhs = value_t{40};
short const rhs = 2;
auto const out = mcpplibs::primitives::operations::add(lhs, rhs);
```

### Operators namespace

```cpp
using namespace mcpplibs::primitives::operators;
```

Without this namespace, call function APIs via `mcpplibs::primitives::operations`.

## Next

- Concurrency usage: [./concurrency.md](./concurrency.md)
- Extension guide: [./extension.md](./extension.md)
- API reference: [../../api/en/README.md](../../api/en/README.md)

