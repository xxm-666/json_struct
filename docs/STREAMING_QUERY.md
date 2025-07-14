# 流式查询和延迟求值功能

## 概述

JsonValue类现在支持流式查询和延迟求值功能，通过生成器模式实现内存高效的大数据处理。这个功能特别适用于：

- 大型JSON文件处理
- 需要早停的查询场景
- 内存受限的环境
- 实时数据流处理

## 核心特性

### 1. QueryGenerator类

`QueryGenerator`是流式查询的核心类，提供了以下功能：

- **延迟求值**: 结果按需生成，不会一次性加载所有匹配项
- **早停支持**: 可以在满足条件时提前终止查询
- **批处理**: 支持分批处理大量结果
- **迭代器接口**: 提供标准的C++迭代器模式

### 2. 配置选项

```cpp
struct GeneratorOptions {
    size_t maxResults = 0;              // 最大结果数量 (0 = 无限制)
    bool stopOnFirstMatch = false;      // 找到第一个匹配后停止
    size_t batchSize = 100;             // 批处理大小
    bool enableEarlyTermination = true; // 允许早停
};
```

## API 接口

### 基础流式查询

```cpp
// 创建查询生成器
auto generator = jsonValue.streamQuery("$.data[*].name");

// 使用迭代器遍历结果
for (auto it = generator.begin(); it != generator.end(); ++it) {
    const JsonValue* value = it->first;
    const std::string& path = it->second;
    // 处理结果...
}
```

### 延迟查询处理

```cpp
// 使用自定义处理函数进行延迟查询
size_t processed = jsonValue.lazyQuery(
    "$.data[*]", 
    [](const JsonValue* value, const std::string& path) -> bool {
        // 处理逻辑
        // 返回false可以提前终止
        return shouldContinue;
    }
);
```

### 早停查询

```cpp
// 找到第一个匹配项后立即返回
auto firstMatch = jsonValue.findFirst("$.users[?(@.age > 18)]");
if (firstMatch) {
    const JsonValue* value = firstMatch->first;
    const std::string& path = firstMatch->second;
}

// 限制结果数量
auto generator = jsonValue.streamQueryLimited("$.products[*]", 100);
```

### 计数查询

```cpp
// 统计匹配数量，不材料化结果
size_t count = jsonValue.countMatches("$.items[?(@.active == true)]");
```

### 批处理

```cpp
QueryGenerator::GeneratorOptions options;
options.batchSize = 500;

auto generator = jsonValue.streamQuery("$.data[*]", options);

while (generator.hasMore()) {
    auto batch = generator.takeBatch();
    // 处理批次...
}
```

### 自定义yield处理

```cpp
auto generator = jsonValue.streamQuery("$.metrics[*].value");

generator.yield([](const JsonValue* value, const std::string& path, size_t index) -> bool {
    // 自定义处理逻辑
    double val = value->toDouble();
    processMetric(val, index);
    
    // 控制是否继续处理
    return index < 1000;  // 只处理前1000个
});
```

### 流式输出

```cpp
std::vector<std::pair<const JsonValue*, std::string>> results;

// 直接流式输出到容器
size_t count = jsonValue.streamTo(
    "$.data[*].important_field",
    std::back_inserter(results)
);
```

## 性能优势

### 内存效率

- **传统方式**: `selectAll()` 一次性加载所有结果到内存
- **流式方式**: 按需生成结果，内存占用固定

```cpp
// 传统方式 - 可能占用大量内存
auto allResults = json.selectAll("$.huge_array[*]");  // 可能几GB内存

// 流式方式 - 内存占用小且固定
auto generator = json.streamQuery("$.huge_array[*]");
for (auto it = generator.begin(); it != generator.end(); ++it) {
    // 只处理当前项，不占用额外内存
}
```

### 早停优化

```cpp
// 查找第一个满足条件的项
auto firstActive = json.findFirst("$.users[?(@.active == true)]");
// 比遍历所有用户然后取第一个要快得多
```

### 批处理优化

```cpp
// 分批处理大数据，避免内存峰值
while (generator.hasMore()) {
    auto batch = generator.takeBatch(1000);
    processBatch(batch);  // 每次只处理1000个项目
}
```

## 使用场景

### 1. 大文件处理

```cpp
// 处理包含百万级记录的JSON文件
auto generator = largeJson.streamQuery("$.records[*]");

generator.yield([&](const JsonValue* record, const std::string& path, size_t index) {
    // 处理单条记录
    processRecord(*record);
    
    // 每10000条打印进度
    if (index % 10000 == 0) {
        std::cout << "Processed " << index << " records" << std::endl;
    }
    
    return true;  // 继续处理
});
```

### 2. 实时数据过滤

```cpp
// 实时过滤数据流
auto generator = dataStream.streamQuery("$.events[?(@.priority == 'high')]");

for (auto it = generator.begin(); it != generator.end(); ++it) {
    const JsonValue* event = it->first;
    handleHighPriorityEvent(*event);
    
    // 检查是否需要停止
    if (shouldStop()) {
        it.terminate();
        break;
    }
}
```

### 3. 分页查询模拟

```cpp
// 模拟分页查询
size_t pageSize = 50;
size_t currentPage = 0;

auto generator = json.streamQuery("$.items[*]");

while (generator.hasMore()) {
    auto page = generator.takeBatch(pageSize);
    if (page.empty()) break;
    
    std::cout << "Page " << ++currentPage << ": " << page.size() << " items" << std::endl;
    
    // 处理当前页
    for (const auto& [value, path] : page) {
        processItem(*value);
    }
}
```

## 实现细节

### 生成器状态

```cpp
enum class State { 
    Ready,      // 准备就绪
    Running,    // 正在运行
    Completed,  // 已完成
    Terminated  // 已终止
};
```

### 线程安全

当前实现不是线程安全的。如果需要在多线程环境中使用，需要外部同步机制。

### 内存管理

- 生成器持有对原始JsonValue的引用，不复制数据
- 返回的指针在原始JsonValue生命周期内有效
- 内部缓存会延迟加载查询结果

## 最佳实践

1. **选择合适的批大小**: 根据内存限制和处理需求调整batchSize
2. **使用早停**: 当不需要所有结果时，设置maxResults或使用stopOnFirstMatch
3. **避免嵌套生成器**: 不要在一个生成器的处理过程中创建另一个生成器
4. **及时释放**: 确保在适当的时候调用terminate()来释放资源

## 兼容性

流式查询功能完全向后兼容，不影响现有的`selectAll()`、`selectFirst()`等方法。可以根据需要选择使用传统方法或流式方法。
