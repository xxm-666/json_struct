# JsonStruct 测试运行器

## 概述

`test_all_runner` 是一个自动化测试运行器，可以运行项目中的所有测试用例并生成详细的报告。

## 功能特性

- ✅ 自动发现并运行所有测试可执行文件
- ✅ 实时显示测试执行进度
- ✅ 解析每个测试的详细结果（通过/失败数量）
- ✅ 计算测试执行时间
- ✅ 生成完整的测试摘要报告
- ✅ 显示失败测试的详细输出
- ✅ 跨平台支持（Windows/Linux）

## 使用方法

### 构建测试运行器

```bash
# 构建所有测试和测试运行器
cmake --build . --target build_all_tests

# 或者单独构建测试运行器
cmake --build . --target test_all_runner
```

### 运行所有测试

```bash
# 方法1：直接运行可执行文件
cd build/Debug  # 或 build/Release
./test_all_runner.exe  # Windows
./test_all_runner      # Linux

# 方法2：使用CMake目标（推荐）
cmake --build . --target run_all_tests
```

## 输出示例

```
=== JsonStruct Test Suite Runner ===
Starting automated test execution...

Running test_core_functionality.exe... PASSED (0.06s)
Running test_json_auto.exe... PASSED (0.05s)
Running test_qt_types_new.exe... PASSED (0.03s)
Running test_enhanced_features.exe... PASSED (0.06s)
Running test_custom_types.exe... PASSED (0.04s)
Running test_json_parsing.exe... PASSED (0.04s)
Running test_error_recovery.exe... PASSED (0.05s)
Running test_performance_struct.exe... PASSED (0.06s)
Running test_special_numbers.exe... PASSED (0.05s)
Running test_json_pointer.exe... PASSED (0.06s)
Running test_basic_functionality.exe... PASSED (0.03s)
Running test_complex_structures.exe... PASSED (0.01s)
Running test_error_handling.exe... PASSED (0.03s)

=== Test Summary ===
================================================================================
test_core_functionality.exe    | PASS   | 35  tests | 0.06  s
test_json_auto.exe             | PASS   | 42  tests | 0.05  s
test_qt_types_new.exe          | PASS   | 15  tests | 0.03  s
test_enhanced_features.exe     | PASS   | 28  tests | 0.06  s
test_custom_types.exe          | PASS   | 20  tests | 0.04  s
test_json_parsing.exe          | PASS   | 18  tests | 0.04  s
test_error_recovery.exe        | PASS   | 12  tests | 0.05  s
test_performance_struct.exe    | PASS   | 65  tests | 0.06  s
test_special_numbers.exe       | PASS   | 79  tests | 0.05  s
test_json_pointer.exe          | PASS   | 25  tests | 0.06  s
test_basic_functionality.exe   | PASS   | 10  tests | 0.03  s
test_complex_structures.exe    | PASS   | 18  tests | 0.01  s
test_error_handling.exe        | PASS   | 8   tests | 0.03  s
================================================================================
Total Test Suites: 13
Suites Passed: 13
Suites Failed: 0

Total Individual Tests: 375
Individual Tests Passed: 375
Individual Tests Failed: 0

Total Execution Time: 0.58 seconds

🎉 ALL TEST SUITES PASSED! 🎉
```

## 包含的测试套件

当前测试运行器会自动运行以下测试：

1. **test_core_functionality** - 核心功能测试
2. **test_json_auto** - JSON自动序列化测试  
3. **test_qt_types_new** - Qt类型支持测试
4. **test_enhanced_features** - 增强功能测试
5. **test_custom_types** - 自定义类型测试
6. **test_json_parsing** - JSON解析测试
7. **test_error_recovery** - 错误恢复测试
8. **test_performance_struct** - 性能结构测试
9. **test_special_numbers** - 特殊数字处理测试
10. **test_json_pointer** - JSON指针测试
11. **test_basic_functionality** - 基础功能测试
12. **test_complex_structures** - 复杂结构测试
13. **test_error_handling** - 错误处理测试

## 失败诊断

当有测试失败时，运行器会：

1. 在摘要中标记失败的测试套件
2. 显示失败测试的完整输出
3. 提供具体的失败位置和原因

例如：
```
❌ 1 TEST SUITE(S) FAILED!

=== Failed Test Details ===

--- test_example.exe ---
=== Running Test Suite ===
Running ExampleTest_BasicOperations...
[FAIL] ExampleTest_BasicOperations (1 failures)
  - ASSERT_EQ failed: expected 5, got 3 at test_example.cpp:42
```

## 技术实现

- **跨平台进程管理**: 使用Windows API或POSIX系统调用
- **输出捕获**: 通过管道捕获子进程的标准输出
- **解析引擎**: 智能解析测试框架的标准输出格式
- **时间测量**: 高精度计时器测量执行时间

## 自定义配置

如需添加新的测试套件，请在 `test_all_runner.cpp` 的 `testExecutables` 向量中添加相应的可执行文件名：

```cpp
testExecutables = {
    "test_core_functionality.exe",
    "test_json_auto.exe",
    // ... 现有测试 ...
    "your_new_test.exe"  // 添加新测试
};
```

## 故障排除

### 常见问题

1. **测试可执行文件未找到**
   - 确保所有测试都已正确编译
   - 检查当前工作目录是否正确（应在build/Debug或build/Release中）

2. **解析错误**
   - 确保测试使用标准的测试框架输出格式
   - 检查测试输出是否包含 "Total: X, Passed: Y, Failed: Z" 格式

3. **权限问题**
   - 确保测试可执行文件有执行权限
   - 在Linux/Mac上可能需要 `chmod +x` 命令

### 调试模式

如需查看详细的解析过程，可以修改源代码中的错误输出部分，或者直接运行单个测试进行调试。
