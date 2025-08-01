# JsonStruct - 高级功能与流式查询

> 本文档合并了原 ADVANCED_FEATURES.md 和 STREAMING_QUERY.md 的内容，提供完整的高级功能指南。

## JSONPath 查询系统 (100% 完整实现)

JsonStruct 提供完整的JSONPath查询语言支持，实现了100%的核心功能。

### 核心查询方法

```cpp
#include "json_value.h"
using namespace JsonStruct;

JsonValue data = JsonValue::parse(R"({
    "store": {
        "book": [
            {"title": "C++ Programming", "price": 29.99, "category": "tech"},
            {"title": "JSON Guide", "price": 19.99, "category": "tech"},
            {"title": "Fiction Novel", "price": 15.99, "category": "fiction"}
        ],
        "bicycle": {
            "color": "red",
            "price": 299.99
        }
    },
    "numbers": [10, 20, 30, 40, 50]
})");

// 1. 路径存在性检查
bool exists = data.pathExists("$.store.book[0].title");     // true
bool missing = data.pathExists("$.store.car");              // false

// 2. 单值选择
const JsonValue* title = data.selectFirst("$.store.book[0].title");  // "C++ Programming"
const JsonValue* price = data.selectFirst("$.store.bicycle.price");  // 299.99

// 3. 多值选择 (返回指针)
auto titles = data.selectAll("$.store.book[*].title");      // 所有书籍标题
auto allPrices = data.selectAll("$..price");                // 递归查找所有价格

// 4. 多值选择 (返回副本)
auto priceValues = data.selectValues("$..price");           // 价格数据的副本
```

### 支持的JSONPath语法功能

#### 1. 基础属性访问
```cpp
// 简单属性
auto color = data.selectFirst("$.store.bicycle.color");     // "red"

// 嵌套属性
auto category = data.selectFirst("$.store.book[0].category"); // "tech"
```

#### 2. 数组索引访问
```cpp
// 正索引
auto firstBook = data.selectFirst("$.store.book[0]");       // 第一本书对象
auto secondTitle = data.selectFirst("$.store.book[1].title"); // "JSON Guide"

// 数组元素
auto number = data.selectFirst("$.numbers[2]");             // 30
```

#### 3. 数组切片
```cpp
// 基础切片 [start:end]
auto slice = data.selectAll("$.numbers[1:4]");              // [20, 30, 40]
auto books = data.selectAll("$.store.book[0:2]");           // 前两本书
```

#### 4. 通配符选择
```cpp
// 对象属性通配符
auto storeItems = data.selectAll("$.store.*");              // [book数组, bicycle对象]

// 数组元素通配符
auto allBooks = data.selectAll("$.store.book[*]");          // 所有书籍对象
auto allTitles = data.selectAll("$.store.book[*].title");   // 所有书籍标题
auto allNumbers = data.selectAll("$.numbers[*]");           // 所有数字

// 根级通配符
auto rootItems = data.selectAll("$.*");                     // [store对象, numbers数组]
```

#### 5. 递归下降搜索
```cpp
// 递归查找所有匹配的属性
auto allPrices = data.selectAll("$..price");                // 找到所有price属性
auto allTitles = data.selectAll("$..title");                // 找到所有title属性

// 递归搜索支持任意深度嵌套
JsonValue deepData = JsonValue::parse(R"({
    "level1": {
        "level2": {
            "level3": {
                "target": "found"
            }
        }
    }
})");

auto target = deepData.selectAll("$..target");              // ["found"]
```

### 完整功能覆盖率

| 功能类别 | 语法示例 | 实现状态 | 说明 |
|----------|----------|----------|------|
| **基础访问** | `$.prop` | ✅ 100% | 属性访问 |
| **嵌套访问** | `$.a.b.c` | ✅ 100% | 多层嵌套 |
| **数组索引** | `$.arr[0]` | ✅ 100% | 正索引访问 |
| **数组切片** | `$.arr[1:3]` | ✅ 100% | 范围切片 |
| **数组通配符** | `$.arr[*]` | ✅ 100% | 所有元素 |
| **对象通配符** | `$.*` | ✅ 100% | 所有属性 |
| **复合通配符** | `$.arr[*].prop` | ✅ 100% | 数组元素属性 |
| **递归下降** | `$..prop` | ✅ 100% | 深度搜索 |
| **路径验证** | `pathExists()` | ✅ 100% | 存在性检查 |
| **单值查询** | `selectFirst()` | ✅ 100% | 第一个匹配 |
| **多值查询** | `selectAll()` | ✅ 100% | 所有匹配 |
| **值复制** | `selectValues()` | ✅ 100% | 数据副本 |

### 实际应用示例

```cpp
// 电商数据查询示例
JsonValue ecommerce = JsonValue::parse(R"({
    "categories": [
        {
            "name": "Electronics",
            "products": [
                {"name": "Laptop", "price": 999.99, "stock": 10},
                {"name": "Phone", "price": 599.99, "stock": 25}
            ]
        },
        {
            "name": "Books", 
            "products": [
                {"name": "C++ Book", "price": 49.99, "stock": 5},
                {"name": "JSON Guide", "price": 29.99, "stock": 8}
            ]
        }
    ]
})");

// 查找所有产品名称
auto productNames = ecommerce.selectAll("$..products[*].name");
// 结果: ["Laptop", "Phone", "C++ Book", "JSON Guide"]

// 查找所有价格超过50的产品
auto expensiveProducts = ecommerce.selectAll("$..products[*]");
for (const auto* product : expensiveProducts) {
    if (product->isObject() && (*product)["price"].toDouble() > 50.0) {
        std::cout << "Expensive: " << (*product)["name"].toString() << std::endl;
    }
}

// 查找第一个类别的所有产品价格
auto firstCategoryPrices = ecommerce.selectAll("$.categories[0].products[*].price");

// 检查特定产品是否存在
bool laptopExists = ecommerce.pathExists("$.categories[0].products[0].name");
```

### 错误处理和边界情况

```cpp
// 无效路径处理
JsonValue data = JsonValue::object({{"test", JsonValue("value")}});

// 路径不存在
assert(!data.pathExists("$.nonexistent"));                  // false
assert(data.selectFirst("$.nonexistent") == nullptr);       // null

// 数组越界
JsonValue arr = JsonValue::object({{"numbers", JsonValue::array({JsonValue(1), JsonValue(2)})}});
assert(arr.selectFirst("$.numbers[10]") == nullptr);        // null
assert(arr.selectAll("$.numbers[10:20]").empty());         // 空结果

// 类型不匹配
JsonValue str = JsonValue::object({{"text", JsonValue("hello")}});
assert(str.selectFirst("$.text[0]") == nullptr);            // 字符串不支持索引
assert(!str.pathExists("$.text.length"));                   // 字符串没有length属性
```

## 流式解析器

### 事件驱动解析

流式解析器适用于处理大型JSON文件，内存使用量仅与嵌套深度相关：

```cpp
#include "json_stream_parser.h"

JsonStreamParser parser;

// 处理文件流
std::ifstream file("large_data.json");
std::string line;

while (std::getline(file, line)) {
    parser.feedData(line);
    
    JsonStreamParser::Event event;
    while ((event = parser.nextEvent()) != JsonStreamParser::Event::End) {
        switch (event) {
            case JsonStreamParser::Event::ObjectStart:
                std::cout << "Object started" << std::endl;
                break;
                
            case JsonStreamParser::Event::ObjectEnd:
                std::cout << "Object ended" << std::endl;
                break;
                
            case JsonStreamParser::Event::ArrayStart:
                std::cout << "Array started" << std::endl;
                break;
                
            case JsonStreamParser::Event::ArrayEnd:
                std::cout << "Array ended" << std::endl;
                break;
                
            case JsonStreamParser::Event::Key:
                std::cout << "Key: " << parser.getCurrentKey() << std::endl;
                break;
                
            case JsonStreamParser::Event::Value:
                JsonValue value = parser.getCurrentValue();
                std::cout << "Value: " << value.serialize() << std::endl;
                break;
                
            case JsonStreamParser::Event::Error:
                std::cout << "Parse error: " << parser.getErrorMessage() << std::endl;
                return;
        }
    }
}
```

### 实时数据处理

```cpp
class JsonDataProcessor {
public:
    void processStream(const std::string& chunk) {
        parser_.feedData(chunk);
        
        JsonStreamParser::Event event;
        while ((event = parser_.nextEvent()) != JsonStreamParser::Event::End) {
            handleEvent(event);
        }
    }
    
private:
    JsonStreamParser parser_;
    std::vector<JsonValue> objects_;
    
    void handleEvent(JsonStreamParser::Event event) {
        switch (event) {
            case JsonStreamParser::Event::ObjectEnd:
                // 完整对象解析完成
                JsonValue obj = parser_.getCurrentObject();
                processCompleteObject(obj);
                break;
            // 其他事件处理...
        }
    }
    
    void processCompleteObject(const JsonValue& obj) {
        // 处理完整的JSON对象
        if (obj.contains("type") && obj["type"].toString() == "user") {
            // 处理用户数据
        }
    }
};
```

## 高精度数值系统

### JsonNumber类详解

JsonNumber类提供超越IEEE 754双精度浮点数限制的数值处理：

```cpp
// 创建高精度数值
JsonNumber bigInt("9007199254740993");    // 2^53 + 1
JsonNumber decimal("123.456789012345678901234567890");

// 类型检查
std::cout << "Is integer: " << bigInt.isInteger() << std::endl;        // true
std::cout << "Is floating: " << decimal.isFloatingPoint() << std::endl; // true

// 精度保持
long long value = bigInt.toInteger();    // 9007199254740993 (精确值)
std::string str = decimal.toString();    // 保持完整精度
```

### 特殊数值处理

```cpp
// 创建特殊数值
JsonNumber nan = JsonNumber::createNaN();
JsonNumber inf = JsonNumber::createInfinity();
JsonNumber negInf = JsonNumber::createNegativeInfinity();

// 检测特殊数值
if (nan.isNaN()) {
    std::cout << "Not a number" << std::endl;
}

if (inf.isInfinity()) {
    std::cout << "Positive infinity" << std::endl;
}

// 序列化特殊数值
JsonValue::SerializeOptions options;
options.allowSpecialNumbers = true;
JsonValue specialValue(nan);
std::string result = specialValue.serialize(options); // "NaN"
```

## 类型注册系统详解

### 复杂类型注册

```cpp
// 复杂结构体
struct UserInfo {
    std::string name;
    int age;
    std::vector<std::string> hobbies;
    std::map<std::string, std::string> metadata;
    
    bool operator==(const UserInfo& other) const {
        return name == other.name && age == other.age && 
               hobbies == other.hobbies && metadata == other.metadata;
    }
};

// 注册复杂类型
REGISTER_JSON_TYPE(UserInfo,
    // 序列化
    [](const UserInfo& user) -> JsonValue {
        JsonValue obj;
        obj["name"] = JsonValue(user.name);
        obj["age"] = JsonValue(user.age);
        
        // 序列化hobbies数组
        JsonValue::ArrayType hobbiesArray;
        for (const auto& hobby : user.hobbies) {
            hobbiesArray.push_back(JsonValue(hobby));
        }
        obj["hobbies"] = JsonValue(hobbiesArray);
        
        // 序列化metadata对象
        JsonValue::ObjectType metadataObj;
        for (const auto& pair : user.metadata) {
            metadataObj[pair.first] = JsonValue(pair.second);
        }
        obj["metadata"] = JsonValue(metadataObj);
        
        return obj;
    },
    // 反序列化
    [](const JsonValue& json, const UserInfo& defaultValue) -> UserInfo {
        if (!json.isObject()) return defaultValue;
        
        UserInfo user;
        
        if (json.contains("name") && json["name"].isString()) {
            user.name = json["name"].toString();
        }
        
        if (json.contains("age") && json["age"].isNumber()) {
            user.age = json["age"].toInt();
        }
        
        if (json.contains("hobbies") && json["hobbies"].isArray()) {
            const auto& hobbiesArray = json["hobbies"].toArray();
            for (const auto& hobby : hobbiesArray) {
                if (hobby.isString()) {
                    user.hobbies.push_back(hobby.toString());
                }
            }
        }
        
        if (json.contains("metadata") && json["metadata"].isObject()) {
            const auto& metadataObj = json["metadata"].toObject();
            for (const auto& pair : metadataObj) {
                if (pair.second.isString()) {
                    user.metadata[pair.first] = pair.second.toString();
                }
            }
        }
        
        return user;
    }
);
```

### 泛型容器自动注册

```cpp
// 自动注册常用STL容器
template<typename T>
void registerStdVector() {
    TypeRegistry::instance().registerType<std::vector<T>>(
        [](const std::vector<T>& vec) -> JsonValue {
            JsonValue::ArrayType arr;
            for (const auto& item : vec) {
                arr.push_back(TypeRegistry::instance().toJson(item));
            }
            return JsonValue(arr);
        },
        [](const JsonValue& json, const std::vector<T>& defaultValue) -> std::vector<T> {
            if (!json.isArray()) return defaultValue;
            
            std::vector<T> result;
            const auto& arr = json.toArray();
            for (const auto& item : arr) {
                result.push_back(TypeRegistry::instance().fromJson<T>(item, T{}));
            }
            return result;
        }
    );
}

// 使用时自动注册
registerStdVector<UserInfo>();
std::vector<UserInfo> users = {/* ... */};
JsonValue usersJson = TypeRegistry::instance().toJson(users);
```

## 错误恢复和容错解析

### 配置容错解析

```cpp
JsonValue::ParseOptions tolerantOptions;
tolerantOptions.allowRecovery = true;       // 启用错误恢复
tolerantOptions.allowComments = true;       // 允许注释
tolerantOptions.allowTrailingCommas = true; // 允许尾随逗号
tolerantOptions.strictMode = false;         // 非严格模式

// 解析可能有错误的JSON
std::string problematicJson = R"({
    "name": "Alice",
    "age": 30,
    /* 这是注释 */
    "hobbies": ["reading", "coding",], // 尾随逗号
    "invalid": undefined,               // 无效值
})";

try {
    JsonValue result = JsonValue::parse(problematicJson, tolerantOptions);
    // 成功解析，跳过无效部分
} catch (const JsonParseException& e) {
    std::cout << "Even tolerant parsing failed: " << e.what() << std::endl;
}
```

### 逐步恢复策略

```cpp
class FaultTolerantParser {
public:
    JsonValue parseWithRecovery(const std::string& jsonText) {
        JsonValue::ParseOptions options;
        options.allowRecovery = true;
        
        try {
            return JsonValue::parse(jsonText, options);
        } catch (const JsonParseException& e) {
            // 记录错误并尝试部分解析
            logError(e);
            return attemptPartialParse(jsonText, e.position());
        }
    }
    
private:
    JsonValue attemptPartialParse(const std::string& text, size_t errorPos) {
        // 尝试解析错误位置之前的部分
        std::string partialText = text.substr(0, errorPos);
        
        // 添加闭合括号使其成为有效JSON
        if (needsClosing(partialText)) {
            partialText += "}";
        }
        
        try {
            return JsonValue::parse(partialText);
        } catch (...) {
            return JsonValue(); // 返回null值
        }
    }
    
    void logError(const JsonParseException& e) {
        std::cerr << "Parse error at " << e.locationInfo() << ": " << e.what() << std::endl;
    }
    
    bool needsClosing(const std::string& text) {
        // 简单检查是否需要闭合括号
        int braceCount = 0;
        for (char c : text) {
            if (c == '{') braceCount++;
            else if (c == '}') braceCount--;
        }
        return braceCount > 0;
    }
};
```

## 性能优化和最佳实践

### 内存管理优化

```cpp
// 预分配内存
JsonValue createLargeObject(const std::vector<std::pair<std::string, std::string>>& data) {
    JsonValue obj;
    
    // 对于已知大小的数据，预留空间
    // (内部实现可能支持reserve操作)
    
    for (const auto& pair : data) {
        obj[pair.first] = JsonValue(pair.second);
    }
    
    return obj; // 返回时使用移动语义
}

// 避免不必要的拷贝
JsonValue processData(JsonValue&& input) { // 接受右值引用
    // 直接操作输入数据，避免拷贝
    input["processed"] = true;
    input["timestamp"] = getCurrentTimestamp();
    return std::move(input);
}
```

### 并发安全考虑

```cpp
#include <mutex>
#include <shared_mutex>

class ThreadSafeJsonCache {
public:
    void set(const std::string& key, const JsonValue& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        cache_[key] = value;
    }
    
    std::optional<JsonValue> get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            return it->second;
        }
        return std::nullopt;
    }
    
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, JsonValue> cache_;
};
```

## JSON过滤器模块

### 1. 基本过滤器使用

```cpp
#include "json_engine/json_filter.h"
using namespace JsonStruct;

JsonValue data = JsonValue::parse(R"({"items": [{"id": 1}, {"id": 2}, {"id": 3}]})");
JsonFilter filter;
filter.addCondition("$.items[*].id", [](const JsonValue& value) {
    return value.toInt() > 1;
});

JsonValue filtered = filter.apply(data);
std::cout << filtered.serialize(2) << std::endl;
```

### 2. 高级过滤器功能

```cpp
filter.addCondition("$.items[*].id", [](const JsonValue& value) {
    return value.toInt() % 2 == 0; // 筛选偶数ID
});

JsonValue filteredEven = filter.apply(data);
std::cout << filteredEven.serialize(2) << std::endl;
```

## JSON查询生成器模块

### 1. 动态查询生成

```cpp
#include "json_engine/json_query_generator.h"
using namespace JsonStruct;

JsonQueryGenerator generator;
generator.addPath("$.store.book[*].title");
generator.addPath("$.store.bicycle.price");

std::vector<std::string> queries = generator.generate();
for (const auto& query : queries) {
    std::cout << query << std::endl;
}
```

## 流式查询和延迟求值

JsonValue 类支持流式查询和延迟求值功能，通过生成器模式实现内存高效的大数据处理。这个功能特别适用于：

- 大型JSON文件处理
- 需要早停的查询场景
- 内存受限的环境
- 实时数据流处理

### 核心特性

#### QueryGenerator类

`QueryGenerator` 是流式查询的核心类，提供了以下功能：

- **延迟求值**: 结果按需生成，不会一次性加载所有匹配项
- **早停支持**: 可以在满足条件时提前终止查询
- **批处理**: 支持分批处理大量结果
- **迭代器接口**: 提供标准的C++迭代器模式

#### 配置选项

```cpp
struct GeneratorOptions {
    size_t maxResults = 0;              // 最大结果数量 (0 = 无限制)
    bool stopOnFirstMatch = false;      // 找到第一个匹配后停止
    size_t batchSize = 100;             // 批处理大小
    bool enableEarlyTermination = true; // 允许早停
};
```

### 流式查询 API

#### 基础流式查询

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

#### 延迟查询处理

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

#### 早停查询

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

#### 计数查询

```cpp
// 统计匹配数量，不材料化结果
size_t count = jsonValue.countMatches("$.items[?(@.active == true)]");
```

#### 批处理

```cpp
QueryGenerator::GeneratorOptions options;
options.batchSize = 500;

auto generator = jsonValue.streamQuery("$.data[*]", options);

while (generator.hasMore()) {
    auto batch = generator.takeBatch();
    // 处理批次...
}
```

#### 自定义yield处理

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

#### 流式输出

```cpp
std::vector<std::pair<const JsonValue*, std::string>> results;

// 直接流式输出到容器
size_t count = jsonValue.streamTo(
    "$.data[*].important_field",
    std::back_inserter(results)
);
```

### 性能优势

#### 内存效率

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

#### 早停优化

```cpp
// 查找第一个满足条件的项
auto firstActive = json.findFirst("$.users[?(@.active == true)]");
// 比遍历所有用户然后取第一个要快得多
```

#### 批处理优化

```cpp
// 分批处理大数据，避免内存峰值
while (generator.hasMore()) {
    auto batch = generator.takeBatch(1000);
    processBatch(batch);  // 每次只处理1000个项目
}
```

### 使用场景

#### 大文件处理

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

#### 实时数据过滤

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

#### 分页查询模拟

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

### 实现细节

#### 生成器状态

```cpp
enum class State { 
    Ready,      // 准备就绪
    Running,    // 正在运行
    Completed,  // 已完成
    Terminated  // 已终止
};
```

#### 线程安全

当前实现不是线程安全的。如果需要在多线程环境中使用，需要外部同步机制。

#### 内存管理

- 生成器持有对原始JsonValue的引用，不复制数据
- 返回的指针在原始JsonValue生命周期内有效
- 内部缓存会延迟加载查询结果

### 最佳实践

1. **选择合适的批大小**: 根据内存限制和处理需求调整batchSize
2. **使用早停**: 当不需要所有结果时，设置maxResults或使用stopOnFirstMatch
3. **避免嵌套生成器**: 不要在一个生成器的处理过程中创建另一个生成器
4. **及时释放**: 确保在适当的时候调用terminate()来释放资源

### 兼容性

流式查询功能完全向后兼容，不影响现有的`selectAll()`、`selectFirst()`等方法。可以根据需要选择使用传统方法或流式方法。
