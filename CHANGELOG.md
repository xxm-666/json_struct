# JsonStruct 变更日志

本文档记录了JsonStruct项目的所有重要变更。

格式基于[Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
并且本项目遵循[语义化版本控制](https://semver.org/lang/zh-CN/)。

## [1.2.x-dev] - 开发中版本

### 完成功能

- ✅ **完整的版本管理系统** - 已完成实现和测试
  - 版本信息查询API (`JsonStruct::Version` 类)
  - 版本兼容性检查功能
  - 构建时版本信息注入
  - Git提交和分支信息集成
  - 版本信息命令行工具 (`version_info`)
  - JSON格式版本信息输出
  - 完整的测试套件和演示程序

- ✅ **极限性能与极端边界测试**
  - 百万级对象序列化/反序列化性能测试
  - 千万级基础类型数组序列化/解析性能测试
  - 复杂嵌套结构极限测试
  - 并发读写与异常安全性测试
  - 自动化测试覆盖极端场景

### 性能与极端测试

- 新增 `test_performance_limits.cpp`，覆盖如下：
  - `Performance_SerializeMillionObjects`：100万对象序列化5秒内
  - `Performance_ParseMillionObjects`：100万对象解析5秒内
  - `Performance_SerializeLargeArray`：1000万数组序列化3秒内
  - `Performance_ParseLargeArray`：1000万数组解析3秒内
  - `Performance_ComplexNestedStructure`：百万级复杂嵌套结构序列化/解析5秒内
  - 所有测试均在Release模式下通过
  - 并发与异常测试见 `test_multithreaded_concurrency.cpp`

### 版本管理功能详细说明

- **核心API**：
  - `Version::getVersionString()` - 获取版本字符串
  - `Version::getDetailedVersionString()` - 获取详细版本信息
  - `Version::getVersionTuple()` - 获取版本元组
  - `Version::isCompatible(major, minor)` - API兼容性检查
  - `Version::compareVersion()` - 版本比较
  - Git元数据访问API
- **构建集成**：
  - CMake自动版本生成
  - Git信息自动提取
  - 版本头文件自动生成
- **工具程序**：
  - `test_version` - 功能测试套件
  - `version_info` - 命令行工具
  - `version_demo` - 使用演示

### 改进

- CMake构建系统增强，支持自动版本生成
- 添加语义化版本控制支持
- 构建过程中显示版本和Git信息
- JsonPath新增多种路径检索功能

## [1.2.0-dev] - 当前开发版本

### 新增

- JSONPath可修改式查询功能
  - `selectFirstMutable()` - 单值修改支持
  - `selectAllMutable()` - 批量修改支持
  - `MutableQueryResult` 结构体
- JSONPath对带空格属性名的完整支持
  - 方括号语法：`['property name']`, `["property name"]`
  - 过滤器中的方括号属性访问：`@['spaced property']`
  - 递归搜索支持：`$..['spaced property']`
  - 混合语法支持：`$.data['user info'][*].name`
- 添加JsonPatch功能
  - `ApplyPatch()`方法
  - 支持添加、删除、替换、移动等操作
  - 支持数组操作
  - 支持对象操作

### 改进

- 过滤器表达式解析增强
  - 支持方括号属性访问语法
  - 向后兼容原有点号语法
  - 改进正则表达式模式匹配
- 全面的测试套件
  - 边界情况测试（特殊字符、Unicode、转义字符等）
  - 可修改操作测试
  - 过滤器功能测试
- 完善JSONPath查询引擎
- 改进流式查询处理

### 修复

- 修复过滤器中带空格属性名的解析问题
- 修复explicit构造函数导致的编译错误

## [1.1.0] - 2025-07-10

### 新增

- 基本JSONPath查询功能
- 只读查询支持：`selectFirst()`, `selectAll()`
- 基础过滤器表达式支持
- 递归搜索功能：`$..property`

### 改进

- JSON解析性能优化
- 错误处理机制完善

## [1.0.0] - 2025-07-07

### 新增

- 初始版本发布
- 基础JSON值操作
- 类型注册系统
- Qt类型支持
- STL容器支持

---

## 版本说明

### 语义化版本控制

本项目采用语义化版本控制 (SemVer)：

- **主版本号 (MAJOR)**：当API发生不兼容的变更时递增
- **次版本号 (MINOR)**：当添加向后兼容的新功能时递增
- **修订号 (PATCH)**：当进行向后兼容的缺陷修复时递增

### 版本后缀

- `-dev`：开发版本
- `-alpha`：内测版本
- `-beta`：公测版本
- `-rc`：发布候选版本
- 无后缀：正式发布版本

### 兼容性承诺

- **主版本内兼容**：同一主版本内的所有次版本都向后兼容
- **API稳定性**：1.x系列保证API稳定，不会有破坏性变更
- **弃用警告**：任何API弃用都会在至少一个次版本中提供警告

### 支持政策

- **当前版本**：获得新功能、性能改进和安全修复
- **前一主版本**：获得关键缺陷修复和安全修复（12个月）
- **更旧版本**：仅获得安全修复（按需提供）
