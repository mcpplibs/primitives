- #50 

--------

## 摘要

本文提议将 [Original-org/original](https://github.com/Original-org/original) 中的`original.basic.number`模块抽象并在MCPP社区内拓展一个“底层强类型原语封装库”。

第一阶段聚焦：

- `Integer<T>`
- `Floating<T>`
- `Boolean`
- `Char<T>`

该库定位为上层容器、算法、序列化、并发组件的基础数值与原语类型层。

## 动机与目标

当前 `original.basic.number` 已具备以下基础：

- `Integer<T>` / `Floating<T>` 封装；
- 强约束的算术与比较接口；
- checked 数值操作路径（溢出、除零等）；
- traits / casts / literals / hash 的模块拆分。

社区化阶段需要：

1. 形成策略化扩展机制（而不是硬编码行为）；
2. 保持与不同工程约束兼容（异常开关、并发模型、性能偏好）。

## 目标

- 提供统一的强类型 primitive 抽象（`Integer/Floating/Boolean/Char`）；
- 默认安全（checked + strict type）；
- 支持策略组合（值安全、类型安全、错误处理类型、并发模式）；
- C++20 Modules 优先，同时为生态兼容预留扩展空间。

## 设计原则

1. 默认组合策略优先安全，在此基础上保证性能。
2. 行为策略可替换、可组合。
3. 禁止强类型之间的危险转换。
4. 零开销抽象，未启用策略不额外付费，`constexpr`友好。
5. 允许渐进迁移。

## 安全策略体系提案

### 值安全

- `checked_value_policy`（默认）
- `unchecked_value_policy`
- `saturating_value_policy`

关注点：overflow、div0、shift 越界、浮点特殊值处理。
 
### 类型安全

- `strict_type_policy`（默认）
- `relaxed_type_policy`

约束建议：

- 禁止隐式窄化；
- 禁止 signed/unsigned 自动混算；
- `Boolean` 禁止数值算术；
- `Char` 默认不参与数值算术。

### 错误模型 / 异常安全

- `throw_error_policy`（默认）
- `expected_error_policy`
- `terminate_error_policy`

建议提供双通道 API：

- `op()`：遵循 ErrorPolicy
- `try_op()`：返回结果对象（无异常路径）

### 并发安全

- `single_thread_policy`（默认）
- `atomic_policy`
- `locked_policy<Mutex>`

原则：并发安全应显式 opt-in，不应对全部 primitive 默认加锁/原子化。

### 默认策略组合建议

- `Integer/Floating`：`checked + strict + throw + single_thread`
- `Boolean/Char`：`strict + throw + single_thread`（且禁数值算术）

该默认组合兼顾直觉、安全和迁移成本。

## 迭代路线（参考）

### 第一阶段

- 稳定 `Integer/Floating/Boolean/Char` API
- 完成 traits/casts/literals/hash 基础设施
- 暴露默认策略组合（内部策略槽位先预留）

### 第二阶段

- 引入 `expected_error_policy`
- 引入 `unchecked/saturating`
- 完善行为矩阵文档

### 第三阶段

- 引入 `atomic_policy`
- 引入语义标签桥接（非强绑定）

### 第四阶段

- 序列化适配（JSON/MsgPack/Proto）
- DTO/数据库边界转换
- 更完整的 ABI 与模块导出策略文档

## 额外补充

以此作为基础，以后可以编写抽象层级更高的**通用单位系统**库（具有数量+量纲+维度组合，并且可以自然拓展各种单位的强类型）

## 参考资料

- https://github.com/Original-org/original
- https://github.com/foonathan/type_safe
- https://github.com/boostorg/safe_numerics
- https://github.com/mpusz/mp-units