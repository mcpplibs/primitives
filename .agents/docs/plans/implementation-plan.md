# Primitives 下一步实现计划清单（2026-03-23）

## 总体原则

- [ ] 先定义稳定 API 边界，再落地实现
- [ ] 每个能力都配套 tests + examples + docs
- [ ] 高风险转换默认禁止隐式触发，只允许显式 API

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
- [ ] 完善 `traits<T>` 元信息（`kind`、`rep_type`、limits、policy tags）
- [ ] 提供检测类 API（可转换性/是否有损/错误模型能力）
- [ ] 统一 `constexpr` 查询入口，减少分散 traits 访问
- [ ] 增加编译期测试矩阵（`static_assert` 覆盖）

### 验收标准

- [ ] 上层模块仅依赖公开 concept/traits，不依赖 `details::*`
- [ ] 元信息可支撑转换层与算法层的约束判定

## M3. 显式转换层（任意策略组适用）

### 工作清单

- [ ] 设计统一接口族（建议）：`explicit_cast`、`try_cast`、`checked_cast`
- [ ] 支持任意策略组组合，不绑定特定策略实现
- [ ] 风险可见化（截断/溢出/精度损失）并可程序化读取
- [ ] 定义失败语义（错误码或 expected 风格，按策略可配置）
- [ ] 建立转换矩阵测试（同类/跨类/跨策略组）

### 验收标准

- [ ] 所有跨类高风险转换必须走显式 API
- [ ] 风险信息可在编译期或运行期被确定性获取

## M4. 算法层（以 max/min 为起点）

### 工作清单

- [ ] 实现 `max`/`min`，并预留 `clamp`/`compare` 扩展位
- [ ] 算法统一依赖 M2+M3 的公开接口，不绕过转换层
- [ ] 支持同类与受约束的异类输入
- [ ] 在可行范围保持 `constexpr`/`noexcept` 特性
- [ ] 增加边界测试（极值、NaN、有符号/无符号混合）

### 验收标准

- [ ] 算法行为与策略约束一致
- [ ] 风险路径始终显式、可审计

## 建议推进顺序

1. M2（先夯实约束与元信息基础）
2. M1（建立跨语言边界）
3. M3（收敛转换风险）
4. M4（复用基础能力实现算法）

## 总体完成跟踪

- [ ] M1 完成
- [ ] M2 完成
- [ ] M3 完成
- [ ] M4 完成
