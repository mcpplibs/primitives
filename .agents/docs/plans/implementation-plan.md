# Primitives 下一步实现计划清单（2026-03-26）

## 总体原则

- [ ] 先定义稳定 API 边界，再落地实现
- [ ] 每个能力都配套 tests + examples + docs
- [ ] 高风险转换默认禁止隐式触发，只允许显式 API
- [ ] 明确分层职责：`algorithms` 负责值域/排序/边界元信息与值算法；`conversion` 负责显式类型变换
- [ ] 明确依赖方向：`conversion` 可依赖 `algorithms`，`algorithms` 不反向依赖 `conversion`

## M1. C API 互操作层（双向）

### 工作清单

- [ ] 定义稳定 C ABI（命名规范、导出宏、版本策略）
- [ ] 设计双向能力：C 调用 primitives；primitives 适配并调用 C API
- [ ] 统一错误模型（error code + 可选 message buffer）
- [ ] 提供最小可用头文件：`include/mcpplibs/primitives/c_api.h`
- [ ] 增加 C/C++ 混合构建测试（C 编译器 + C++ 链接）
- [ ] 增加示例：`examples/c/basic.c`、`examples/cpp/use_c_api.cpp`

### 验收标准

- [ ] 纯 C 工程可链接并完成基础能力调用
- [ ] C++ 对外部 C API 的适配调用行为可控、无未定义行为

## M2. Concept/Traits 体系增强与元信息 API

### 工作清单

- [ ] 扩展 concept 分层（category/representation/policy-capability）
- [ ] 明确 `underlying::traits<T>` 与 `algorithms::traits<T>` 的职责边界：前者描述表示层；后者描述值算法层
- [ ] 完善 `traits<T>` 元信息（`kind`、`rep_type`、policy tags）
- [ ] 设计 `algorithms::traits<T>`，内容参考 `std::numeric_limits`，但只保留算法/比较所需子集：
- [ ] `min()` / `lowest()` / `max()` / `epsilon()` / `infinity()` / `quiet_nan()`
- [ ] `is_bounded` / `is_exact` / `is_signed` / `is_integer` / `is_iec559`
- [ ] `has_infinity` / `has_quiet_nan` / `digits` / `digits10` / `radix`
- [ ] 增加排序能力元信息：`comparison_category`、`totally_ordered`、`partially_ordered`、`unordered_possible`
- [ ] 提供双参聚合元信息：`algorithms::common_traits<Lhs, Rhs>`，统一暴露 `common_rep`、可比较性、边界查询与策略兼容性
- [ ] 提供检测类 API（可比较性/可裁剪性/可转换性/是否有损/错误模型能力）
- [ ] 统一 `constexpr` 查询入口，减少分散 traits 访问
- [ ] 增加编译期测试矩阵（`static_assert` 覆盖）

### 验收标准

- [ ] 上层模块仅依赖公开 concept/traits，不依赖 `details::*`
- [ ] `algorithms::traits` 可直接支撑算法层判定，并作为 conversion 的元信息依赖
- [ ] `std::numeric_limits` 相关逻辑不再散落在 `conversion`/`operations` 内部

## M3. 显式转换层（任意策略组适用）

### 工作清单

- [ ] 设计统一接口族（建议）：`explicit_cast`、`try_cast`、`checked_cast`
- [ ] 将边界、NaN、无穷大、精度与排序相关判定统一改为依赖 `algorithms::traits` / `algorithms::common_traits`
- [ ] 支持任意策略组组合，不绑定特定策略实现
- [ ] 风险可见化（截断/溢出/精度损失）并可程序化读取
- [ ] 定义失败语义（错误码或 expected 风格，按策略可配置）
- [ ] 建立转换矩阵测试（同类/跨类/跨策略组）

### 验收标准

- [ ] 所有跨类高风险转换必须走显式 API
- [ ] 风险信息可在编译期或运行期被确定性获取
- [ ] `conversion` 模块仅消费 `algorithms` 提供的元信息，不反向定义算法层协议

## M4. 算法层（traits 先行，以 max/min 为起点）

### 工作清单

- [ ] 确定模块结构：`mcpplibs.primitives.algorithms`、`mcpplibs.primitives.algorithms.traits`、`mcpplibs.primitives.algorithms.compare`、`mcpplibs.primitives.algorithms.minmax`
- [ ] 先实现 `algorithms::traits` 与 `algorithms::common_traits`，作为整个算法层与 conversion 的基础依赖
- [ ] 约束算法职责只涉及值的比较、选择、裁剪与边界处理，不提供目标类型导向的 cast API
- [ ] `max`/`min` 内部仅允许做 `common_rep` 归一化或排序语义协商，不绕回 conversion 层
- [ ] 实现 `max`/`min`，并预留 `clamp`/`compare` 扩展位
- [ ] 支持同类输入与受约束的异类输入；异类场景必须满足 `common_rep`、排序能力与策略组约束
- [ ] 明确 `partial_ordering` / `unordered` 语义，尤其是浮点 `NaN` 路径
- [ ] 为 `clamp`/`compare` 抽出复用内核：公共比较、边界判定、值选择
- [ ] 在可行范围保持 `constexpr`/`noexcept` 特性
- [ ] 增加边界测试（极值、NaN、有符号/无符号混合、不同 `comparison_category`）

### 验收标准

- [ ] 算法行为与策略约束一致
- [ ] 风险路径始终显式、可审计
- [ ] `algorithms` 可独立编译并被 `conversion` 依赖，不形成反向模块耦合
- [ ] 算法层实现不通过“显式转换 API”间接完成值比较

## 建议推进顺序

1. M2（先夯实约束与元信息基础）
2. M4 前半：`algorithms::traits` / `common_traits` / 排序能力查询
3. M3（让 conversion 改为依赖 algorithms 元信息）
4. M4 后半：`max` / `min` / `clamp` / `compare`
5. M1（建立跨语言边界，可按需求并行推进）

## 总体完成跟踪

- [ ] M1 完成
- [ ] M2 完成
- [ ] M3 完成
- [ ] M4 完成
