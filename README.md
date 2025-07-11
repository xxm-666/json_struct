# JsonStruct Registry System - 标准C++版本

这个目录包含了基于类型注册的JSON序列化框架的完整实现，提供了Qt版本和标准C++版本两种选择。

## 🎯 特性

- **类型注册系统**: 动态注册任意类型的序列化/反序列化逻辑
- **模块化设计**: 核心框架与具体类型实现解耦
- **两种版本**:
  - **Qt版本## ⚠️ JsonValue 当前限制与改进

### 🔍 版本选择指南

本项目提供了两个版本的JsonValue，满足不同需求：

#### 📋 版本对比

| 特性 | JsonValue (基础版) | JsonValueEnhanced (增强版) |
|------|-------------------|---------------------------|
| **C++标准** | C++11+ | C++17+ |
| **定位** | 兼容性优先 | 功能完整，推荐使用 |
| **类型安全** | Union | std::variant ✅ |
| **性能** | std::map | std::unordered_map ✅ |
| **JSON指针** | ❌ | ✅ (RFC 6901) |
| **注释支持** | ❌ | ✅ (JSON5风格) |
| **Unicode** | 基础支持 | ✅ 完整支持(代理对) |
| **错误信息** | 简单 | ✅ 详细位置信息 |
| **现代特性** | 基础 | ✅ 移动语义、optional等 |

### 🔍 基础版本限制
虽然 JsonValue 提供了完整的基础JSON功能，但存在一些限制，详情请参考 [ENHANCED_LIMITATIONS_ANALYSIS.md](ENHANCED_LIMITATIONS_ANALYSIS.md)：

#### 主要限制
- **Unicode转义**: 不支持 `\uXXXX` 转义序列
- **数值精度**: 所有数字转为double，大整数可能丢失精度
- **性能优化**: 使用std::map而非unordered_map，某些操作较慢
- **错误信息**: 缺少详细的位置信息（行号、列号）
- **高级特性**: 不支持JSON5注释、JSONPath查询、流式解析等Value，与Qt项目无缝集成
  - **标准C++版本**: 纯标准C++实现，不依赖任何外部库
- **可扩展**: 用户可以轻松注册自定义类型
- **高性能**: 基于std::unordered_map的O(1)类型查找

## 📁 文件结构

### Qt版本
```
json_type_registry.h         # Qt版本类型注册系统
jsonstruct_v2.h             # Qt版本核心框架
qt_types_registration.h     # Qt类型注册实现
jsonstruct_with_qt.h        # Qt版本便利头文件
custom_types_example.h      # Qt版本自定义类型示例
test_registry_system.cpp    # Qt版本测试
```

### 标准C++版本
```
json_value.h                # 标准C++ JSON值实现
json_value.cpp              # JsonValue实现文件
std_type_registry.h         # 标准C++版本类型注册系统
std_jsonstruct.h            # 标准C++版本核心框架
std_types_registration.h   # 标准C++类型注册实现
jsonstruct_std.h            # 标准C++版本便利头文件
std_custom_types_example.h # 标准C++版本自定义类型示例
test_std_system.cpp         # 标准C++版本测试
```

### 构建文件
```
CMakeLists.txt              # 统一的CMake构建文件
REGISTRY_SYSTEM_GUIDE.md   # 详细使用指南
README.md                   # 本文件
```

## 🚀 快速开始

### 标准C++版本（推荐用于非Qt项目）

```cpp
#include "jsonstruct_std.h"

struct Config {
    std::string name = "test";
    int value = 42;
    std::vector<std::string> items = {"a", "b", "c"};
    
    JSON_AUTO(name, value, items)
};

int main() {
    Config config;
    
    // 序列化
    auto json = config.toJson();
    std::string jsonStr = JsonStruct::JsonValue(json).dump(2);
    std::cout << jsonStr << std::endl;
    
    // 反序列化
    auto parsed = JsonStruct::JsonValue::parse(jsonStr);
    config.fromJson(parsed.toObject());
    
    return 0;
}
```

### Qt版本（推荐用于Qt项目）

```cpp
#include "jsonstruct_with_qt.h"

struct Config {
    QString name = "test";
    int value = 42;
    QStringList items = {"a", "b", "c"};
    QPointF position = {10.0, 20.0};
    
    JSON_AUTO(name, value, items, position)
};

int main() {
    Config config;
    
    // 序列化
    QJsonObject json = config.toJson();
    QJsonDocument doc(json);
    QString jsonStr = doc.toJson();
    
    // 反序列化
    QJsonDocument parsed = QJsonDocument::fromJson(jsonStr.toUtf8());
    config.fromJson(parsed.object());
    
    return 0;
}
```

## 🔧 构建

### 使用CMake

```bash
mkdir build && cd build
cmake ..
make

# 运行Qt版本测试
./json_struct_qt

# 运行标准C++版本测试
./json_struct_std

# 运行JSON解析演示
./demo_json_parsing_en
```

### 仅标准C++版本

```bash
# 直接编译
g++ -std=c++11 -I. json_value.cpp test_std_system.cpp -o test_std
./test_std

# 或使用CMake仅构建标准C++版本
cmake -DBUILD_QT_VERSION=OFF ..
make json_struct_std
```

## 💡 JsonValue 完整JSON解析功能

JsonValue 类提供了完整的JSON解析和操作能力，可以处理任何标准的JSON格式：

### 支持的JSON类型
- **基础类型**: `null`, `bool`, `number`, `string`
- **复合类型**: `array`, `object`
- **嵌套结构**: 任意深度的对象和数组嵌套

### 主要功能
```cpp
#include "json_value.h"

// 1. 解析JSON字符串
std::string jsonStr = R"({
    "name": "Alice", 
    "age": 25, 
    "hobbies": ["reading", "coding"], 
    "address": {"city": "Beijing", "country": "China"}
})";
auto json = JsonStruct::JsonValue::parse(jsonStr);

// 2. 访问数据
std::string name = json["name"].toString();
int age = json["age"].toInt();
std::string city = json["address"]["city"].toString();

// 3. 遍历数组
auto hobbies = json["hobbies"];
for (size_t i = 0; i < hobbies.size(); ++i) {
    std::cout << hobbies[i].toString() << std::endl;
}

// 4. 动态构建JSON
JsonStruct::JsonValue newObj;
newObj["title"] = JsonStruct::JsonValue("Test");
newObj["count"] = JsonStruct::JsonValue(42);

JsonStruct::JsonValue arr;
arr.append(JsonStruct::JsonValue("item1"));
arr.append(JsonStruct::JsonValue("item2"));
newObj["items"] = arr;

// 5. 序列化为字符串
std::string output = newObj.dump(2); // 带缩进的美化输出
```

### 错误处理
```cpp
try {
    auto parsed = JsonStruct::JsonValue::parse("invalid json");
} catch (const std::exception& e) {
    std::cout << "Parse error: " << e.what() << std::endl;
}
```

## ⚠️ JsonValue 当前限制与改进

### 🔍 基础版本限制
虽然 JsonValue 提供了完整的基础JSON功能，但存在一些限制，详情请参考 [JSONVALUE_LIMITATIONS.md](JSONVALUE_LIMITATIONS.md)：

#### 主要限制
- **Unicode转义**: 不支持 `\uXXXX` 转义序列
- **数值精度**: 所有数字转为double，大整数可能丢失精度
- **性能优化**: 使用std::map而非unordered_map，某些操作较慢
- **错误信息**: 缺少详细的位置信息（行号、列号）
- **高级特性**: 不支持JSON5注释、JSONPath查询、流式解析等

### ✨ 增强版本解决方案
我们还提供了 **JsonValue Enhanced** 增强版本，解决了大部分限制：

```cpp
#include "json_value_enhanced.h"
using namespace JsonStruct;

// 现代C++17+特性
auto json = R"({"emoji": "\uD83D\uDE00", "numbers": [1,2,3,]})"_json;
auto value = json.at("/numbers/0");  // JSON指针支持

// 配置化解析
ParseOptions options;
options.allowComments = true;
options.allowTrailingCommas = true;
auto parsed = JsonValueEnhanced::parse(jsonWithComments, options);

// 安全的可选值访问
if (auto num = json["age"].getNumber()) {
    std::cout << "Age: " << *num << std::endl;
}
```

#### 增强版本特性
- ✅ **完整Unicode支持** (包括代理对)
- ✅ **高性能** (std::unordered_map + O(1)查找)
- ✅ **详细错误信息** (行列位置)
- ✅ **JSON指针支持** (RFC 6901)
- ✅ **注释和尾随逗号** (JSON5风格)
- ✅ **类型安全** (std::variant + std::optional)
- ✅ **现代C++17+特性** (移动语义、string_view等)

详细对比分析请参考：[ENHANCED_LIMITATIONS_ANALYSIS.md](ENHANCED_LIMITATIONS_ANALYSIS.md)

### 适用场景对比

| 使用场景 | 基础版本 | 增强版本 |
|---------|---------|---------|
| 简单JSON处理 | ✅ 推荐 | ✅ 推荐 |
| 配置文件读写 | ✅ 推荐 | ✅ 更佳 |
| API数据交换 | ✅ 适合 | ✅ 更佳 |
| 大型JSON文件 | ⚠️ 慎用 | ✅ 适合 |
| 高性能要求 | ⚠️ 一般 | ✅ 优秀 |
| 现代C++项目 | ⚠️ 基本 | ✅ 完美匹配 |
| Unicode处理 | ❌ 限制 | ✅ 完整支持 |
| 复杂查询 | ❌ 不支持 | ✅ JSON指针 |

### 建议选择
- **基础版本** (`json_value.h`): 需要C++11兼容性、简单项目、学习用途
- **增强版本** (`json_value_enhanced.h`): 现代C++17+项目、生产环境、功能丰富需求（**强烈推荐**）

## 📋 预注册类型

### 标准C++版本
- `std::vector<int>`
- `std::vector<std::string>`
- `std::list<int>`
- `std::map<std::string, int>`
- `std::map<std::string, std::string>`
- 基础类型：`bool`, `int`, `double`, `std::string`等

### Qt版本
- `QStringList`
- `QPointF`
- `QRectF`
- `QRect`
- `QColor`
- `QList<QPointF>`
- 基础类型：`bool`, `int`, `double`, `QString`等

## 🎨 自定义类型注册

### 标准C++版本

```cpp
struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

REGISTER_JSON_TYPE(Point3D,
    // 序列化
    [](const Point3D& p) -> JsonStruct::JsonValue {
        JsonStruct::JsonValue::ArrayType arr;
        arr.push_back(JsonStruct::JsonValue(p.x));
        arr.push_back(JsonStruct::JsonValue(p.y));
        arr.push_back(JsonStruct::JsonValue(p.z));
        return JsonStruct::JsonValue(arr);
    },
    // 反序列化
    [](const JsonStruct::JsonValue& json, const Point3D& defaultValue) -> Point3D {
        if (json.isArray()) {
            const auto& arr = json.toArray();
            if (arr.size() == 3) {
                return Point3D(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
            }
        }
        return defaultValue;
    }
);
```

### Qt版本

```cpp
struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

REGISTER_JSON_TYPE(Point3D,
    // 序列化
    [](const Point3D& p) -> QJsonValue {
        QJsonArray arr;
        arr.append(p.x);
        arr.append(p.y);
        arr.append(p.z);
        return arr;
    },
    // 反序列化
    [](const QJsonValue& json, const Point3D& defaultValue) -> Point3D {
        if (json.isArray()) {
            auto arr = json.toArray();
            if (arr.size() == 3) {
                return Point3D(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
            }
        }
        return defaultValue;
    }
);
```

## 📊 性能特点

- **类型查找**: O(1) 复杂度，基于std::unordered_map
- **内存管理**: 
  - Qt版本：依赖Qt的引用计数和写时复制
  - 标准C++版本：使用RAII和智能指针管理
- **编译时间**: 模块化设计减少编译依赖
- **运行时开销**: 最小化虚函数调用和dynamic_cast

## 🔍 兼容性

### 编译器要求
- **标准C++版本**: C++11及以上（推荐C++14+）
- **Qt版本**: C++17及以上（Qt 5.12+）

### 平台支持
- Windows (MSVC, MinGW)
- Linux (GCC, Clang)
- macOS (Clang)

## 📖 详细文档

请参考 [REGISTRY_SYSTEM_GUIDE.md](REGISTRY_SYSTEM_GUIDE.md) 获取完整的使用指南和最佳实践。

## 🤝 贡献

1. 这个框架设计为模块化和可扩展的
2. 可以添加新的序列化格式支持（如MessagePack、CBOR）
3. 可以优化性能热点
4. 可以添加更多预定义类型支持

## 📝 许可证

请遵循项目原有的许可证条款。

---

这个系统提供了强大而灵活的JSON序列化解决方案，既保持了易用性，又提供了高度的可扩展性。无论是Qt项目还是纯C++项目，都能找到合适的版本。

## 🆕 增强版C++17+ JsonValue

### 最新特性

我们新增了一个完全基于现代C++17+标准的增强版JsonValue实现：

```
json_value_enhanced.h           # C++17+增强版JSON值实现
json_value_enhanced.cpp         # 增强版实现文件
test_enhanced_features.cpp      # 增强版全面测试
JSON_VALUE_ENHANCED_GUIDE.md    # 详细使用指南
```

### 🌟 增强版特性

- **🔥 现代C++**: 完全基于C++17+，使用std::variant、std::optional、std::string_view
- **🚀 高性能**: std::unordered_map、移动语义优化、内存预分配
- **🛡️ 类型安全**: 基于std::variant的类型系统，完全避免union的安全问题
- **🔧 丰富功能**: JSON指针(RFC 6901)、配置化解析、高级序列化选项
- **📦 容器集成**: 原生支持std::array、std::vector、std::set、std::map等
- **✨ 现代语法**: 字面量语法(_json)、访问者模式、工厂函数
- **🌍 Unicode**: 完整Unicode支持，包括代理对处理
- **⚙️ 可配置**: 注释支持、尾随逗号、深度限制、严格模式等

### 快速体验

```cpp
#include "json_value_enhanced.h"
using namespace JsonStruct;
using namespace JsonStruct::literals;

// 现代语法
auto config = R"({
    "app": "MyApp",
    "users": ["Alice", "Bob"],
    "settings": {"debug": true}
})"_json;

// 类型安全访问
if (auto debug = config.at("/settings/debug").getBool()) {
    std::cout << "Debug mode: " << (*debug ? "on" : "off") << std::endl;
}

// 容器集成
std::vector<std::string> names = {"Charlie", "David"};
config["users"] = JsonValueEnhanced(names);

// 高级序列化
JsonValueEnhanced::SerializeOptions options;
options.indent = 2;
options.sortKeys = true;
std::cout << config.dump(options) << std::endl;
```
