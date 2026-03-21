## Plan: 策略组一致性 + Primitive/Underlying 混合操作（增量）

本计划仅覆盖新增规则与新增重载，不重复已完成的分层协议与分发基建。

### A. 已确认规则（实现目标）
1. 单个 `primitive` 的策略组规则保持不变  
   - 同类别策略标签重复时，编译期报错。

2. 两个 `primitive` 的策略组合并规则  
   - 先按默认策略补全四类策略（`value/type/error/concurrency`）。  
   - 补全后两边策略组必须完全一致（忽略声明顺序），否则编译期报错。

3. 两个 `primitive` 底层类型不同时的二元操作规则  
   - 先满足规则 2。  
   - `type::strict`：编译期拒绝。  
   - `type::compatible`：两侧 `underlying category` 必须一致，且 `common_rep_traits` 可用且推导结果非 `void`。  
   - `type::transparent`：忽略 `category`，但 `common_rep_traits` 必须可用且推导结果非 `void`。

4. 不新增策略标签  
   - 维持 `compatible` / `transparent` 现有策略集合，不引入新 policy。

### B. 新增范围：Primitive 与 Underlying 混合作为二元操作数
1. 在 `apply/operation` 层新增 `underlying` 参与重载  
   - 支持 `(primitive, underlying)`。  
   - 若是自由函数，必须同时支持 `(underlying, primitive)`（左右位置都可用）。

2. 返回类型约束  
   - 算术类型操作结果始终返回 `primitive`（不返回裸 `underlying`）。

3. 策略选择约束  
   - 混合操作中仅有一个 `primitive` 时，`type` 策略取该 `primitive` 的策略。  
   - 其余策略（`value/error/concurrency`）同样取该 `primitive`，保持策略来源单一。

4. 底层类型推导约束  
   - 混合操作的 `underlying` 推导与“两侧都是 primitive”的规则一致：  
   - 走相同的 `type` 协商路径，使用同一套 `common_rep_traits` 可用性与非 `void` 判定。

### C. 代码改动计划
1. `src/operations/dispatcher.cppm`
   - 增加“跨 primitive 策略组补全后一致”的编译期断言。  
   - 增加 `compatible` 下 `category` 一致性判定（与 `common_rep` 可用性判定协同）。  
   - 补齐 `common_rep` 非 `void` 的显式断言。

2. `src/operations/operators.cppm`
   - 为自由函数 `apply/add/sub/mul/div/equal/not_equal/three_way_compare` 增加混合重载：  
   - `(primitive, underlying)` 与 `(underlying, primitive)`。  
   - 保证非交换操作（如 `sub/div/<=>`）保留正确操作数顺序语义。

3. `src/primitive/impl.cppm`（如需）
   - 若存在 primitive access 的自由函数入口，同步补齐混合重载的左右位置版本。  
   - 成员函数形式的 access API 不做左右重载扩展。

4. `tests/basic/test_operations.cpp`
   - 新增混合重载测试：  
   - `(primitive, underlying)` 与 `(underlying, primitive)` 都可编译并语义正确。  
   - 算术结果类型恒为 `primitive`。  
   - 策略组不一致时编译期失败。  
   - `strict/compatible/transparent` 下的跨底层类型判定符合规则 A-3。

### D. 验收标准
1. 所有新增约束均在编译期可判定，不引入运行期兜底分支。
2. 混合操作重载完整覆盖左右操作数位置（自由函数）。
3. 算术混合操作返回类型恒为 `primitive`。
4. 混合操作的 `type` 策略来源唯一且可预测（取 primitive 操作数）。
5. 与现有行为无回归，新增正反测试通过。

