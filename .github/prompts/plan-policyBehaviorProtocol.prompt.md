## Plan: Policy 行为协议与分发规范

基于现有 traits/concept 架构，采用“policy handler 协议 + 固定 dispatcher + primitive 路由层”的设计。type 协商前移到编译期静态决议，运行期 dispatcher 固定为三层并按外到内调用：concurrency -> value（已知 common type）-> error。第一阶段覆盖四则运算（加减乘除），错误通道统一为 expected 返回；跨底层类型运算由 type policy 在编译期决定“是否允许 + common type”；value 层负责溢出判定与值调整/错误下放且需要接收 concurrency 层注入；error 层按策略执行最终错误处理。用户扩展边界限定为仅可特化 policy handler，以保持上层接口可复用、低耦合且强约束。

**Steps**
1. 明确协议核心抽象（阻塞后续步骤）
2. 在 policy 层定义行为协议接口（依赖步骤 1）
3. 在 operations 层建立统一分发入口（依赖步骤 2）
4. 在 primitive 层实现运算路由与返回类型统一（依赖步骤 3）
5. 为跨类型运算定义 type policy 协商规则（依赖步骤 4，部分可与步骤 6 并行）
6. 建立编译期约束与诊断信息（依赖步骤 2，可并行于步骤 5）
7. 增加测试矩阵与示例验证（依赖步骤 4、5、6）
8. 文档化协议与扩展边界（依赖步骤 7）

**Phase 1: 协议基线（接口层）**
1. 在 [src/policy/traits.cppm](src/policy/traits.cppm) 复用 category 与 policy_type 概念，新增“行为协议概念”所需的基础类型约束（仅声明契约，不放具体行为）。
2. 在 [src/policy/impl.cppm](src/policy/impl.cppm) 保留现有标签与 traits 特化，补充“策略能力声明位”对应的 traits 扩展位（用于编译期判定策略是否实现某 operation）。
3. 新增 policy 协议模块（建议命名 mcpplibs.primitives.policy.handler），内容包含：
4. `policy_handler<Policy, OperationTag, Lhs, Rhs>` 主模板（默认禁用）
5. `policy_handler_available` 概念（用于约束上层调用）
6. 标准返回别名（统一为 expected 语义）
7. 明确用户扩展点：只允许特化 handler，不开放分发规则特化。

**Phase 2: Operation 分发层（固定三层运行期链路）**
1. 在 [src/operations/impl.cppm](src/operations/impl.cppm) 维持 operation tag + arity 元数据，增加“可分发 operation”约束入口（例如合法 operation 概念）。
2. 在 [src/operations/operators.cppm](src/operations/operators.cppm) 将 type 协商前移为编译期预处理，并实现运行期固定 dispatcher：
3. concurrency handler：负责并发包装（如加锁/解锁、内存屏障注入；具体机制可后续细化），并向 value 层注入并发执行上下文。
4. value handler：接收已决议的 common type + concurrency 注入上下文，在 common type 上执行值域与溢出判定，决定“本层修正结果值”或“下放错误处理请求”。
5. error handler：接收上层错误请求并按策略完成最终处理，统一映射到 expected 错误值。
6. dispatcher 对外统一返回 expected，且不依赖具体 primitive 别名，仅依赖 traits 协议。

**Phase 3: Primitive 路由层（上层调用面）**
1. 在 [src/primitive/traits.cppm](src/primitive/traits.cppm) 复用 primitive_traits 与 resolve_policy_t，新增：
2. 跨类型运算协商 trait（由 type policy 决定是否允许、结果 common type）
3. 结果 primitive 重建工具（保持策略传播一致）
4. 在 primitive 对外运算入口中接入 operations dispatcher，覆盖加减乘除四个操作。
5. 返回类型统一为 expected（成功值为 primitive，失败值为错误域类型）。

**Phase 4: 聚合导出与契约稳定**
1. 在 [src/primitives.cppm](src/primitives.cppm) 增加新协议模块与分发层导出，保持聚合入口一致。
2. 在 [src/policy/utility.cppm](src/policy/utility.cppm) 保持 common policy 选择逻辑不变，仅补充与 handler 可用性相关的辅助别名（若需要）。

**Phase 5: 验证与回归**
1. 在 [tests](tests) 新增 policy 行为协议测试：
2. handler 未实现时应在编译期失败（约束强）
3. 四则运算均返回 expected
4. 相同 type policy 下跨类型协商正确/错误路径正确
5. 在 [examples/basic.cpp](examples/basic.cpp) 增加最小示例：
6. 默认策略下四则运算
7. 自定义 policy handler 的单点扩展示例（仅特化 handler）


**Interface Contracts（最小签名清单）**
1. Dispatcher 总入口（放在 [src/operations/operators.cppm](src/operations/operators.cppm)）：
2. 输入：OperationTag、Lhs Primitive、Rhs Primitive、四类 policy（由 primitive_traits 解析）。
3. 输出：expected<ResultPrimitive, ErrorPayload>。
4. 约束：type 协商在编译期完成；运行期调用顺序固定为 concurrency -> value -> error。
5. 编译期 type 协商合约（静态层，不进入运行期链路）：
6. 输入：操作标签、Lhs/Rhs 静态类型信息、type policy。
7. 输出：type_decision（is_allowed、common_type、diagnostic_id）。
8. 责任：拒绝非法运算并确定 common_type；其结果作为运行期 value 层输入前提。
9. concurrency handler 合约（放在 policy handler 模块）：
10. 输入：operation_context、下一层 continuation、type_decision。
11. 输出：concurrency_injection + 下一层 expected 透传能力。
12. 责任：加锁/解锁、内存屏障、线程可见性包装；向 value 层注入并发执行上下文，不做值域判定。
13. value handler 合约：
14. 输入：common_type 下的 lhs/rhs、operation_context、concurrency_injection。
15. 输出：二选一路径：
16. 路径 A：直接给出成功值（已修正或原值）。
17. 路径 B：下发 error_request（包含错误类别、上下文、候选回退值）。
18. 责任：执行溢出/下溢/除零等值域检查，并决定是否本层修正。
19. error handler 合约：
20. 输入：error_request + operation_context。
21. 输出：expected<ResultPrimitive, ErrorPayload> 的最终错误分支或恢复成功值分支。
22. 责任：按 error policy 落地处理（例如映射错误域、终止、抛错转译为 expected 错误值）。
23. 跨层数据结构建议：
24. operation_context：封装 op tag、源 primitive traits、policy 句柄、调试标签。
25. type_decision：封装 is_allowed、common_type、diagnostic_id。
26. concurrency_injection：封装 guard_handle、memory_order/屏障策略、可选调度标签。
27. value_decision：封装 has_value、value、error_request。
28. error_request：封装 error_kind、reason、operation_context、可选 fallback。
29. 扩展边界：
30. 用户只可特化三类运行期 handler（concurrency/value/error）与静态 type 协商策略实现，不可替换 dispatcher 顺序、不可改动跨层数据结构主骨架。

**Relevant files**
- [src/policy/traits.cppm](src/policy/traits.cppm) — 复用并扩展 policy 概念边界，承载行为协议约束基础。
- [src/policy/impl.cppm](src/policy/impl.cppm) — 保持标签定义，补充策略能力元信息挂点。
- [src/policy/utility.cppm](src/policy/utility.cppm) — 与 common_policies 选择逻辑对齐，必要时提供 handler 检查辅助。
- [src/operations/impl.cppm](src/operations/impl.cppm) — operation 元数据与分发前置约束。
- [src/operations/operators.cppm](src/operations/operators.cppm) — dispatcher 核心实现位置。
- [src/primitive/traits.cppm](src/primitive/traits.cppm) — policy 解析与跨类型协商 trait 的主落点。
- [src/primitives.cppm](src/primitives.cppm) — 对外聚合导出。
- [tests](tests) — 编译期约束与行为结果验证。
- [examples/basic.cpp](examples/basic.cpp) — API 用法与扩展示例。

**Verification**
1. 编译期契约验证：
2. 未提供对应 policy_handler 的 operation 调用必须报清晰静态断言。
3. type policy 禁止的跨类型运算必须在编译期拒绝。
4. 固定链路验证：
5. type 协商必须在编译期完成，运行期 dispatcher 必须按 concurrency -> value -> error 顺序调用，禁止跳层与重排。
6. value 层必须验证“接收 concurrency 注入”这一前提生效。
7. value 层“值修正”与“错误下放”两条路径都需覆盖测试。
8. 行为一致性验证：
9. 四则运算对默认策略与自定义策略均返回 expected。
10. 失败路径（溢出/除零/不兼容类型）按 error policy 语义编码到 expected 错误值。
11. unchecked_value 路径验证：
12. 不触发 error policy，保持原生算术语义（含 UB 风险）并通过测试明确边界。
13. 导出稳定性验证：
14. 通过聚合模块导入可直接访问 policy 协议、operation 分发、primitive 运算。
15. 运行 tests 与示例构建，确认无回归。

**Decisions**
- 错误通道：默认路径统一返回 expected。
- dispatcher 形态：type 协商前移到编译期，运行期固定三层链路 concurrency -> value -> error，不开放顺序重排。
- 跨底层类型运算：由 type policy 在编译期决定可行性与 common type。
- value 层职责：判定溢出并决定“本层值修正”或“下放 error 层处理”。
- unchecked_value 语义：不做错误处理，不调用 error policy，行为尽量贴近原生 C/C++（包含 UB 风险）。
- 扩展边界：仅开放 policy handler 特化，不开放分发规则。
- 第一阶段覆盖范围：Addition/Subtraction/Multiplication/Division 全部纳入。
- In scope：policy 行为协议、operation 分发、primitive 路由、测试与文档。
- Out of scope：并发策略的运行时同步原语实现细节（atomic/lock-free/锁策略具体执行体）。

## Underlying Bridge Execution Contract (Runtime)

1. `dispatch` 在 value 阶段前统一执行 `to_rep`，将原始 value 映射到可协商的 `rep_type`。
2. `type_handler` 的协商对象是 `lhs_rep/rhs_rep`，而非原始 value type。
3. 若任一输入 `is_valid_rep(...) == false`，立即构造 `runtime_error_kind::domain_error` 并进入 error policy。
4. 通过校验后执行 `from_rep -> to_rep` 规范化，再进入 value handler 与 op binding。
5. 当前结果值仍按 `common_rep` 回传；comparison 最小闭环采用 `0/1` 表示（`static_cast<common_rep>(bool)`）。
6. 本契约不改变 dispatcher 链路顺序，也不改变错误枚举体系。

**Further Considerations**
1. expected 的错误载体类型建议先统一为轻量错误枚举，再逐步演进到可扩展错误域，以减少首版模板复杂度。
2. type policy 的 common type 规则建议先采用“显式白名单 + static_assert 诊断”，避免首版引入过宽的隐式提升。
3. 后续可新增“native 快速路径”作为可选 API：当组合为 primitive<unchecked_value, transparent_type, single_thread> 时，提供非 expected 返回通道以最大化贴近原生 C/C++ 性能与行为。
4. 后续可新增 C API 适配层（extern "C" 薄封装，POD 入参与返回值），内部复用 unchecked/native 路径，优先保障与现有 C 调用约定兼容。
