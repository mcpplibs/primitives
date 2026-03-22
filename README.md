# mcpplibs primitives

> C++23 模块化原语库 - `import mcpplibs.primitives;`

本仓库提供可配置的 `primitive` 基础设施（`underlying traits`、`policy`、`operations/dispatcher`），用于统一约束数值计算、错误处理与并发访问语义。

> [!WARNING]
> 当前项目仍在快速演进中，API 可能发生变更。

## 特性

- **C++23 模块** — `import mcpplibs.primitives;`
- **双构建系统** — 同时支持 xmake 和 CMake
- **策略驱动行为** — 值/类型/错误/并发策略可组合配置
- **混合运算支持** — 支持 `primitive` 与 `underlying` 的混合二元运算
- **并发访问接口** — `primitive::load/store/compare_exchange`

## Operators

该库为 `primitive` 提供了常见的一元、算术、位运算与比较操作。  
算术结果通过统一分发链路返回 `std::expected<..., policy::error::kind>`。

- 值策略（`policy::value::checked` / `policy::value::saturating` / `policy::value::unchecked`）决定溢出行为；
- 错误策略（`policy::error::throwing` / `policy::error::expected` / `policy::error::terminate`）决定错误传播方式。

示例：

```cpp
import std;
import mcpplibs.primitives;

using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::operators;

primitive<int> a{1};
primitive<int> b{2};
auto sum = a + b; // std::expected<primitive<int>, policy::error::kind>

using checked_t =
    primitive<int, policy::value::checked, policy::error::expected>;
auto maybe_overflow =
    checked_t{std::numeric_limits<int>::max()} + checked_t{1};
```

## Policy 协议命名空间

自定义 policy 时，协议入口按职责拆分到子命名空间：

- `policy::type::handler` / `policy::type::handler_available`
- `policy::concurrency::handler` / `policy::concurrency::injection`
- `policy::value::handler` / `policy::value::decision`
- `policy::error::handler` / `policy::error::request` / `policy::error::kind`

预设 policy 标签：

- `policy::value::{checked, unchecked, saturating}`
- `policy::type::{strict, compatible, transparent}`
- `policy::error::{throwing, expected, terminate}`
- `policy::concurrency::{none, fenced, fenced_relaxed, fenced_acq_rel, fenced_seq_cst}`

并发策略说明：

- `fenced*` 系列是操作级并发语义，通过策略注入内存序 fence；
- `primitive` 存储仍保持统一、零额外存储抽象；
- `primitive::load/store/compare_exchange` 由并发策略协议提供，若策略未实现会在编译期报错。

示例（并发访问 API）：

```cpp
using shared_t = primitive<int, policy::value::checked,
                           policy::concurrency::fenced_acq_rel,
                           policy::error::expected>;

shared_t v{1};
v.store(2);
auto expected = 2;
if (v.compare_exchange(expected, 3)) {
  auto now = v.load();
  (void)now;
}
```

默认策略位于 `policy::defaults`：

- `policy::defaults::value`
- `policy::defaults::type`
- `policy::defaults::error`
- `policy::defaults::concurrency`

## 示例程序

- `ex01_default_arithmetic`: 默认策略下的基础算术运算示例。
- `ex02_type_policy`: 展示 `strict/compatible` 的类型协商差异，并包含 `underlying` 构造路径对 type 策略的影响。
- `ex03_value_policy`: 展示 `checked/unchecked/saturating`，并包含与 `underlying` 的混合二元运算行为。
- `ex04_error_policy`: 展示不同 error 策略的处理方式。
- `ex05_concurrency_policy`: 读写组合并发场景（writer `store` + reader `add/sub` + `CAS`）示例。
- `ex06_custom_underlying`: 自定义 underlying traits、rep 校验与 common rep 扩展。
- `ex07_custom_policy`: 自定义策略协议实现示例。
- `ex08_custom_operation`: 自定义 operation 扩展示例。

## 项目结构

```
mcpplibs-primitives/
├── src/                        # 模块源码
│   ├── primitives.cppm         # 顶层聚合模块
│   ├── primitive/              # primitive 定义与 traits
│   ├── policy/                 # policy 标签与协议实现
│   ├── operations/             # operation tags / dispatcher / operators
│   └── underlying/             # underlying traits 与 common_rep
├── examples/                   # ex01 ~ ex08 示例
├── tests/                      # 测试入口与 basic 测试集
├── xmake.lua                   # xmake 构建脚本
├── CMakeLists.txt              # CMake 构建脚本
└── .xlings.json                # xlings 包描述文件
```

## 快速开始

```cpp
import std;
import mcpplibs.primitives;

int main() {
  using namespace mcpplibs::primitives;

  using value_t = primitive<int, policy::error::expected>;
  auto const result = operations::add(value_t{40}, value_t{2});
  return (result.has_value() && result->value() == 42) ? 0 : 1;
}
```

## 安装与配置

```bash
xlings install
```

## 构建与运行

**使用 xmake**

```bash
xmake build mcpplibs-primitives
xmake run basic                    # 等价于 ex01_default_arithmetic
xmake run ex05_concurrency_policy
xmake run primitives_test
```

**使用 CMake**

```bash
cmake -B build -G Ninja
cmake --build build --target mcpplibs-primitives
cmake --build build --target ex01_default_arithmetic
cmake --build build --target basic_tests
ctest --test-dir build --output-on-failure
```

## 集成到构建工具

### xmake

```lua
add_repositories("mcpplibs-index https://github.com/mcpplibs/mcpplibs-index.git")

add_requires("primitives")

target("myapp")
    set_kind("binary")
    set_languages("c++23")
    add_files("main.cpp")
    add_packages("primitives")
    set_policy("build.c++.modules", true)
```

## 相关链接

- [mcpp-style-ref | 现代 C++ 编码/项目风格参考](https://github.com/mcpp-community/mcpp-style-ref)
- [d2mystl | 从零实现一个迷你STL库](https://github.com/mcpp-community/d2mystl)
- [mcpp 社区官网](https://mcpp.d2learn.org)
- [mcpp | 现代 C++ 爱好者论坛](https://mcpp.d2learn.org/forum)
- [入门教程: 动手学现代 C++](https://github.com/Sunrisepeak/mcpp-standard)
