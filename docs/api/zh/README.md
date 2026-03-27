# primitives API 文档

> 适用模块：`import mcpplibs.primitives;`

## 模块入口

顶层模块：

```cpp
import mcpplibs.primitives;
```

该模块导出以下子模块：

- `mcpplibs.primitives.underlying`
- `mcpplibs.primitives.policy`
- `mcpplibs.primitives.primitive`
- `mcpplibs.primitives.operations`
- `mcpplibs.primitives.algorithms`
- `mcpplibs.primitives.conversion`

## 命名空间总览

- `mcpplibs::primitives`: 核心类型、概念与 `primitive`。
- `mcpplibs::primitives::underlying`: underlying traits 与类别。
- `mcpplibs::primitives::policy`: policy 标签、默认值与协议。
- `mcpplibs::primitives::operations`: 分发函数 API。
- `mcpplibs::primitives::conversion`: 数值风险检查与转换辅助接口。
- `mcpplibs::primitives::algorithms`: limits 与 hash 辅助接口。
- `mcpplibs::primitives::operators`: 运算符重载入口。
- `mcpplibs::primitives::literals`: 带范围校验的 underlying 字面量后缀。
- `mcpplibs::primitives::meta`: primitive 元信息 traits。
- `mcpplibs::primitives::types`: 常用 underlying 的 primitive 别名。

## 核心概念与元信息

### underlying 相关概念

- `underlying_type<T>`
- `boolean_underlying_type<T>`
- `character_underlying_type<T>`
- `integer_underlying_type<T>`
- `floating_underlying_type<T>`
- `numeric_underlying_type<T>`
- `has_common_rep<LhsRep, RhsRep>`
- `common_rep_t<LhsRep, RhsRep>`

### policy 相关概念

- `policy::policy_type<P>`
- `policy::value_policy<P>`
- `policy::type_policy<P>`
- `policy::error_policy<P>`
- `policy::concurrency_policy<P>`
- `policy::resolve_policy_t<category, Policies...>`

### primitive 元信息

- `meta::traits<primitive<...>>`  
  暴露 `value_type / policies / value_policy / type_policy / error_policy / concurrency_policy`
- `meta::make_primitive_t<T, PoliciesTuple>`

## `primitive<T, Policies...>`

```cpp
template <underlying_type T, policy::policy_type... Policies>
class primitive;
```

### 成员类型

- `value_type = T`
- `policies = std::tuple<Policies...>`

### 主要构造与赋值

- `explicit primitive(T)`：同 underlying 构造。
- `explicit primitive(U)`：跨 underlying 构造（是否允许由 type 策略决定）。
- 支持同策略组下的拷贝/移动构造与赋值。
- 支持同策略组、不同 underlying 的 primitive 互构造/赋值（受 type 策略约束）。

### 值访问与并发访问 API

- `value()` / `value() const`
- `explicit operator value_type() const`
- `load() -> value_type`
- `store(desired)`
- `compare_exchange(expected, desired) -> bool`

说明：

- `store/compare_exchange` 支持同 underlying。
- 在 type 策略允许时，`store/compare_exchange` 也支持跨 underlying 参数。
- 并发访问能力由 `policy::concurrency::handler<..., void, ...>` 提供。

### type 策略对跨 underlying 行为的影响

- `policy::type::strict`：仅同类型允许，跨 underlying 构造/存储/CAS 不可用。
- `policy::type::compatible`：要求同 underlying category 且可协商 `common_rep`。
- `policy::type::transparent`：只要求可协商 `common_rep`。

### 预定义别名（`types`）

- 整型：`U8/U16/U32/U64`、`Size`、`Diff`、`I8/I16/I32/I64`
- 浮点：`F32/F64/F80`
- 布尔/字符：`Bool/UChar/Char8/Char16/Char32/WChar`

用法示例：

```cpp
using value_t = mcpplibs::primitives::types::I32<
    mcpplibs::primitives::policy::value::checked,
    mcpplibs::primitives::policy::error::expected>;
```

### 工厂函数与 literals

- `with(value)`：从 underlying 值按默认策略构造 `primitive`
- `with<Policies...>(value)`：构造时显式指定策略集合
- `with(policies_tuple, value)`：复用已有策略元组
- `mcpplibs::primitives::literals` 提供带校验的后缀，例如
  `_u8/_u16/_u32/_u64/_size/_diff`、`_i8/_i16/_i32/_i64`、
  `_f32/_f32e/_f64/_f64e/_f80/_f80e`，以及
  `_uchar/_char8/_char16/_char32/_wchar`

示例：

```cpp
using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::literals;

auto value = with<policy::value::checked, policy::error::expected>(42_i32);
```

## `operations` API

`operations` 命名空间提供函数式调用接口，统一返回 `std::expected`。

### 常用结果类型别名

- `primitive_dispatch_result_t<OpTag, Lhs, Rhs, ErrorPayload>`
- `mixed_primitive_dispatch_result_t<OpTag, Primitive, Underlying, ErrorPayload>`
- `three_way_dispatch_result_t<Lhs, Rhs, ErrorPayload>`

### 一元操作

- `increment`
- `decrement`
- `bit_not`
- `unary_plus`
- `unary_minus`

### 二元算术

- `add`
- `sub`
- `mul`
- `div`
- `mod`

### 二元位运算

- `shift_left`
- `shift_right`
- `bit_and`
- `bit_or`
- `bit_xor`

### 比较

- `equal`
- `not_equal`
- `three_way_compare`

### 复合赋值

- `add_assign/sub_assign/mul_assign/div_assign/mod_assign`
- `shift_left_assign/shift_right_assign`
- `bit_and_assign/bit_or_assign/bit_xor_assign`

### 混合操作支持

对多数二元操作，支持以下三种形式：

- `primitive op primitive`
- `primitive op underlying`
- `underlying op primitive`

## `operators` 运算符重载

若需要 `+ - * / % ...` 运算符语法，请引入：

```cpp
using namespace mcpplibs::primitives::operators;
```

重载结果同样是 `std::expected<...>`，不是裸值。

## `conversion` API

`conversion` 命名空间提供 underlying、primitive 以及混合路径之间的统一转换接口。

### 常用结果类型与风险模型

- `cast_result<T> = std::expected<T, conversion::risk::kind>`
- `conversion::risk::kind`：
  `none`、`overflow`、`underflow`、`domain_error`、`precision_loss`、
  `sign_loss`、`invalid_type_combination`

### 转换函数

- `numeric_risk<Dest>(value)`
- `unchecked_cast<Dest>(value)`
- `checked_cast<Dest>(value)`
- `saturating_cast<Dest>(value)`
- `truncating_cast<Dest>(value)`
- `exact_cast<Dest>(value)`

说明：

- `checked_cast` 与 `exact_cast` 返回 `cast_result<T>`
- `saturating_cast` 会对数值溢出/下溢执行钳制
- `truncating_cast` 适合浮点到整数的截断场景
- 提供 underlying -> underlying、primitive -> primitive、
  primitive -> underlying、underlying -> primitive 四类重载

## `algorithms` API

`algorithms` 命名空间提供 limits 元信息与 hash 辅助接口，适用于内置类型和受支持的自定义 underlying。

### Limits

- `limits<T>`
- `limited_type<T>`
- `min_value<T>()`
- `lowest_value<T>()`
- `max_value<T>()`
- `epsilon_value<T>()`
- `infinity_value<T>()`
- `quiet_nan_value<T>()`

### Hash

- `hash<T>`
- `hash_result_t<T>`
- `hashable<T>`
- `hash_value(value)`

## policy API

### 内置策略标签

- 值策略：`policy::value::{checked, unchecked, saturating}`
- 类型策略：`policy::type::{strict, compatible, transparent}`
- 错误策略：`policy::error::{throwing, expected, terminate}`
- 并发策略：`policy::concurrency::{none, fenced, fenced_relaxed, fenced_acq_rel, fenced_seq_cst}`

### 默认策略

- `policy::defaults::value = policy::value::checked`
- `policy::defaults::type = policy::type::strict`
- `policy::defaults::error = policy::error::throwing`
- `policy::defaults::concurrency = policy::concurrency::none`

### 错误码

`policy::error::kind`：

- `none`
- `invalid_type_combination`
- `overflow`
- `underflow`
- `divide_by_zero`
- `domain_error`
- `unspecified`

## underlying 扩展点

### 1) 注册 `underlying::traits<T>`

需要提供：

- `value_type`
- `rep_type`
- `enabled`
- `kind`
- `to_rep(value)`
- `from_rep(rep)`
- `is_valid_rep(rep)`

### 2) 自定义 common rep 协商

可特化 `underlying::common_rep_traits<LhsRep, RhsRep>`，以覆盖默认 `std::common_type_t` 规则。

## 返回模型与错误处理模型

- 绝大多数 operations/operators API 返回 `std::expected<...>`。
- 当错误策略为 `policy::error::expected` 时，错误通过 `unexpected(policy::error::kind)` 返回。
- 当错误策略为 `policy::error::throwing` 时，运行期错误会抛出异常（签名仍保持 `std::expected`）。

## 最小示例

```cpp
import std;
import mcpplibs.primitives;

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::operators;

int main() {
  using value_t =
      primitive<int, policy::value::checked, policy::error::expected>;

  auto const out = value_t{40} + value_t{2};
  if (!out.has_value()) {
    return 1;
  }
  return out->value() == 42 ? 0 : 1;
}
```
