# JsonStruct 性能基准测试

这是一个独立的性能基准测试套件，用于测试和对比 JsonStruct 与其他主流 JSON 库的性能。

## 目录结构

```
benchmarks/
├── CMakeLists_independent.txt  # 独立的 CMake 配置文件
├── build_benchmarks.ps1        # Windows 构建脚本
├── README.md                   # 本文档
└── build/                      # 构建输出目录
```

## 特性

- **完全独立**: 与主项目完全分离，可以独立构建和运行
- **多库对比**: 支持与 nlohmann/json、RapidJSON、JsonCpp 等主流库对比
- **灵活配置**: 可选择是否启用真实库对比、Qt支持等
- **全面测试**: 包含性能、内存使用、对比测试等多个维度

## 快速开始

### 1. 默认构建（推荐）

```powershell
cd benchmarks
.\build_benchmarks.ps1
```

### 2. 构建并运行测试

```powershell
.\build_benchmarks.ps1 -RunTests
```

### 3. 查看帮助信息

```powershell
.\build_benchmarks.ps1 -Help
```

## 构建选项

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-BuildType` | 构建类型 (Release, Debug, RelWithDebInfo) | Release |
| `-Generator` | CMake 生成器 | Visual Studio 17 2022 |
| `-WithRealLibraries` | 启用真实JSON库对比测试 | true |
| `-WithQt` | 启用Qt支持 | false |
| `-Clean` | 清理构建目录 | false |
| `-RunTests` | 构建完成后运行测试 | false |

## 使用示例

### 基本使用

```powershell
# 默认构建
.\build_benchmarks.ps1

# Debug 构建
.\build_benchmarks.ps1 -BuildType Debug

# 清理构建
.\build_benchmarks.ps1 -Clean

# 构建并运行测试
.\build_benchmarks.ps1 -RunTests
```

### 高级配置

```powershell
# 不使用真实JSON库（仅测试JsonStruct性能）
.\build_benchmarks.ps1 -WithRealLibraries:$false

# 启用Qt支持
.\build_benchmarks.ps1 -WithQt

# 完整配置
.\build_benchmarks.ps1 -BuildType Release -WithRealLibraries -WithQt -RunTests
```

## 测试类型

### 1. 性能基准测试 (performance_benchmark.exe)
- 测试 JsonStruct 的基础性能
- 包含序列化、反序列化、查询等操作
- 生成详细的性能报告

### 2. 内存使用测试 (memory_benchmark.exe)
- 测试内存分配和释放
- 监控内存峰值使用量
- 检测内存泄漏

### 3. 简单对比测试 (comparison_benchmark_simple.exe)
- 不依赖外部库的对比测试
- 测试 JsonStruct 的相对性能
- 快速执行，适合持续集成

### 4. 完整对比测试 (comparison_benchmark.exe)
- 与真实 JSON 库对比
- 包含 nlohmann/json、RapidJSON、JsonCpp
- 提供详细的性能对比报告

### 5. 性能报告生成器 (performance_reporter.exe)
- 整合所有测试结果
- 生成综合性能报告
- 支持多种输出格式

## 手动构建

如果您不想使用脚本，也可以手动构建：

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -DBUILD_REAL_JSON_COMPARISON=ON ..

# 构建
cmake --build . --config Release --target build_all_benchmarks

# 运行测试
.\Release\performance_benchmark.exe
.\Release\memory_benchmark.exe
.\Release\comparison_benchmark.exe
```

## 输出文件

运行测试后，会在构建目录中生成以下文件：

- `performance_results.json` - 性能测试结果（JSON格式）
- `benchmark_results.txt` - 基准测试结果（文本格式）
- `memory_results.txt` - 内存使用测试结果

## 故障排除

### 1. CMake 配置失败
- 确保已安装 CMake 3.16 或更高版本
- 检查 C++ 编译器是否正确配置

### 2. 第三方库下载失败
- 检查网络连接
- 可以使用 `-WithRealLibraries:$false` 跳过第三方库

### 3. 构建失败
- 检查 C++ 编译器是否支持 C++17
- 确保有足够的磁盘空间
- 查看详细的构建日志

### 4. 测试执行失败
- 确保没有防病毒软件干扰
- 检查可执行文件权限
- 查看测试输出中的错误信息

## 与主项目的关系

- **完全独立**: 此基准测试套件可以独立于主项目运行
- **源码共享**: 使用相同的 JsonStruct 源代码
- **配置隔离**: 拥有独立的 CMake 配置，不影响主项目构建
- **结果独立**: 测试结果和构建产物完全隔离

# Benchmarks for JsonStruct

This directory contains benchmark tests for the JsonStruct library. These benchmarks are designed to measure the performance of various JSON operations, including serialization, deserialization, and pipeline processing.

## Structure
- `tests/`: Contains individual benchmark test files.
- `CMakeLists.txt`: Configuration for building and running benchmarks.
- `run_benchmarks.ps1`: Script for executing benchmarks on Windows.

## Running Benchmarks
1. Build the benchmarks:
   ```powershell
   cmake -S . -B build
   cmake --build build
   ```

2. Run the benchmarks:
   ```powershell
   .\run_benchmarks.ps1
   ```

## Adding New Benchmarks
To add a new benchmark:
1. Create a new `.cpp` file in the `tests/` directory.
2. Update `CMakeLists.txt` to include the new file.
3. Implement the benchmark logic using the existing framework.

## 贡献指南

1. 添加新的基准测试时，请更新 CMake 配置
2. 确保新测试与现有测试格式一致
3. 更新本文档说明新功能
4. 测试在不同平台上的兼容性

## 许可证

本基准测试套件遵循与主项目相同的许可证。
