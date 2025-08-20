# Fuzzing Tests for json_struct

本目录包含用于 `json_struct` 库的模糊测试（fuzzing tests），使用 libFuzzer 来发现潜在的解析器漏洞和崩溃。

## 前置条件

1. **Clang 编译器**：模糊测试需要 Clang 编译器，因为它内置了 libFuzzer 支持。
2. **AddressSanitizer 和 UndefinedBehaviorSanitizer**：这些工具会帮助检测内存错误和未定义行为。

## 构建和运行

### 1. 配置项目

使用 Clang 编译器并启用 fuzzing：

```bash
cmake -S . -B build \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DBUILD_FUZZER=ON \
    -DCMAKE_BUILD_TYPE=Debug
```

### 2. 构建 Fuzzer

```bash
cmake --build build --target json_fuzzer
```

### 3. 运行 Fuzzer

有三种运行方式：

#### 快速测试（10,000 次迭代）
```bash
cmake --build build --target run_fuzzer
```

#### 连续运行（直到手动停止）
```bash
cmake --build build --target run_fuzzer_continuous
```

#### 使用种子语料库
```bash
cmake --build build --target run_fuzzer_corpus
```

### 4. 直接运行 Fuzzer

您也可以直接运行 fuzzer 可执行文件：

```bash
# 基本运行
./build/tests/fuzzing/json_fuzzer

# 指定参数
./build/tests/fuzzing/json_fuzzer -max_len=4096 -timeout=10 -runs=50000

# 使用语料库目录
./build/tests/fuzzing/json_fuzzer corpus/ -max_len=2048
```

## Fuzzer 参数说明

- `-max_len=N`：限制输入的最大长度（字节）
- `-timeout=N`：单次测试的超时时间（秒）
- `-runs=N`：运行指定次数后退出
- `-dict=file`：使用字典文件来引导 fuzzer 生成更有针对性的输入
- `-minimize_crash=1`：最小化发现的崩溃输入

## 种子语料库

`seed_corpus/` 目录包含一些初始的测试用例，包括：

- `valid_simple.json`：简单的有效 JSON
- `valid_nested.json`：嵌套结构的有效 JSON  
- `valid_array.json`：数组格式的有效 JSON
- `invalid_incomplete.json`：不完整的无效 JSON
- `invalid_malformed.json`：格式错误的无效 JSON

这些文件帮助 fuzzer 了解输入格式，从而生成更有效的测试用例。

## 发现崩溃时的处理

如果 fuzzer 发现了导致崩溃的输入，它会：

1. 在当前目录保存崩溃输入文件（如 `crash-xxx`）
2. 显示崩溃的堆栈跟踪信息
3. 停止执行

您可以使用以下方式重现崩溃：

```bash
./build/tests/fuzzing/json_fuzzer crash-xxx
```

## 最佳实践

1. **长时间运行**：模糊测试的效果随时间增加，建议长时间运行以发现更深层的问题。
2. **监控覆盖率**：使用 `-print_coverage=1` 参数监控代码覆盖率。
3. **定期清理**：模糊测试会产生大量文件，定期清理测试目录。
4. **CI 集成**：在持续集成中运行短时间的模糊测试作为回归测试。

## 故障排除

### 编译器不支持
```
CMake Warning: Fuzzing requires Clang compiler. Current compiler: GCC.
```
解决方案：安装 Clang 并在 CMake 配置时指定编译器。

### libFuzzer 不可用
```
CMake Warning: Compiler does not support -fsanitize=fuzzer flag
```
解决方案：确保使用足够新的 Clang 版本（推荐 9.0+）。

### 运行时错误
如果遇到运行时链接错误，确保系统中的 Clang 安装完整，包含所有必要的运行时库。