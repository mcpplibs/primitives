# Installation and Build

## Prerequisites

- C++23-capable compiler
  - GCC >= 14
  - Clang >= 17
  - MSVC >= 19.34 (MSVC version >= 1934)
- Build tools
  - xmake (recommended)
  - or CMake + Ninja

## Build with xmake

Build the library:

```bash
xmake build mcpplibs-primitives
```

Run an example:

```bash
xmake run ex01_default_arithmetic
```

Run tests:

```bash
xmake run primitives_test
```

Compatibility alias:

```bash
xmake run basic
```

`basic` runs `ex01_default_arithmetic`.

## Build with CMake

Configure and build:

```bash
cmake -B build -G Ninja
cmake --build build --target mcpplibs-primitives
```

Build an example and tests:

```bash
cmake --build build --target ex01_default_arithmetic
cmake --build build --target basic_tests
ctest --test-dir build --output-on-failure
```

## Example Targets

Examples are available as independent targets:

- `ex01_default_arithmetic`
- `ex02_type_policy`
- `ex03_value_policy`
- `ex04_error_policy`
- `ex05_concurrency_policy`
- `ex06_custom_underlying`
- `ex07_custom_policy`
- `ex08_custom_operation`

## Common Build Issues

### 1) C++ standard or modules not enabled

Ensure C++23 is enabled and your compiler supports C++ modules.

### 2) Compiler version too old

Use the minimum versions listed above. For GCC/Clang/MSVC, older versions can fail early at configuration or module compilation.

### 3) Failing to find test dependencies

The test target depends on GoogleTest. Use xmake package resolution or ensure CMake can fetch/access the configured repository.

## Next

- Continue with [Quick Start](./quick-start.md)
- API details: [../../api/en/README.md](../../api/en/README.md)

