# JsonStruct 性能基准测试入门指南

## 快速开始 (3分钟上手)

### 1. 准备工作
- 确保系统已安装 Visual Studio 2022 或更高版本
- 确保已安装 CMake 3.16 或更高版本
- 确保已安装 Git (用于下载依赖库)

### 2. 一键构建和运行
```powershell
# 进入基准测试目录
cd benchmarks

# 构建并运行所有测试
.\build_benchmarks.ps1 -RunTests
```

### 3. 查看结果
测试完成后，会在 `build` 目录中生成结果文件：
- `performance_results.json` - 详细性能数据
- `benchmark_results.txt` - 文本格式报告

## 常用场景

### 场景1：快速性能检测
```powershell
# 仅运行基础性能测试，不对比其他库
.\build_benchmarks.ps1 -WithRealLibraries:$false -RunTests
```

### 场景2：完整性能对比
```powershell
# 下载并对比主流JSON库
.\build_benchmarks.ps1 -WithRealLibraries -RunTests
```

### 场景3：持续集成
```powershell
# 适合CI环境的快速测试
.\build_benchmarks.ps1 -WithRealLibraries:$false
.\run_benchmarks.ps1 -Simple
```

### 场景4：内存分析
```powershell
.\build_benchmarks.ps1
.\run_benchmarks.ps1 -Memory
```

## 故障排除

### 问题1：构建失败
**症状**: CMake 配置失败
**解决**: 
1. 检查 CMake 版本: `cmake --version`
2. 检查 Visual Studio 是否正确安装
3. 使用管理员权限运行 PowerShell

### 问题2：网络问题
**症状**: 第三方库下载失败
**解决**: 
```powershell
# 跳过第三方库下载
.\build_benchmarks.ps1 -WithRealLibraries:$false -RunTests
```

### 问题3：权限问题
**症状**: 可执行文件无法运行
**解决**: 
```powershell
# 以管理员身份运行 PowerShell
# 或设置执行策略
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

## 理解测试结果

### 性能指标含义
- **吞吐量 (ops/sec)**: 每秒处理的操作数，越高越好
- **延迟 (ms)**: 单次操作耗时，越低越好
- **内存使用 (MB)**: 峰值内存占用，越低越好

### 对比结果解读
- **相对性能**: JsonStruct 相对于其他库的性能比例
- **优势场景**: JsonStruct 表现最佳的使用场景
- **劣势场景**: 需要优化的场景

## 自定义测试

### 添加新的测试用例
1. 在 `tests/` 目录添加新的 `.cpp` 文件
2. 更新 `CMakeLists_independent.txt`
3. 重新构建测试

### 修改测试参数
编辑测试源文件中的配置常量：
```cpp
// 例如修改测试数据大小
const int TEST_DATA_SIZE = 10000;
const int TEST_ITERATIONS = 1000;
```

## 最佳实践

### 1. 性能测试环境
- 关闭其他应用程序
- 使用 Release 构建
- 多次运行取平均值

### 2. 数据收集
- 保存测试结果文件
- 记录测试环境信息
- 对比不同版本的结果

### 3. 问题调试
- 使用 Debug 构建调试
- 查看详细的错误信息
- 对比简单测试和复杂测试

## 进阶使用

### 使用不同的编译器
```powershell
# 使用 MinGW
.\build_benchmarks.ps1 -Generator "MinGW Makefiles"

# 使用 Ninja
.\build_benchmarks.ps1 -Generator "Ninja"
```

### 性能分析
```powershell
# 使用 Visual Studio 分析器
.\build_benchmarks.ps1 -BuildType RelWithDebInfo
# 然后在 Visual Studio 中打开项目进行分析
```

### 交叉编译
```powershell
# 为不同平台构建
cmake -DCMAKE_TOOLCHAIN_FILE=your_toolchain.cmake ..
```

## 集成到 CI/CD

### GitHub Actions 示例
```yaml
- name: Run Benchmarks
  run: |
    cd benchmarks
    .\build_benchmarks.ps1 -WithRealLibraries:$false -RunTests
    
- name: Upload Results
  uses: actions/upload-artifact@v2
  with:
    name: benchmark-results
    path: benchmarks/build/performance_results.json
```

### 性能回归检测
```powershell
# 比较不同版本的性能
.\build_benchmarks.ps1 -RunTests
# 保存结果并与基线版本对比
```

## 获取帮助

如果遇到问题，请：
1. 查看脚本帮助: `.\build_benchmarks.ps1 -Help`
2. 查看构建日志中的错误信息
3. 检查 `benchmarks/README.md` 获取详细文档
4. 在项目仓库中提交 Issue

## 总结

这个独立的性能基准测试系统让您可以：
- ✅ 快速评估 JsonStruct 性能
- ✅ 与主流 JSON 库对比
- ✅ 监控性能回归
- ✅ 独立于主项目运行
- ✅ 适合 CI/CD 集成

现在您可以开始性能测试之旅了！
