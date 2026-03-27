# mcpplibs primitives

> C++23 modular primitives library - `import mcpplibs.primitives;`

[![d2x](https://img.shields.io/badge/d2x-ok-green.svg)](https://github.com/d2learn/d2x)
[![Online-ebook](https://img.shields.io/badge/online-ebook-orange.svg)](https://github.com/d2learn/d2x)
[![License](https://img.shields.io/badge/license-Apache_2.0-blue.svg)](LICENSE-CODE)

| [中文](README.zh.md) - [English](README.md) - [Forum](https://mcpp.d2learn.org/forum) |
|---------------------------------------------------------------------------------|
| [用户文档](docs/guide/zh/README.md) - [User Documentation](docs/guide/en/README.md) |
| [API文档](docs/api/zh/README.md) - [API Documentation](docs/api/en/README.md)  |

This repository provides configurable `primitive` infrastructure (`underlying traits`, `policy`, and `operations/dispatcher`) to unify numeric behavior, error handling, and concurrency access semantics.

> [!WARNING]
> The project is still evolving quickly, and APIs may change.

## Features

- **C++23 modules** — `import mcpplibs.primitives;`
- **Dual build systems** — both xmake and CMake are supported
- **Policy-driven behavior** — value/type/error/concurrency policies are composable
- **Mixed operations support** — binary operations between `primitive` and `underlying` are supported
- **Concurrency access APIs** — `primitive::load/store/compare_exchange`

## Operators

The library provides unary, arithmetic, bitwise, and comparison operations for `primitive`.  
Arithmetic paths are dispatched through a unified pipeline and return `std::expected<..., policy::error::kind>`.

- Value policies (`policy::value::checked` / `policy::value::saturating` / `policy::value::unchecked`) define overflow behavior.
- Error policies (`policy::error::throwing` / `policy::error::expected` / `policy::error::terminate`) define how errors are propagated.

Example:

```cpp
import std;
import mcpplibs.primitives;

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::operators;

primitive<int> a{1};
primitive<int> b{2};
auto sum = a + b; // std::expected<primitive<int>, policy::error::kind>

using checked_t =
    primitive<int, policy::value::checked, policy::error::expected>;
auto maybe_overflow =
    checked_t{std::numeric_limits<int>::max()} + checked_t{1};
```

## Policy Protocol Namespaces

When implementing custom policies, protocol entry points are split by responsibility:

- `policy::type::handler` / `policy::type::handler_available`
- `policy::concurrency::handler` / `policy::concurrency::injection`
- `policy::value::handler` / `policy::value::decision`
- `policy::error::handler` / `policy::error::request` / `policy::error::kind`

Built-in policy tags:

- `policy::value::{checked, unchecked, saturating}`
- `policy::type::{strict, compatible, transparent}`
- `policy::error::{throwing, expected, terminate}`
- `policy::concurrency::{none, fenced, fenced_relaxed, fenced_acq_rel, fenced_seq_cst}`

Concurrency notes:

- `fenced*` policies provide operation-level concurrency semantics with injected memory-order fences.
- `primitive` storage keeps a uniform, zero-extra-storage abstraction.
- `primitive::load/store/compare_exchange` are provided by concurrency policy protocols and fail at compile time if unsupported.

Example (concurrent access APIs):

```cpp
using shared_t = primitive<int, policy::value::checked,
                           policy::concurrency::fenced_acq_rel,
                           policy::error::expected>;

shared_t v{1};
v.store(2);
auto expected = 2;
if (v.compare_exchange(expected, 3)) {
  auto now = v.load();
  (void)now;
}
```

Default policies are available under `policy::defaults`:

- `policy::defaults::value`
- `policy::defaults::type`
- `policy::defaults::error`
- `policy::defaults::concurrency`

## Examples

- `ex01_basic_usage`: Literals + primitive factory helpers with the built-in operator set.
- `ex02_type_policy`: Type negotiation with `strict/compatible`, including how type policy affects construction from `underlying`.
- `ex03_value_policy`: `checked/unchecked/saturating` behavior, including mixed binary operations with `underlying`.
- `ex04_error_policy`: Error-handling behavior across different error policies.
- `ex05_concurrency_policy`: Representative mixed read/write concurrency workload (writer `store` + reader `add/sub` + `CAS`).
- `ex06_conversion`: Checked/saturating/truncating/exact conversion helpers across underlying values and primitives.
- `ex07_algorithms`: Limits metadata, special numeric values, and hashing helpers.
- `ex08_custom_underlying`: Custom underlying traits, rep validation, and common-rep extension.
- `ex09_custom_policy`: Custom policy protocol implementation.
- `ex10_custom_operation`: Custom operation extension.

## Project Layout

```
mcpplibs-primitives/
├── src/                        # module sources
│   ├── primitives.cppm         # top-level aggregate module
│   ├── primitive/              # primitive definitions and traits
│   ├── policy/                 # policy tags and protocol implementations
│   ├── operations/             # operation tags / dispatcher / operators
│   └── underlying/             # underlying traits and common_rep
├── examples/                   # example programs
├── tests/                      # test entry and basic test suite
├── xmake.lua                   # xmake build script
├── CMakeLists.txt              # CMake build script
└── .xlings.json                # xlings package descriptor
```

## Quick Start

```cpp
import std;
import mcpplibs.primitives;

int main() {
  using namespace mcpplibs::primitives;

  using value_t = primitive<int, policy::error::expected>;
  auto const result = operations::add(value_t{40}, value_t{2});
  return (result.has_value() && result->value() == 42) ? 0 : 1;
}
```

## Installation and Setup

```bash
xlings install
```

## Build and Run

**Using xmake**

```bash
xmake build mcpplibs-primitives
xmake run basic                    # compatibility alias for ex01_basic_usage
xmake run ex01_basic_usage
xmake run ex06_conversion
xmake run ex07_algorithms
xmake run ex05_concurrency_policy
xmake run primitives_test
```

**Using CMake** (`CMake >= 3.31`)

```bash
cmake -B build -G Ninja
cmake --build build --target mcpplibs-primitives
cmake --build build --target ex01_basic_usage
cmake --build build --target ex06_conversion
cmake --build build --target ex07_algorithms
cmake --build build --target basic_tests
ctest --test-dir build --output-on-failure
```

## Build System Integration

### xmake

```lua
add_repositories("mcpplibs-index https://github.com/mcpplibs/mcpplibs-index.git")

add_requires("primitives")

target("myapp")
    set_kind("binary")
    set_languages("c++23")
    add_files("main.cpp")
    add_packages("primitives")
    set_policy("build.c++.modules", true)
```

## Related Links

- [mcpp-style-ref | Modern C++ coding and project style reference](https://github.com/mcpp-community/mcpp-style-ref)
- [d2mystl | Build a mini STL from scratch](https://github.com/mcpp-community/d2mystl)
- [mcpp community website](https://mcpp.d2learn.org)
- [mcpp forum](https://mcpp.d2learn.org/forum)
- [Getting Started: Learn Modern C++ by Building](https://github.com/Sunrisepeak/mcpp-standard)
