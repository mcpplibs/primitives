# Concurrency Guide

## Concurrency Policies

Built-in concurrency policies:

- `policy::concurrency::none`
- `policy::concurrency::fenced`
- `policy::concurrency::fenced_relaxed`
- `policy::concurrency::fenced_acq_rel`
- `policy::concurrency::fenced_seq_cst`

General guidance:

- Use `none` for single-threaded paths.
- Use `fenced` or `fenced_seq_cst` first for simple correctness.
- Use relaxed/acq_rel variants only when you have clear memory-order requirements.

## Shared State with `primitive`

```cpp
using shared_t = mcpplibs::primitives::primitive<
    int,
    mcpplibs::primitives::policy::value::checked,
    mcpplibs::primitives::policy::concurrency::fenced,
    mcpplibs::primitives::policy::error::expected>;

shared_t counter{0};
```

Use:

- `counter.load()`
- `counter.store(v)`
- `counter.compare_exchange(expected, desired)`

## CAS Loop Pattern

```cpp
auto expected = counter.load();
while (!counter.compare_exchange(expected, expected + 1)) {
}
```

This is the typical lock-free increment style used in multi-threaded updates.

## Mixed Read/Write Workload

For practical concurrent workloads, combine:

- writer threads updating operands through `store`
- reader threads calling `operations::add` / `operations::sub`
- optional sink updates via `store` and checkpoint `compare_exchange`

See `examples/ex05_concurrency_policy.cpp` for a representative end-to-end workload.

## Pitfalls

### 1) Atomic-ref compatibility

Fenced access handlers require underlying representations to satisfy atomic-ref constraints (trivially copyable and alignment-compatible).

### 2) Treating operations as implicit synchronization

Operation dispatch consistency and concurrency fences are policy-driven. Do not assume synchronization semantics unless explicitly configured.

### 3) Ignoring error paths

Operation results are `std::expected<...>`. Always handle `has_value()`.

## Next

- Extension guide: [./extension.md](./extension.md)
- API concurrency details: [../../api/en/README.md](../../api/en/README.md)

