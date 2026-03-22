# 并发使用指南

## 并发策略

内置并发策略：

- `policy::concurrency::none`
- `policy::concurrency::fenced`
- `policy::concurrency::fenced_relaxed`
- `policy::concurrency::fenced_acq_rel`
- `policy::concurrency::fenced_seq_cst`

选择建议：

- 单线程路径使用 `none`。
- 若优先正确性，先用 `fenced` 或 `fenced_seq_cst`。
- 仅在明确内存序需求时使用 `relaxed/acq_rel` 变体。

## 使用 `primitive` 管理共享状态

```cpp
using shared_t = mcpplibs::primitives::primitive<
    int,
    mcpplibs::primitives::policy::value::checked,
    mcpplibs::primitives::policy::concurrency::fenced,
    mcpplibs::primitives::policy::error::expected>;

shared_t counter{0};
```

常用接口：

- `counter.load()`
- `counter.store(v)`
- `counter.compare_exchange(expected, desired)`

## CAS 循环模式

```cpp
auto expected = counter.load();
while (!counter.compare_exchange(expected, expected + 1)) {
}
```

这是多线程下常见的无锁自增写法。

## 读写组合并发场景

建议组合使用：

- writer 线程通过 `store` 更新操作数
- reader 线程调用 `operations::add` / `operations::sub`
- 可选地将结果写入 sink，并用 `compare_exchange` 做检查点更新

可参考：`examples/ex05_concurrency_policy.cpp`

## 常见问题

### 1) atomic_ref 约束不满足

fenced 访问路径依赖 atomic-ref，表示类型需要满足可平凡复制与对齐约束。

### 2) 误把运算调用当作隐式同步

分发一致性与并发内存序都由策略决定，未配置前不要假设有同步语义。

### 3) 忽略错误分支

运算结果是 `std::expected<...>`，请始终检查 `has_value()`。

## 下一步

- 扩展开发： [./extension.md](./extension.md)
- API 并发细节： [../../api/zh/README.md](../../api/zh/README.md)

