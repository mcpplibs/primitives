# 快速上手

## 最小可运行示例

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

关键点：算术和运算符接口大多返回 `std::expected<...>`。

## 第一组推荐策略

建议起步组合：

- `policy::value::checked`
- `policy::error::expected`
- 默认 `policy::type::strict`
- 默认 `policy::concurrency::none`

这组配置通常具备较好的安全性与可诊断性。

## 通过示例学习

建议按以下顺序运行示例：

1. `ex01_default_arithmetic`
2. `ex02_type_policy`
3. `ex03_value_policy`
4. `ex04_error_policy`
5. `ex05_concurrency_policy`
6. `ex06_custom_underlying`
7. `ex07_custom_policy`
8. `ex08_custom_operation`

运行示例：

```bash
xmake run ex03_value_policy
```

## 常用代码片段

### 默认 primitive

```cpp
using default_i32 = mcpplibs::primitives::primitive<int>;
```

### primitive 与 underlying 混合运算

```cpp
using value_t = mcpplibs::primitives::primitive<
    int,
    mcpplibs::primitives::policy::type::compatible,
    mcpplibs::primitives::policy::error::expected>;

auto const lhs = value_t{40};
short const rhs = 2;
auto const out = mcpplibs::primitives::operations::add(lhs, rhs);
```

### 启用运算符重载命名空间

```cpp
using namespace mcpplibs::primitives::operators;
```

如果不引入该命名空间，请使用 `mcpplibs::primitives::operations` 下的函数式 API。

## 下一步

- 并发用法： [./concurrency.md](./concurrency.md)
- 扩展开发： [./extension.md](./extension.md)
- API 参考： [../../api/zh/README.md](../../api/zh/README.md)

