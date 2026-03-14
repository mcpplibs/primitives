# mcpplibs primitives

> C++23 模块化原语库 - `import mcpplibs.primitive;`

本仓库实现了底层强类型 primitive 基础设施（traits、policy、underlying 类型分类），供上层 `Integer`/`Floating`/`Boolean` 等封装使用。

## 特性

- **C++23 模块** — `import mcpplibs.templates;`
- **双构建系统** — 同时支持 xmake 和 CMake
- **CI/CD** — GitHub Actions 多平台构建（Linux / macOS / Windows）
- **标准化结构** — 遵循 [mcpp-style-ref](https://github.com/mcpp-community/mcpp-style-ref) 编码规范
- **开箱即用** — 包含示例、测试和架构文档

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
import mcpplibs.primitive;

int main() {
    static_assert(mcpplibs::primitive::std_integer<int>);
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
