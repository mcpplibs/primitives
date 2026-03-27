# Primitives 下一步实现计划清单（2026-03-27）

## 当前状态

- [ ] M1 完成：C API 互操作层（双向）
- [x] M2 完成：Concept/Traits 体系增强与元信息 API
- [x] M3 完成：显式转换层（任意策略组适用）
- [x] M4 完成：算法层（traits 先行，以 max/min 为起点）

## 持续约束

- 先定义稳定 API 边界，再落地实现。
- 每个能力都配套 `tests + examples + docs`。
- 高风险转换默认禁止隐式触发，只允许显式 API。
- 明确分层职责：`algorithms` 负责值域/排序/边界元信息与值算法；`conversion` 负责显式类型变换。
- 明确依赖方向：`conversion` 可依赖 `algorithms`，`algorithms` 不反向依赖 `conversion`。
- C ABI 只暴露稳定、可版本化的边界；不直接泄露模板、模块实现细节或未稳定的策略矩阵。

## M1. C API 互操作层（最后剩余部分）

### M1-0. 范围冻结与接口裁剪

- [ ] 冻结 `M1 v1` 范围：仅支持 `int32_t` / `uint32_t` / `double` 三类基础值。
- [ ] 冻结 `M1 v1` 能力面：仅提供 `add` / `sub` / `mul` / `div`、基础比较、显式转换、版本查询。
- [ ] 明确第一版不纳入 ABI 的内容：`char` 系列、`long double`、自定义 underlying、完整策略组合、完整 operator 集。
- [ ] 统一第一版语义映射为“显式调用 + checked 路径 + 错误码返回”，不允许异常跨越 C ABI。
- [ ] 输出一份最小公共命名规范：`mcpplibs_primitives_{type}_{op}`。

### M1-1. 稳定 C ABI 与头文件

- [ ] 新增最小公共头文件：`include/mcpplibs/primitives/c_api.h`。
- [ ] 定义导出宏：`MCPPLIBS_PRIMITIVES_C_API`、`MCPPLIBS_PRIMITIVES_EXTERN_C`。
- [ ] 定义 C API 版本宏：`MCPPLIBS_PRIMITIVES_C_API_VERSION_MAJOR` / `MINOR` / `PATCH`。
- [ ] 定义统一错误码枚举，至少覆盖：
- [ ] `ok`
- [ ] `invalid_argument`
- [ ] `overflow`
- [ ] `underflow`
- [ ] `divide_by_zero`
- [ ] `domain_error`
- [ ] `unsupported`
- [ ] `internal_error`
- [ ] 定义统一消息缓冲区协议：`char* message` + `size_t message_size`，支持空指针与零长度缓冲区。
- [ ] 定义统一返回约定：函数返回错误码，业务结果通过 `out` 参数返回。
- [ ] 提供版本查询函数与能力探测函数，确保后续 ABI 演进可协商。

### M1-2. C -> primitives 调用桥

- [ ] 在 `src/c_api/` 下建立实现目录。
- [ ] 拆分实现文件，建议至少包含：
- [ ] `src/c_api/abi.cpp`
- [ ] `src/c_api/arithmetic.cpp`
- [ ] `src/c_api/compare.cpp`
- [ ] `src/c_api/conversion.cpp`
- [ ] 以公开模块能力为唯一实现入口：只调用 `mcpplibs.primitives` 已导出的 `types`、`operations`、`conversion`。
- [ ] 为每个 ABI 入口补齐参数校验：`out == nullptr`、非法 buffer、除零、溢出、域错误。
- [ ] 建立统一错误翻译层：将 `policy::error::kind` 与转换风险映射到 C 错误码。
- [ ] 在 ABI 边界统一拦截异常并映射为 `internal_error`，禁止任何 C++ 异常穿透到 C 侧。
- [ ] 保证所有导出函数不暴露 `std::expected`、模板实例、模块内部类型或 STL 容器布局。

### M1-3. primitives -> 外部 C API 适配桥

- [ ] 新增 C++ 适配层，建议文件：
- [ ] `src/c_api/adapter.cppm`
- [ ] `src/c_api/adapter.cpp`
- [ ] 用“函数表 + `void* context`”建模外部 C API，而不是散落裸函数指针。
- [ ] 第一版只适配与 `M1 v1` 对称的能力：四则、比较、显式 cast。
- [ ] 明确适配协议：
- [ ] 适配层不拥有 `context`
- [ ] 回调必须是 `noexcept` 语义约束
- [ ] 返回码必须可映射到 `policy::error::kind`
- [ ] 外部 C API 回传值必须再经过 primitives 的范围与错误检查，不能绕开现有风险路径。
- [ ] 为适配层建立最小公共抽象，避免未来直接把外部 C API 绑定死到具体函数签名。

### M1-4. 构建系统与安装集成

- [ ] 更新 `CMakeLists.txt`：将 `project(... LANGUAGES CXX)` 扩展为 `project(... LANGUAGES C CXX)`。
- [ ] 为 C API 头文件添加安装与导出规则，确保 `include/mcpplibs/primitives/c_api.h` 进入安装产物。
- [ ] 决定 C ABI 的产物形态：
- [ ] 方案 A：并入现有 `mcpplibs-primitives` 静态库
- [ ] 方案 B：新增单独目标 `mcpplibs-primitives-capi`
- [ ] 更新 `xmake.lua`，确保 C 示例与 C 测试能参与构建。
- [ ] 更新 `examples/CMakeLists.txt` 与 `examples/xmake.lua`，纳入 C 示例和 C++ 适配示例。
- [ ] 更新 `tests/CMakeLists.txt`、`tests/basic/CMakeLists.txt`、`tests/xmake.lua`，纳入 C/C++ 混合验证目标。

### M1-5. 测试、示例与文档

- [ ] 新增纯 C 示例：`examples/c/basic.c`。
- [ ] 新增 C++ 调用 C API 示例：`examples/cpp/use_c_api.cpp`。
- [ ] 新增纯 C smoke test，验证：
- [ ] 头文件可被 C 编译器独立包含
- [ ] C 目标可链接库产物
- [ ] `int32 add`、`double div` 成功路径可运行
- [ ] 除零或溢出错误路径可观测
- [ ] 新增 C++ 适配层测试，验证：
- [ ] 外部 C API 成功路径映射正确
- [ ] 错误码映射正确
- [ ] 空指针/非法参数会被拒绝
- [ ] 回调异常或非法返回不会引入未定义行为
- [ ] 新增 ABI 边界测试，覆盖消息 buffer 截断、空 buffer、空 `out` 参数、版本查询。
- [ ] 更新 `README.md` / `README.zh.md`，补充 C API 简介、边界约束与示例入口。
- [ ] 更新 `docs/api/en` / `docs/api/zh`，补充错误模型、版本策略和双向互操作说明。

### M1-6. 收口与验收

- [ ] 纯 C 工程可独立包含 `c_api.h`、成功链接并调用基础能力。
- [ ] C API 成功路径、错误路径、异常隔离路径全部可回归测试。
- [ ] C++ 对外部 C API 的适配调用行为可控、无未定义行为。
- [ ] ABI 命名、导出宏、版本查询、错误码模型在文档中有明确承诺。
- [ ] `M1 v1` 中未纳入 ABI 的能力在文档中明确标注为“未稳定/不承诺”。

## 建议推进顺序（按 PR / 工作包）

1. `M1-0 + M1-1`：先冻结第一版范围，并提交 `c_api.h` 与 ABI 契约。
2. `M1-2`：实现 C 调用 primitives 的最小闭环，先跑通 `int32_t` 和 `double`。
3. `M1-4`：补齐 CMake/xmake/安装规则，让 C 目标进入正式构建链路。
4. `M1-5`：补齐 `examples/c/basic.c`、`examples/cpp/use_c_api.cpp` 和混合测试。
5. `M1-3`：实现 primitives 对外部 C API 的适配桥，并补齐适配测试。
6. `M1-6`：统一收口文档、版本策略与验收项，准备关闭 M1。

## 已完成里程碑归档

- [x] M2 已完成，后续不再作为阻塞项跟踪。
- [x] M3 已完成，后续仅在 M1 适配中复用已有显式转换能力。
- [x] M4 已完成，后续仅在 M1 实现中复用已有算法层与 traits 元信息。
