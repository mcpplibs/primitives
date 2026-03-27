# 扩展开发指南

本页面面向需要扩展库能力的用户。

## 1) 扩展 underlying 类型

定义业务类型后，特化 `underlying::traits<T>`。

必需成员：

- `value_type`
- `rep_type`
- `static constexpr bool enabled`
- `static constexpr underlying::category kind`
- `to_rep(value)`
- `from_rep(rep)`
- `is_valid_rep(rep)`

完成后，只要满足 `underlying_type`，即可用于 `primitive<YourType, ...>`。

参考：`examples/ex08_custom_underlying.cpp`

## 2) 自定义 common rep 协商

当默认 `std::common_type_t` 不满足需求时，可特化：

```cpp
template <>
struct mcpplibs::primitives::underlying::common_rep_traits<LhsRep, RhsRep> {
  using type = YourCommonRep;
  static constexpr bool enabled = true;
};
```

这会影响 mixed dispatch 与 type negotiation。

## 3) 自定义 policy 标签与协议处理器

自定义 policy 需要：

1. 通过 `policy::traits<YourPolicyTag>` 注册标签。
2. 在对应命名空间提供协议特化：
   - `policy::type::handler`
   - `policy::value::handler`
   - `policy::error::handler`
   - `policy::concurrency::handler`

如果 value policy 需要参与运行期运算逻辑，还要补充 `operations::runtime::op_binding` 特化。

参考：`examples/ex09_custom_policy.cpp`

## 4) 自定义 operation 标签

新增 operation 的典型步骤：

1. 定义 operation tag 类型。
2. 特化 `operations::traits<OpTag>`，设置：
   - `enabled = true`
   - `arity`
   - `capability_mask`
3. 提供 `operations::runtime::op_binding<OpTag, ValuePolicy, CommonRep>`。
4. 通过 `operations::apply<OpTag>(lhs, rhs)` 调用。

参考：`examples/ex10_custom_operation.cpp`

## 扩展检查清单

- 双操作数策略组是否一致。
- operation capability 元信息是否正确。
- value policy + operation binding 是否覆盖运行期分发路径。
- error handler 返回类型是否匹配。
- 若需要 `load/store/CAS`，并发 access handler 是否实现。

## 下一步

- 协议契约与模板细节： [../../api/zh/README.md](../../api/zh/README.md)

