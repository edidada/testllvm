# testllvm 仓库代码功能介绍

## 仓库定位

这是一个基于 LLVM 18 的 C++ 学习与演示项目（不是库、也不是完整编译器）。它用一组互相独立的小程序，演示 LLVM 核心 API 的典型用法：

- 在运行时用 `IRBuilder` 构造 LLVM IR；
- 通过 ORC `LLJIT` 把 IR 即时编译（JIT）成本机代码并调用；
- 用 legacy `FunctionPassManager` 运行优化 Pass，观察优化前后的 IR 变化；
- 使用 LLVM ADT / Support 里的常用工具类（`StringRef`、`MemoryBuffer`、`raw_ostream`、版本宏等）。

> 注意：代码依赖 LLVM 18 的新接口（例如 `ExecutorAddr::toPtr<T>()`、`StringRef::contains()`），LLVM 14 无法通过编译，README 中已注明「llvm18 可以，llvm14 不行」。

## 目录结构

```
.
├── CMakeLists.txt              # 构建配置，生成 5 个可执行目标
├── README.md                  # 运行输出片段（JIT 与优化 Pass 示例）
├── CMakeSettings.json         # Visual Studio / Ninja 本地配置
├── .gitignore
├── .github/workflows/         # GitHub Actions 多平台 CMake 构建
├── docs/
│   └── overview.md            # 本文档
└── src/
    ├── main.cpp               # -> testllvm    （ORC LLJIT 即时编译）
    ├── testpass.cpp           # -> testpass    （单 Pass：指令组合）
    ├── testpass2.cpp          # -> testpass2   （多 Pass 组合优化）
    ├── adt.cpp                # -> adt         （StringRef / MemoryBuffer 文本处理）
    └── adt_string.cpp         # -> adt_string  （版本宏 + StringRef / MemoryBuffer 基础）
```

## 构建方式

`CMakeLists.txt` 使用 `find_package(LLVM REQUIRED CONFIG)`，并分别链接 `core`、`support`、`orcjit`、`native`（Linux 还额外链接 `executionengine`）等组件。

- Windows：LLVM 路径被硬编码为 `C:/LLVM/clang+llvm-18.1.8-x86_64-pc-windows-msvc`，并尝试链接 VS2022 的 DIA SDK（`diaguids.lib`）以支持 PDB；同时链接 `advapi32`、`shell32`。
- Linux / macOS：依赖系统安装的 LLVM（`LLVM_DIR` 指向 `lib/cmake/llvm`），CMake 打印 `LLVM_PACKAGE_VERSION`、`LLVM_DEFINITIONS` 等信息。

五个源文件各自编译成独立的可执行文件，彼此之间没有共享代码。

## 可执行程序逐一说明

### 1. `testllvm`（src/main.cpp）—— ORC LLJIT 即时编译

演示「运行时生成 IR → JIT 编译 → 取函数指针调用」的完整流程：

1. `InitLLVM` + `InitializeNativeTarget`/`AsmPrinter` 初始化本地目标；
2. `LLJITBuilder().create()` 创建 JIT 实例；
3. 新建 `LLVMContext` 与名为 `jit_module` 的 `Module`，并套用 JIT 的 DataLayout；
4. 用 `IRBuilder` 生成 `int add(int, int)`：一个 `EntryBlock`，执行 `%sum = add i32 %0, %1` 并 `ret`；
5. `M->print(errs(), nullptr)` 打印生成的 IR；
6. 将 `Module` + `ThreadSafeContext` 包装成 `ThreadSafeModule` 加入 JIT；
7. `JIT->lookup("add")` 得到符号，用 LLVM 18 的 `toPtr<AddFunction>()` 转成函数指针；
8. 调用 `add(3, 4)`，输出 `3 + 4 = 7`。

### 2. `testpass`（src/testpass.cpp）—— 单个优化 Pass

- `createTestFunction()` 构造 `int @test(int a)`，故意写成冗余计算 `(a + 0) * 1`；
- `printFunction()` 逐基本块、逐指令打印 IR；
- `verifyFunction()` 在优化前后各做一次合法性校验；
- 用 `legacy::FunctionPassManager` 仅添加 `createInstructionCombiningPass()`（指令组合），运行后 `+0`、`*1` 被消除；
- 对比输出「Before / After Instruction Combining」。

### 3. `testpass2`（src/testpass2.cpp）—— 多 Pass 组合优化

- `createComplexFunction()` 构造 `int @complex_calc(int a, int b)`：包含 `a+0`、`*1`、`b-0`、`*1` 等冗余运算，一个恒真条件 `icmp eq a, a` 分出 `then` / `else` / `merge` 三个基本块，并用 `PHINode` 合并；
- 依次添加 4 个优化 Pass：
  - `createInstructionCombiningPass()`（指令组合）
  - `createReassociatePass()`（表达式重关联）
  - `createGVNPass()`（全局值编号，消除冗余）
  - `createCFGSimplificationPass()`（控制流简化）
- 优化后，整段冗余计算与死分支被折叠为一条 `%2 = add i32 %1, %0; ret i32 %2`（见 README 中的输出）；
- 同样在优化前后做 `verifyFunction()` 校验，最后打印 `All optimizations completed successfully!`。

### 4. `adt`（src/adt.cpp）—— ADT 文本处理演示

接收一个文件名参数：

- `MemoryBuffer::getFile()` 读取文件，用 LLVM 17+ 的 `Expected`/`Error` 风格判错；
- `processFileContent(StringRef)` 演示 `StringRef` 的常用方法：`count('\n')`（统计行数）、`split('\n')`（取首行）、`find("LLVM")`、`contains("error")`、`trim()`；
- 按行遍历并打印包含 `error` / `Error` 的行。

### 5. `adt_string`（src/adt_string.cpp）—— 版本宏与基础 API

- 通过 `llvm/Config/llvm-config.h` 打印 `LLVM_VERSION_STRING`、`LLVM_VERSION_MAJOR`、`LLVM_VERSION_MINOR`、`LLVM_VERSION_PATCH`；
- 演示 `StringRef` 的 `size()`、`contains()`；
- `MemoryBuffer::getFile("example.txt")` 读取文件并打印大小与前 50 个字符（文件不存在时通过 `getError().message()` 报错，需要当前目录存在 `example.txt`）。

## 持续集成

`.github/workflows/` 下是 GitHub Actions 的 CMake 多平台构建配置（另有单平台与 Windows 专用 workflow）：

- 触发分支：`multiple` 的 `push` / `pull_request`；
- 矩阵：Linux（`gcc`、`clang`）× `Release` / `Debug`；
- 各平台安装 LLVM 18（Linux `apt install llvm`、macOS `brew install llvm@18`、Windows `choco install llvm --version=18.1.8`），然后执行标准 CMake 配置 / 构建 / `ctest` 三步。

## 小结

| 目标 | 演示主题 | 关键 API |
| --- | --- | --- |
| `testllvm` | JIT 即时编译与执行 | `LLJIT`、`IRBuilder`、`ThreadSafeModule`、`ExecutorAddr::toPtr` |
| `testpass` | 单 Pass 优化 | `legacy::FunctionPassManager`、`InstructionCombiningPass`、`verifyFunction` |
| `testpass2` | 多 Pass 组合优化 | `InstCombine` / `Reassociate` / `GVN` / `CFGSimplification`、`PHINode` |
| `adt` | 文本处理工具类 | `StringRef`、`MemoryBuffer` |
| `adt_string` | 版本信息 + 基础工具类 | `LLVM_VERSION_*`、`StringRef`、`MemoryBuffer` |
