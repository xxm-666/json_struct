# JsonValue Enhanced - C++17+ 现代化JSON库

## 🎯 概述

`JsonValueEnhanced` 是一个完全基于C++17及以上标准的现代化JSON处理库，充分利用现代C++特性提供安全、高性能、易用的JSON操作接口。

## 🌟 核心特性

### ✨ 现代C++特性
- **类型安全**: 基于 `std::variant` 的类型系统，完全避免了传统union的安全问题
- **内存安全**: 自动资源管理，支持RAII，防止内存泄漏
- **移动语义**: 全面支持移动语义，大幅提升大对象操作性能
- **可选值**: 基于 `std::optional` 的安全访问接口，避免未定义行为
- **字符串视图**: 使用 `std::string_view` 减少不必要的字符串拷贝
- **完美转发**: 模板参数完美转发，保持值类别

### 🚀 性能优化
- **哈希映射**: 使用 `std::unordered_map` 实现O(1)对象键查找
- **预分配策略**: 智能的内存预分配减少重分配开销
- **零拷贝操作**: 尽可能避免不必要的数据拷贝
- **编译时优化**: 大量使用 `constexpr` 和模板特化

### 🔧 高级功能
- **JSON指针**: 完整支持RFC 6901 JSON指针规范
- **配置化解析**: 支持注释、尾随逗号、深度限制等选项
- **高级序列化**: 键排序、格式化、精度控制等选项
- **容器集成**: 原生支持STL容器的直接构造
- **访问者模式**: 类型安全的值访问和处理
- **字面量语法**: 支持用户定义字面量`_json`

### 🛡️ 安全特性
- **Unicode完整支持**: 包括代理对(surrogate pairs)处理
- **深度限制**: 防止栈溢出攻击
- **严格验证**: 可配置的JSON标准合规性检查
- **详细错误信息**: 精确的行列位置错误报告
- **类型验证**: 编译时和运行时的类型安全检查

## 📚 快速入门

### 基本使用

```cpp
#include "json_value_enhanced.h"
using namespace JsonStruct;

// 基本类型
JsonValueEnhanced nullVal;                    // null
JsonValueEnhanced boolVal(true);              // boolean  
JsonValueEnhanced intVal(42);                 // number
JsonValueEnhanced strVal("hello");            // string

// 容器类型
std::vector<int> vec = {1, 2, 3};
JsonValueEnhanced arrVal(vec);                // array

std::map<std::string, int> map = {{"a", 1}};
JsonValueEnhanced objVal(map);                // object
```

### 解析JSON

```cpp
// 基本解析
std::string json = R"({"name": "Alice", "age": 30})";
auto parsed = JsonValueEnhanced::parse(json);

// 配置化解析
JsonValueEnhanced::ParseOptions options;
options.allowComments = true;
options.allowTrailingCommas = true;
options.maxDepth = 100;

std::string relaxedJson = R"({
    "name": "Alice", // 姓名
    "age": 30,       // 年龄
})";
auto relaxedParsed = JsonValueEnhanced::parse(relaxedJson, options);
```

### 序列化JSON

```cpp
JsonValueEnhanced data;
data["users"] = JsonValueEnhanced(std::vector<std::string>{"Alice", "Bob"});
data["config"] = JsonValueEnhanced();
data["config"]["debug"] = JsonValueEnhanced(true);

// 紧凑格式
std::string compact = data.dump();

// 美化格式
JsonValueEnhanced::SerializeOptions prettyOptions;
prettyOptions.indent = 2;
prettyOptions.sortKeys = true;
std::string pretty = data.dump(prettyOptions);
```

## 🔍 类型安全访问

### Optional接口（推荐）

```cpp
JsonValueEnhanced value = JsonValueEnhanced::parse(R"({"number": 42})");

// 安全访问，返回std::optional
if (auto num = value["number"].getNumber()) {
    std::cout << "Number: " << *num << std::endl;
} else {
    std::cout << "Not a number or doesn't exist" << std::endl;
}

// 类型检查
if (value["number"].isNumber()) {
    double num = value["number"].toDouble();
}
```

### 传统接口（带默认值）

```cpp
// 如果类型不匹配或键不存在，返回默认值
int age = value["age"].toInt(0);
std::string name = value["name"].toString("unknown");
bool debug = value["debug"].toBool(false);
```

## 🎯 容器集成

### 标准容器构造

```cpp
// 数组类型
std::array<int, 3> arr = {1, 2, 3};
JsonValueEnhanced arrJson(arr);

std::vector<std::string> vec = {"a", "b", "c"};
JsonValueEnhanced vecJson(vec);

std::set<int> s = {3, 1, 2};  // 自动转为数组
JsonValueEnhanced setJson(s);

// 映射类型
std::map<std::string, int> m = {{"key1", 1}, {"key2", 2}};
JsonValueEnhanced mapJson(m);

std::unordered_map<std::string, double> um = {{"pi", 3.14}};
JsonValueEnhanced umapJson(um);
```

### 数组操作

```cpp
JsonValueEnhanced arr;

// 添加元素
arr.append(JsonValueEnhanced(1));
arr.append(2);  // 自动转换
arr.append("hello");

// 索引访问
arr[0] = JsonValueEnhanced(42);
auto first = arr[0];

// 遍历
for (size_t i = 0; i < arr.size(); ++i) {
    std::cout << arr[i].dump() << " ";
}
```

### 对象操作

```cpp
JsonValueEnhanced obj;

// 键值操作
obj["name"] = JsonValueEnhanced("Alice");
obj["age"] = JsonValueEnhanced(30);

// 检查键存在
if (obj.contains("name")) {
    auto name = obj["name"];
}

// 删除键
obj.erase("age");

// 遍历对象
if (auto* objPtr = obj.getObject()) {
    for (const auto& [key, value] : *objPtr) {
        std::cout << key << ": " << value.dump() << std::endl;
    }
}
```

## 🔗 JSON指针 (RFC 6901)

```cpp
std::string json = R"({
    "users": [
        {"name": "Alice", "age": 30},
        {"name": "Bob", "age": 25}
    ],
    "config": {
        "version": "1.0",
        "features": {
            "debug": true
        }
    }
})";

auto data = JsonValueEnhanced::parse(json);

// 访问嵌套值
auto& alice = data.at("/users/0/name");
auto& version = data.at("/config/version");
auto& debug = data.at("/config/features/debug");

// 修改值
data.at("/users/1/age") = JsonValueEnhanced(26);

// 处理转义
data.at("/config/app~1name") = JsonValueEnhanced("MyApp");  // "app/name"
```

## 🎨 访问者模式

```cpp
JsonValueEnhanced value = JsonValueEnhanced::parse(R"([42, "hello", true, null])");

for (size_t i = 0; i < value.size(); ++i) {
    value[i].visit([](const auto& val) {
        using T = std::decay_t<decltype(val)>;
        
        if constexpr (std::is_same_v<T, std::monostate>) {
            std::cout << "null";
        } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << (val ? "true" : "false");
        } else if constexpr (std::is_same_v<T, double>) {
            std::cout << val;
        } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << '"' << val << '"';
        } else if constexpr (std::is_same_v<T, JsonValueEnhanced::ArrayType>) {
            std::cout << "[array with " << val.size() << " elements]";
        } else if constexpr (std::is_same_v<T, JsonValueEnhanced::ObjectType>) {
            std::cout << "{object with " << val.size() << " keys}";
        }
    });
    std::cout << " ";
}
```

## ✨ 现代语法糖

### 字面量语法

```cpp
using namespace JsonStruct::literals;

// 直接解析JSON字面量
auto config = R"({
    "app": "MyApp",
    "version": "1.0",
    "users": ["Alice", "Bob"]
})"_json;

auto simpleArray = R"([1, 2, 3, 4, 5])"_json;
```

### 工厂函数

```cpp
// 类型自动推导
auto intVal = makeJson(42);
auto strVal = makeJson("hello");
auto boolVal = makeJson(true);
auto doubleVal = makeJson(3.14);

// 容器自动转换
auto vecVal = makeJson(std::vector<int>{1, 2, 3});
auto mapVal = makeJson(std::map<std::string, int>{{"a", 1}});
```

### 流操作符

```cpp
// 输出
JsonValueEnhanced data = makeJson(42);
std::cout << data << std::endl;  // 输出: 42

// 输入
std::istringstream iss(R"({"key": "value"})");
JsonValueEnhanced parsed;
iss >> parsed;
```

## ⚙️ 配置选项

### 解析配置

```cpp
JsonValueEnhanced::ParseOptions options;

// 深度限制
options.maxDepth = 512;

// 允许注释 (JSON5风格)
options.allowComments = true;

// 允许尾随逗号
options.allowTrailingCommas = true;

// 严格模式
options.strictMode = false;

// UTF-8验证
options.validateUtf8 = true;

auto parsed = JsonValueEnhanced::parse(json, options);
```

### 序列化配置

```cpp
JsonValueEnhanced::SerializeOptions options;

// 缩进美化
options.indent = 2;

// 键排序
options.sortKeys = true;

// Unicode转义
options.escapeUnicode = false;

// 紧凑数组
options.compactArrays = true;

// 数值精度
options.maxPrecision = 15;

std::string result = data.dump(options);
```

## 🛡️ 错误处理

### 解析错误

```cpp
try {
    std::string invalidJson = R"({
        "name": "test",
        "age": ,
        "city": "Beijing"
    })";
    
    auto parsed = JsonValueEnhanced::parse(invalidJson);
} catch (const std::runtime_error& e) {
    std::cout << "Parse error: " << e.what() << std::endl;
    // 输出: Parse error: Unexpected character ',' at line 3, column 16
}
```

### 访问错误

```cpp
JsonValueEnhanced data = makeJson(42);

try {
    // 尝试将数字当作对象访问
    auto invalid = data.at("/key");
} catch (const std::runtime_error& e) {
    std::cout << "Access error: " << e.what() << std::endl;
    // 输出: Access error: Cannot index into non-container type
}
```

## 🚀 性能考虑

### 移动语义

```cpp
// 优先使用移动语义
JsonValueEnhanced createLargeObject() {
    JsonValueEnhanced obj;
    for (int i = 0; i < 10000; ++i) {
        obj["key" + std::to_string(i)] = makeJson("value" + std::to_string(i));
    }
    return obj;  // 自动移动
}

auto large = createLargeObject();  // 移动构造
auto moved = std::move(large);     // 显式移动
```

### 预分配策略

```cpp
// 预分配数组大小
JsonValueEnhanced::ArrayType arr;
arr.reserve(1000);  // 预分配空间

for (int i = 0; i < 1000; ++i) {
    arr.emplace_back(makeJson(i));
}

JsonValueEnhanced data(std::move(arr));
```

### 避免不必要拷贝

```cpp
// 使用string_view避免拷贝
void processJson(std::string_view json) {
    auto parsed = JsonValueEnhanced::parse(json);
    // ...
}

// 使用引用访问
const auto& objRef = data.toObject();
for (const auto& [key, value] : objRef) {
    // 避免拷贝大对象
}
```

## 🔄 与标准库集成

### STL算法兼容

```cpp
JsonValueEnhanced arr = makeJson(std::vector<int>{3, 1, 4, 1, 5});

// 转换为标准容器进行算法操作
auto stdVec = arr.toArray();
std::sort(stdVec.begin(), stdVec.end(), [](const auto& a, const auto& b) {
    return a.toDouble() < b.toDouble();
});

// 转回JsonValue
JsonValueEnhanced sorted(std::move(stdVec));
```

### 类型特征

```cpp
// 检查类型
static_assert(std::is_nothrow_move_constructible_v<JsonValueEnhanced>);
static_assert(std::is_nothrow_move_assignable_v<JsonValueEnhanced>);

// 使用type_traits
template<typename T>
void processValue(const JsonValueEnhanced& value) {
    if constexpr (std::is_integral_v<T>) {
        auto num = value.getNumber();
        if (num) {
            T result = static_cast<T>(*num);
            // ...
        }
    }
}
```

## 📊 性能基准

基于10,000个键值对的测试结果：

| 操作 | 时间 | 备注 |
|------|------|------|
| 序列化 | ~20ms | 包含格式化 |
| 解析 | ~40ms | 包含验证 |
| 1000次查找 | ~1ms | O(1)哈希查找 |
| 移动构造 | ~4μs | 10,000元素数组 |

## 🔮 高级用法

### 自定义类型支持

```cpp
struct Person {
    std::string name;
    int age;
    std::vector<std::string> hobbies;
    
    JsonValueEnhanced toJson() const {
        JsonValueEnhanced obj;
        obj["name"] = makeJson(name);
        obj["age"] = makeJson(age);
        obj["hobbies"] = makeJson(hobbies);
        return obj;
    }
    
    static Person fromJson(const JsonValueEnhanced& json) {
        Person p;
        p.name = json["name"].toString();
        p.age = json["age"].toInt();
        
        if (auto* hobbies = json["hobbies"].getArray()) {
            for (const auto& hobby : *hobbies) {
                p.hobbies.push_back(hobby.toString());
            }
        }
        return p;
    }
};
```

### 条件解析

```cpp
JsonValueEnhanced parseConditional(std::string_view json, bool strict = true) {
    JsonValueEnhanced::ParseOptions options;
    options.strictMode = strict;
    options.allowComments = !strict;
    options.allowTrailingCommas = !strict;
    
    return JsonValueEnhanced::parse(json, options);
}
```

### 批量操作

```cpp
void batchUpdate(JsonValueEnhanced& data, const std::vector<std::string>& paths, 
                 const std::vector<JsonValueEnhanced>& values) {
    for (size_t i = 0; i < paths.size() && i < values.size(); ++i) {
        try {
            data.at(paths[i]) = values[i];
        } catch (const std::exception& e) {
            std::cerr << "Failed to update " << paths[i] << ": " << e.what() << std::endl;
        }
    }
}
```

## 🆚 与其他库对比

| 特性 | JsonValueEnhanced | nlohmann/json | RapidJSON | jsoncpp |
|------|-------------------|---------------|-----------|---------|
| C++17特性 | ✅ 完整支持 | ✅ 支持 | ❌ C++11 | ❌ C++11 |
| 类型安全 | ✅ std::variant | ✅ 模板 | ⚠️ 有限 | ⚠️ 有限 |
| JSON指针 | ✅ RFC 6901 | ✅ 支持 | ❌ 无 | ❌ 无 |
| 配置化解析 | ✅ 全面 | ⚠️ 有限 | ✅ 全面 | ⚠️ 有限 |
| 错误信息 | ✅ 详细位置 | ✅ 详细 | ✅ 详细 | ⚠️ 基本 |
| 性能 | ✅ 高 | ✅ 高 | ✅ 极高 | ⚠️ 中等 |
| 内存安全 | ✅ RAII | ✅ RAII | ⚠️ 手动 | ✅ RAII |
| 依赖 | ✅ 无 | ✅ 无 | ✅ 无 | ✅ 无 |

## 📝 最佳实践

### 1. 优先使用Optional接口
```cpp
// 好的做法
if (auto value = json["key"].getNumber()) {
    process(*value);
}

// 避免的做法 (可能隐藏错误)
double value = json["key"].toDouble(0.0);
```

### 2. 合理使用移动语义
```cpp
// 好的做法
JsonValueEnhanced data = createData();
container.emplace_back(std::move(data));

// 避免不必要的拷贝
const auto& ref = data.toObject();
```

### 3. 错误处理策略
```cpp
// 生产环境
try {
    auto parsed = JsonValueEnhanced::parse(userInput);
    return processData(parsed);
} catch (const std::exception& e) {
    logError("JSON parse error", e.what());
    return defaultResponse();
}
```

### 4. 性能敏感场景
```cpp
// 预分配
JsonValueEnhanced::ArrayType arr;
arr.reserve(expectedSize);

// 批量操作
JsonValueEnhanced::SerializeOptions fastOptions;
fastOptions.indent = -1;  // 紧凑格式
```

## 🔧 编译要求

- **C++标准**: C++17或更高
- **编译器**: 
  - GCC 7+
  - Clang 5+
  - MSVC 2017+
- **依赖**: 仅标准库，无外部依赖

## 📦 安装使用

### CMake集成
```cmake
# 添加到你的CMakeLists.txt
add_subdirectory(path/to/jsonstruct_registry)
target_link_libraries(your_target jsonstruct_enhanced_lib)
```

### 直接包含
```cpp
#include "json_value_enhanced.h"
// 开始使用！
```

---

*JsonValueEnhanced - 为现代C++设计的高性能JSON库* 🚀
