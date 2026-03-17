# mcpplibs primitives

> C++23 模块化原语库 - `import mcpplibs.primitives;`

本仓库实现了底层强类型 primitive 基础设施（traits、policy、underlying 类型分类），供上层 `Integer`/`Floating`/`Boolean` 等封装使用。

> [!WARNING]
> 目前项目还在开发中，API会随着后续演进而改变

## 特性

- **C++23 模块** — `import mcpplibs.primitives;`
- **双构建系统** — 同时支持 xmake 和 CMake
- **CI/CD** — GitHub Actions 多平台构建（Linux / macOS / Windows）
- **标准化结构** — 遵循 [mcpp-style-ref](https://github.com/mcpp-community/mcpp-style-ref) 编码规范
- **开箱即用** — 包含示例、测试和架构文档

## Operators

该库在 `primitive` 类型上重载了常见的 C++ 算术、位运算和一元运算符。算术行为受策略（policy）控制：

- 值策略（`policy::value::checked` / `policy::value::saturating` / `policy::value::unchecked`）决定溢出行为；
- 错误策略（`policy::error::throwing` / `policy::error::expected` / `policy::error::terminate`）决定在 `policy::value::checked` 且发生错误时的处理方式。

示例：

```cpp
import mcpplibs.primitives;
using namespace mcpplibs::primitives;
using namespace mcpplibs::primitives::policy;

primitive<int> a{1}, b{2};
auto c = a + b; // primitive<int>

primitive<int, policy::error::expected> x{std::numeric_limits<int>::max()};
primitive<int, policy::error::expected> y{1};
auto maybe = x + y; // std::expected<primitive<int, policy::error::expected>, policy::error::kind>
```

## Policy 协议命名空间

自定义 policy 时，协议入口已按职责拆分到子命名空间：

- `policy::type::handler` / `policy::type::handler_available`
- `policy::concurrency::handler` / `policy::concurrency::injection`
- `policy::value::handler` / `policy::value::decision`
- `policy::error::handler` / `policy::error::request` / `policy::error::kind`

预设 policy 标签也按类别归档：

- `policy::value::{checked, unchecked, saturating}`
- `policy::type::{strict, compatible, transparent}`
- `policy::error::{throwing, expected, terminate}`
- `policy::concurrency::{none, atomic}`

默认策略位于 `policy::defaults`：

- `policy::defaults::value`
- `policy::defaults::type`
- `policy::defaults::error`
- `policy::defaults::concurrency`


## 项目结构

```
mcpplibs-primitives/
├── src/                    # 模块源码
│   └── primitive.cppm      # 主模块接口（导出 traits 与 primitive 聚合）
├── tests/                  # 测试
│   ├── main.cpp
│   └── xmake.lua
├── examples/               # 示例
│   ├── basic.cpp
│   └── xmake.lua
├── docs/                   # 文档
│   └── architecture.md
├── .github/workflows/      # CI/CD
│   └── ci.yml
├── xmake.lua               # xmake 构建配置
├── CMakeLists.txt           # CMake 构建配置
└── config.xlings            # xlings 工具链配置
```

## 快速开始

```cpp
import std;
import mcpplibs.primitives;

int main() {
    static_assert(mcpplibs::primitives::std_integer<int>);
    return 0;
}
```

## 安装与配置

```bash
xlings install
```

## 构建与运行

**使用 xmake**

```bash
xmake build                 # 构建库
xmake run basic             # 运行基础示例
xmake run primitives_test   # 运行测试
```

**使用 CMake**

```bash
cmake -B build -G Ninja
cmake --build build
ctest --test-dir build
```

## 集成到构建工具

### xmake

```lua
add_repositories("mcpplibs-index https://github.com/mcpplibs/mcpplibs-index.git")

add_requires("templates")

target("myapp")
    set_kind("binary")
    set_languages("c++23")
    add_files("main.cpp")
    add_packages("templates")
    set_policy("build.c++.modules", true)
```

## 相关链接

- [mcpp-style-ref | 现代C++编码/项目风格参考](https://github.com/mcpp-community/mcpp-style-ref)
- [mcpplibs/cmdline | 命令行解析库](https://github.com/mcpplibs/cmdline)
- [mcpp社区官网](https://mcpp.d2learn.org)
- [mcpp | 现代C++爱好者论坛](https://mcpp.d2learn.org/forum)
- [入门教程: 动手学现代C++](https://github.com/Sunrisepeak/mcpp-standard)
