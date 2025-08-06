# Enhanced Test Framework Documentation

## 概述

本文档描述了对 `test_framework.h` 的全面增强，这些改进大幅提升了测试框架的功能性、易用性和可维护性。

## 主要改进

### 1. 增强的断言系统

#### 字符串断言
- `ASSERT_STREQ(actual, expected)` - 字符串相等断言
- `ASSERT_STRNE(actual, expected)` - 字符串不等断言
- `ASSERT_CONTAINS(haystack, needle)` - 字符串包含断言
- `ASSERT_NOT_CONTAINS(haystack, needle)` - 字符串不包含断言
- `ASSERT_STARTS_WITH(str, prefix)` - 字符串开头断言
- `ASSERT_ENDS_WITH(str, suffix)` - 字符串结尾断言
- `ASSERT_MATCHES_REGEX(str, pattern)` - 正则表达式匹配断言

#### 容器断言
- `ASSERT_EMPTY(container)` - 容器为空断言
- `ASSERT_NOT_EMPTY(container)` - 容器非空断言
- `ASSERT_SIZE_EQ(container, expected_size)` - 容器大小断言

#### 异常断言
- `ASSERT_THROWS(statement, exception_type)` - 期望抛出特定异常
- `ASSERT_NO_THROW(statement)` - 期望不抛出异常
- `ASSERT_ANY_THROW(statement)` - 期望抛出任何异常

#### 指针断言
- `ASSERT_NULL(ptr)` - 指针为空断言
- `ASSERT_NOT_NULL(ptr)` - 指针非空断言

#### 数值比较断言
- `ASSERT_LT(left, right)` - 小于断言
- `ASSERT_LE(left, right)` - 小于等于断言
- `ASSERT_GT(left, right)` - 大于断言
- `ASSERT_GE(left, right)` - 大于等于断言

### 2. 测试管理功能

#### 测试标签和分类
```cpp
TEST_WITH_TAGS(TestName, "tag1", "tag2", "tag3") {
    // 测试代码
}
```

#### 测试跳过功能
```cpp
// 静态跳过测试
TEST_SKIP(TestName, "跳过原因")

// 动态跳过测试
TEST(ConditionalTest) {
    if (condition) {
        SKIP_TEST("基于条件跳过");
    }
    // 测试代码
}
```

#### 测试过滤和配置
```cpp
TestFramework::TestConfig config;
config.includePatterns = {"Core", "Json"};  // 只运行包含这些模式的测试
config.excludePatterns = {"Performance"};   // 排除包含这些模式的测试
config.includeTags = {"unit"};              // 只运行带有这些标签的测试
config.excludeTags = {"slow"};              // 排除带有这些标签的测试
config.verbose = true;                      // 详细输出
config.timing = true;                       // 显示测试时间
config.stopOnFirstFailure = false;         // 第一个失败后是否停止

SET_TEST_CONFIG(config);
```

### 3. 性能和计时功能

- **自动计时**: 每个测试的执行时间自动测量和显示
- **套件总时间**: 显示整个测试套件的执行时间
- **可配置显示**: 可以通过配置启用/禁用时间显示

### 4. 改进的错误报告

#### 详细的失败信息
- 包含文件名和行号
- 显示期望值和实际值
- 提供更详细的上下文信息

#### 异常处理
- 自动捕获测试中的未处理异常
- 提供异常类型和消息
- 确保测试框架的稳定性

### 5. 测试上下文和状态管理

#### TestContext 类
```cpp
class MyTestContext : public TestFramework::TestContext {
public:
    void setUp() override {
        // 测试前设置
    }
    
    void tearDown() override {
        // 测试后清理
    }
};

// 设置全局上下文
TestSuite::instance().setContext(std::make_unique<MyTestContext>());
```

### 6. 高级输出和报告

#### 详细模式
- 显示每个测试的执行状态
- 显示配置信息
- 显示过滤的测试

#### 统计信息
- 运行的测试数量
- 通过/失败/跳过的测试计数
- 总执行时间

## 使用示例

### 基本测试
```cpp
#include "test_framework.h"

TEST(BasicExample) {
    ASSERT_TRUE(true);
    ASSERT_EQ(42, 42);
    ASSERT_STREQ("hello", "hello");
}
```

### 带标签的测试
```cpp
TEST_WITH_TAGS(DatabaseTest, "integration", "database") {
    // 数据库集成测试
    ASSERT_NOT_NULL(database_connection);
}
```

### 异常测试
```cpp
TEST(ExceptionHandling) {
    ASSERT_THROWS(throw std::runtime_error("test"), std::runtime_error);
    ASSERT_NO_THROW(safe_operation());
}
```

### 容器测试
```cpp
TEST(ContainerOperations) {
    std::vector<int> vec = {1, 2, 3};
    ASSERT_SIZE_EQ(vec, 3);
    ASSERT_NOT_EMPTY(vec);
    ASSERT_CONTAINS(vector_to_string(vec), "2");
}
```

### 配置化运行
```cpp
int main() {
    TestFramework::TestConfig config;
    config.verbose = true;
    config.timing = true;
    config.includePatterns = {"Core"};
    
    SET_TEST_CONFIG(config);
    return RUN_ALL_TESTS();
}
```

## 向后兼容性

所有现有的测试代码都能够无修改地在增强的框架中运行：

- 所有原有的断言宏保持不变
- 原有的 `TEST()` 宏继续工作
- 原有的 `RUN_ALL_TESTS()` 宏继续工作
- 默认配置确保原有行为不变

## 迁移指南

### 从基础框架迁移

1. **无需修改现有测试**: 所有现有测试自动兼容
2. **逐步采用新功能**: 可以在新测试中使用新的断言和功能
3. **配置增强**: 在 main 函数中添加配置以启用新功能

### 建议的最佳实践

1. **使用适当的断言**: 选择最具描述性的断言类型
2. **添加测试标签**: 为测试分类和过滤添加有意义的标签
3. **启用详细模式**: 在开发时使用详细模式获得更多信息
4. **测试异常**: 使用异常断言确保错误处理的正确性
5. **性能监控**: 使用计时功能监控测试性能

## 错误排查

### 常见问题

1. **编译错误**: 确保 C++17 标准支持
2. **正则表达式错误**: 检查正则表达式语法
3. **标签过滤**: 确保标签名称正确匹配

### 调试技巧

1. 启用详细模式查看测试执行详情
2. 使用 `LIST_TESTS()` 查看所有可用测试
3. 检查测试配置是否正确应用

## 性能考虑

- 计时功能开销最小
- 标签和过滤在测试启动时处理
- 详细输出可能影响大型测试套件的性能

## 总结

增强的测试框架提供了：

1. **更丰富的断言系统** - 支持字符串、容器、异常等多种类型
2. **灵活的测试管理** - 标签、过滤、跳过功能
3. **详细的错误报告** - 更好的调试信息
4. **性能监控** - 自动计时功能
5. **完全的向后兼容性** - 无需修改现有代码

这些改进大大提升了测试的可维护性、可读性和开发效率。
