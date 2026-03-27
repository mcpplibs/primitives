# mcpplibs primitives

> C++23 模块化 primitives 库 - `import mcpplibs.primitives;`

[![d2x](https://img.shields.io/badge/d2x-ok-green.svg)](https://github.com/d2learn/d2x)
[![Online-ebook](https://img.shields.io/badge/online-ebook-orange.svg)](https://github.com/d2learn/d2x)
[![License](https://img.shields.io/badge/license-Apache_2.0-blue.svg)](LICENSE-CODE)

| [中文](README.zh.md) - [English](README.md) - [论坛](https://mcpp.d2learn.org/forum) |
|----------------------------------------------------------------------------------|
| [用户文档](docs/guide/zh/README.md) - [User Documentation](docs/guide/en/README.md) |
| [API文档](docs/api/zh/README.md) - [API Documentation](docs/api/en/README.md) |

本仓库提供可配置的 `primitive` 基础设施（`underlying traits`、`policy`、`operations/dispatcher`），用于统一数值行为、错误处理与并发访问语义。

> [!WARNING]
> 项目仍在快速演进中，API 可能继续调整。

## 特性

- **C++23 模块**: `import mcpplibs.primitives;`
- **双构建系统**: 同时支持 xmake 和 CMake
- **策略驱动行为**: value/type/error/concurrency 策略可组合配置
- **混合运算支持**: 支持 `primitive` 与 `underlying` 的二元运算
- **并发访问接口**: `primitive::load/store/compare_exchange`

## Operators

该库为 `primitive` 提供常见的一元、算术、位运算和比较操作。
算术路径通过统一分发链路返回 `std::expected<..., policy::error::kind>`。

- 值策略（`policy::value::checked` / `policy::value::saturating` / `policy::value::unchecked`）定义溢出行为。
- 错误策略（`policy::error::throwing` / `policy::error::expected` / `policy::error::terminate`）定义错误传播方式。

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

实现自定义 policy 时，协议入口按职责拆分为以下命名空间：

- `policy::type::handler` / `policy::type::handler_available`
- `policy::concurrency::handler` / `policy::concurrency::injection`
- `policy::value::handler` / `policy::value::decision`
- `policy::error::handler` / `policy::error::request` / `policy::error::kind`

内置 policy 标签：

- `policy::value::{checked, unchecked, saturating}`
- `policy::type::{strict, compatible, transparent}`
- `policy::error::{throwing, expected, terminate}`
- `policy::concurrency::{none, fenced, fenced_relaxed, fenced_acq_rel, fenced_seq_cst}`

并发说明：

- `fenced*` 系列提供操作级并发语义，并通过策略注入内存序 fence。
- `primitive` 存储保持统一的零额外存储抽象。
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

- `ex01_basic_usage`: 展示 literal 与 primitive 工厂函数联合使用，并覆盖更多内置操作符。
- `ex02_type_policy`: 展示 `strict/compatible` 的类型协商差异，以及 `underlying` 构造对 type 策略的影响。
- `ex03_value_policy`: 展示 `checked/unchecked/saturating` 行为，以及与 `underlying` 的混合二元运算。
- `ex04_error_policy`: 展示不同 error 策略下的错误处理方式。
- `ex05_concurrency_policy`: 展示典型的读写混合并发场景（writer `store` + reader `add/sub` + `CAS`）。
- `ex06_conversion`: 展示 underlying 与 primitive 之间的 `checked/saturating/truncating/exact` 转换辅助接口。
- `ex07_algorithms`: 展示 limits 元信息、特殊数值和哈希辅助接口。
- `ex08_custom_underlying`: 展示自定义 underlying traits、rep 校验和 common rep 扩展。
- `ex09_custom_policy`: 展示自定义 policy 协议实现。
- `ex10_custom_operation`: 展示自定义 operation 扩展。

## 项目结构

```
mcpplibs-primitives/
├── src/                        # 模块源码
│   ├── primitives.cppm         # 顶层聚合模块
│   ├── primitive/              # primitive 定义与 traits
│   ├── policy/                 # policy 标签与协议实现
│   ├── operations/             # operation tags / dispatcher / operators
│   └── underlying/             # underlying traits 与 common_rep
├── examples/                   # 示例程序
├── tests/                      # 测试入口与基础测试集
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
xmake run basic                    # ex01_basic_usage 的兼容别名
xmake run ex01_basic_usage
xmake run ex06_conversion
xmake run ex07_algorithms
xmake run ex05_concurrency_policy
xmake run primitives_test
```

**使用 CMake**（`CMake >= 3.31`）

```bash
cmake -B build -G Ninja
cmake --build build --target mcpplibs-primitives
cmake --build build --target ex01_basic_usage
cmake --build build --target ex06_conversion
cmake --build build --target ex07_algorithms
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

- [mcpp-style-ref | 现代 C++ 编码与项目风格参考](https://github.com/mcpp-community/mcpp-style-ref)
- [d2mystl | 从零实现一个迷你 STL](https://github.com/mcpp-community/d2mystl)
- [mcpp 社区网站](https://mcpp.d2learn.org)
- [mcpp 论坛](https://mcpp.d2learn.org/forum)
- [入门教程：动手学习现代 C++](https://github.com/Sunrisepeak/mcpp-standard)
