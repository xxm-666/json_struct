# JsonStruct 性能基准测试报告

## 测试环境信息

| 项目 | 值 |
|------|----|
| 操作系统 | Windows |
| 编译器 | MSVC 1944 |
| 构建类型 | Release |
| CPU架构 | x64 Architecture |
| 测试时间 | 2025-07-18 16:41:58 |
| JsonStruct版本 | 1.2.0-dev |

## 性能测试结果

### 详细基准测试数据

| 测试名称 | 迭代次数 | 平均时间(ms) | 中位数(ms) | 最小时间(ms) | 最大时间(ms) | 标准差(ms) | 吞吐量(ops/sec) |
|----------|----------|--------------|------------|--------------|--------------|------------|----------------|
| SimpleStruct序列化 | 10000 | 0.142 | 0.138 | 0.125 | 0.180 | 0.015 | 7042 |
| ComplexStruct序列化 | 5000 | 0.485 | 0.475 | 0.450 | 0.620 | 0.035 | 2062 |
| LargeStruct序列化 | 100 | 16.100 | 15.900 | 15.200 | 18.500 | 0.850 | 62 |
| SimpleStruct反序列化 | 10000 | 0.195 | 0.190 | 0.180 | 0.250 | 0.020 | 5128 |
| JSONPath简单查询 | 1000 | 2.650 | 2.600 | 2.400 | 3.100 | 0.180 | 377 |
| JSONPath复杂查询 | 500 | 9.200 | 9.000 | 8.500 | 11.200 | 0.650 | 109 |
| JSONPath递归查询 | 200 | 27.800 | 27.200 | 25.600 | 32.100 | 1.850 | 36 |

### 性能分析

#### 🚀 高性能操作 (>1000 ops/sec)

- **SimpleStruct序列化**: 7042 ops/sec
- **SimpleStruct反序列化**: 5128 ops/sec
- **ComplexStruct序列化**: 2062 ops/sec

#### ⚡ 中等性能操作 (100-1000 ops/sec)

- **JSONPath简单查询**: 377 ops/sec
- **JSONPath复杂查询**: 109 ops/sec

#### 🔍 复杂操作 (<100 ops/sec)

- **LargeStruct序列化**: 62 ops/sec
- **JSONPath递归查询**: 36 ops/sec

### 性能图表数据

```json
{
  "testResults": [
    {
      "name": "SimpleStruct序列化",
      "avgTime": 0,
      "throughput": 7042,
      "iterations": 10000
    },
    {
      "name": "ComplexStruct序列化",
      "avgTime": 0,
      "throughput": 2062,
      "iterations": 5000
    },
    {
      "name": "LargeStruct序列化",
      "avgTime": 16,
      "throughput": 62,
      "iterations": 100
    },
    {
      "name": "SimpleStruct反序列化",
      "avgTime": 0,
      "throughput": 5128,
      "iterations": 10000
    },
    {
      "name": "JSONPath简单查询",
      "avgTime": 3,
      "throughput": 377,
      "iterations": 1000
    },
    {
      "name": "JSONPath复杂查询",
      "avgTime": 9,
      "throughput": 109,
      "iterations": 500
    },
    {
      "name": "JSONPath递归查询",
      "avgTime": 28,
      "throughput": 36,
      "iterations": 200
    }
  ]
}
```

### 结论和建议

#### 📊 性能总结

- **平均吞吐量**: 2e+03 ops/sec
- **最快操作**: SimpleStruct序列化 (7e+03 ops/sec)
- **最慢操作**: JSONPath递归查询 (4e+01 ops/sec)
- **性能比**: 2e+02:1

#### 🎯 优化建议

1. **序列化优化**:
   - 简单结构的序列化性能最佳
   - 对于大型复杂结构，考虑分批处理
   - 使用移动语义减少拷贝开销

2. **JSONPath查询优化**:
   - 简单路径查询性能优于复杂过滤器
   - 递归查询在大数据集上开销较高
   - 使用selectAllMutable进行就地修改

3. **内存使用优化**:
   - RAII设计确保自动内存管理
   - 大数据处理时考虑流式处理
   - Release模式下性能显著提升

#### 🆚 与其他库对比

JsonStruct的主要优势:
- **代码简洁**: `JSON_FIELDS()`一行完成注册
- **类型安全**: 编译时类型检查
- **功能丰富**: 内置JSONPath查询和版本管理
- **STL集成**: 原生支持标准库类型

适用场景:
- 新项目开发，追求现代C++特性
- 需要JSONPath查询功能的应用
- STL/Qt重度使用的项目
- 希望减少序列化样板代码的团队
