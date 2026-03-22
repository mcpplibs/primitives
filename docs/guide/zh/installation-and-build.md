# 安装与构建

## 环境要求

- 支持 C++23 的编译器
  - GCC >= 14
  - Clang >= 17
  - MSVC >= 19.34（MSVC 版本号 >= 1934）
- 构建工具
  - 推荐：xmake
  - 或：CMake + Ninja

## 使用 xmake 构建

构建库：

```bash
xmake build mcpplibs-primitives
```

运行示例：

```bash
xmake run ex01_default_arithmetic
```

运行测试：

```bash
xmake run primitives_test
```

兼容别名：

```bash
xmake run basic
```

`basic` 对应 `ex01_default_arithmetic`。

## 使用 CMake 构建

配置并构建：

```bash
cmake -B build -G Ninja
cmake --build build --target mcpplibs-primitives
```

构建示例与测试：

```bash
cmake --build build --target ex01_default_arithmetic
cmake --build build --target basic_tests
ctest --test-dir build --output-on-failure
```

## 示例目标

可独立构建/运行的示例目标：

- `ex01_default_arithmetic`
- `ex02_type_policy`
- `ex03_value_policy`
- `ex04_error_policy`
- `ex05_concurrency_policy`
- `ex06_custom_underlying`
- `ex07_custom_policy`
- `ex08_custom_operation`

## 常见构建问题

### 1) 未启用 C++23 / 模块能力

请确认工程使用 C++23，且编译器支持 C++ modules。

### 2) 编译器版本过低

请使用本文档列出的最低版本，否则可能在配置阶段或模块编译阶段失败。

### 3) 测试依赖获取失败

测试目标依赖 GoogleTest。请确保 xmake 包管理可用，或 CMake 侧可访问配置的拉取源。

## 下一步

- 继续阅读：[快速上手](./quick-start.md)
- API 细节： [../../api/zh/README.md](../../api/zh/README.md)

