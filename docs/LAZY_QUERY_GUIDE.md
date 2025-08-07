# Enhanced Lazy Query Generator 使用指南

## 概述

懒查询生成器 (`LazyQueryGenerator`) 是JsonStruct库中的高级查询组件，提供完整的JSONPath语法支持、智能缓存系统和性能监控功能。

## 快速开始

### 基本使用

```cpp
#include "jsonstruct.h"
using namespace JsonStruct;

// 创建JSON数据
JsonValue data = JsonValue::parse(R"({
    "store": {
        "book": [
            {"title": "Book1", "price": 10.99, "category": "fiction"},
            {"title": "Book2", "price": 15.99, "category": "non-fiction"},
            {"title": "Book3", "price": 8.99, "category": "fiction"}
        ]
    }
})");

// 创建过滤器和查询生成器
JsonFilter filter = JsonFilter::createDefault();
auto gen = EnhancedQueryFactory::createGenerator(filter, data, 
    "$.store.book[?(@.price < 12)].title");

// 处理结果
while (gen.hasNext()) {
    auto result = gen.next();
    std::cout << "Found: " << result.value->toString() << std::endl;
}
```

### 启用缓存系统

```cpp
// 启用缓存
gen.enableCache(true);

// 首次查询（建立缓存）
processResults(gen);

// 重置生成器（保留缓存）
gen.reset();

// 第二次查询将利用缓存
processResults(gen);

// 查看缓存统计
std::cout << "缓存命中率: " << gen.getCacheHitRatio() << "%" << std::endl;
std::cout << "缓存大小: " << gen.getCacheSize() << " 条目" << std::endl;
```

## 支持的JSONPath语法

### 1. 基础路径访问
```cpp
"$.property"           // 属性访问
"$.nested.property"    // 嵌套属性
"$['spaced property']" // 带空格的属性名
```

### 2. 数组操作
```cpp
"$.array[0]"          // 索引访问
"$.array[-1]"         // 负索引（最后一个元素）
"$.array[*]"          // 通配符（所有元素）
```

### 3. 数组切片
```cpp
"$.array[1:4]"        // 切片 [1, 2, 3]
"$.array[1:]"         // 从索引1到末尾
"$.array[:3]"         // 从开始到索引3
"$.array[::2]"        // 步长为2
"$.array[1:5:2]"      // 开始:结束:步长
```

### 4. 递归下降
```cpp
"$..property"         // 递归查找所有property
"$..book[*].title"    // 递归查找所有书籍标题
```

### 5. 过滤器表达式
```cpp
"$.books[?(@.price > 10)]"                    // 数值比较
"$.books[?(@.category == 'fiction')]"         // 字符串比较
"$.books[?(@.price > 10 && @.available)]"     // 逻辑与
"$.books[?(@.price < 5 || @.category == 'sale')]"  // 逻辑或
```

### 6. Union操作
```cpp
"$.book.title,$.book.author,$.book.price"    // 多路径Union
"$.users[0].name,$.users[1].name"            // 特定索引Union
```

## 性能监控

### 获取性能统计

```cpp
// 执行查询后获取统计信息
std::cout << "处理帧数: " << gen.getFramesProcessed() << std::endl;
std::cout << "生成结果: " << gen.getResultsGenerated() << std::endl;
std::cout << "效率比: " << gen.getEfficiencyRatio() << "%" << std::endl;

// 缓存统计
if (gen.isCacheEnabled()) {
    std::cout << "缓存命中率: " << gen.getCacheHitRatio() << "%" << std::endl;
    std::cout << "缓存大小: " << gen.getCacheSize() << " 条目" << std::endl;
}
```

### 性能对比示例

```cpp
#include <chrono>

auto start = std::chrono::steady_clock::now();

// 执行查询
processAllResults(gen);

auto end = std::chrono::steady_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

std::cout << "查询耗时: " << duration.count() << "μs" << std::endl;
std::cout << "平均每结果: " << duration.count() / gen.getResultsGenerated() << "μs" << std::endl;
```

## 缓存系统详解

### 缓存工作原理

增强型查询生成器使用基于路径的智能缓存系统：

1. **路径级缓存**: 每个查询路径都有独立的缓存条目
2. **自动管理**: 超过最大缓存大小时自动清理旧条目
3. **持久化**: `reset()` 操作保留缓存数据
4. **统计追踪**: 详细的命中率和使用统计

### 缓存配置

```cpp
// 启用/禁用缓存
gen.enableCache(true);

// 清理缓存
gen.clearCache();

// 检查缓存状态
if (gen.isCacheEnabled()) {
    std::cout << "缓存已启用" << std::endl;
}
```

### 最佳实践

1. **启用缓存的场景**:
   - 重复执行相同或相似查询
   - 复杂的过滤器表达式
   - 大型数据集查询

2. **不建议启用缓存的场景**:
   - 一次性简单查询
   - 内存严格限制的环境
   - 数据频繁变化的场景

## 错误处理

### 异常安全

```cpp
try {
    auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
    
    while (gen.hasNext()) {
        auto result = gen.next();
        // 处理结果
    }
} catch (const std::exception& e) {
    std::cerr << "查询错误: " << e.what() << std::endl;
}
```

### 边界条件处理

增强型查询生成器优雅处理各种边界条件：

- 空数组和对象
- 无效的索引访问
- 恶意构造的查询
- 深度嵌套结构

## 实际应用示例

### 场景1: 电商数据分析

```cpp
// 查找价格在特定范围内的热销商品
auto gen = EnhancedQueryFactory::createGenerator(filter, ecommerceData,
    "$.categories[*].products[?(@.price >= 20 && @.price <= 100 && @.sales > 1000)]");

gen.enableCache(true);  // 启用缓存，因为可能需要多次查询

std::vector<JsonValue> results;
while (gen.hasNext() && results.size() < 50) {  // 限制结果数量
    results.push_back(*gen.next().value);
}

std::cout << "找到 " << results.size() << " 个符合条件的商品" << std::endl;
std::cout << "查询效率: " << gen.getEfficiencyRatio() << "%" << std::endl;
```

### 场景2: 日志数据筛选

```cpp
// 查找特定时间段的错误日志
auto gen = EnhancedQueryFactory::createGenerator(filter, logData,
    "$..logs[?(@.level == 'ERROR' && @.timestamp > '2025-01-01')]");

// 分批处理大量日志
while (gen.hasNext()) {
    std::vector<JsonValue> batch;
    
    // 每次处理100条
    for (int i = 0; i < 100 && gen.hasNext(); ++i) {
        batch.push_back(*gen.next().value);
    }
    
    processLogBatch(batch);
}
```

### 场景3: 配置文件查询

```cpp
// 查找所有服务的配置信息
auto gen = EnhancedQueryFactory::createGenerator(filter, configData,
    "$.services[*].name,$.services[*].port,$.services[*].enabled");

gen.enableCache(true);  // 配置查询适合缓存

// 构建配置映射
std::map<std::string, ServiceConfig> services;
while (gen.hasNext()) {
    auto result = gen.next();
    // 解析和存储配置
}
```

## 性能优化建议

### 1. 查询优化

```cpp
// 好的做法：具体的路径
"$.users[0].profile.settings.theme"

// 避免：过于宽泛的递归查询
"$..theme"  // 可能搜索整个数据结构
```

### 2. 缓存策略

```cpp
// 适合缓存：重复查询
for (int i = 0; i < iterations; ++i) {
    gen.reset();
    processResults(gen);  // 利用缓存提升性能
}

// 不适合缓存：一次性查询
auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
// gen.enableCache(false);  // 可以显式禁用
processResults(gen);
```

### 3. 内存管理

```cpp
// 处理大量结果时避免全部存储在内存中
while (gen.hasNext()) {
    auto result = gen.next();
    processImmediately(*result.value);  // 立即处理，不存储
}
```

## 故障排除

### 常见问题

1. **查询返回空结果**
   - 检查JSONPath语法是否正确
   - 验证数据结构是否匹配查询路径
   - 使用简单查询逐步调试

2. **性能问题**
   - 检查是否启用了缓存
   - 避免过于复杂的递归查询
   - 考虑分批处理大量数据

3. **内存使用过高**
   - 避免一次性加载所有结果
   - 检查缓存大小是否合理
   - 使用流式处理

### 调试技巧

```cpp
// 启用详细日志
auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
gen.enableCache(true);

// 分步执行和统计
int stepCount = 0;
while (gen.hasNext() && stepCount < 10) {
    auto result = gen.next();
    std::cout << "Step " << stepCount << ": " 
              << result.path << " = " << result.value->toString() << std::endl;
    stepCount++;
}

// 输出性能统计
std::cout << "统计信息:" << std::endl;
std::cout << "- 帧数: " << gen.getFramesProcessed() << std::endl;
std::cout << "- 结果: " << gen.getResultsGenerated() << std::endl;
std::cout << "- 效率: " << gen.getEfficiencyRatio() << "%" << std::endl;
```

## 总结

增强型懒查询生成器提供了强大而灵活的JSON查询能力，特别适合：

- 复杂的JSONPath查询需求
- 大数据集的高效处理
- 需要性能监控的应用场景
- 重复查询优化需求

通过合理使用缓存系统和性能监控功能，可以显著提升JSON数据处理的效率和性能。
