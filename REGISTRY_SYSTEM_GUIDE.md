# JsonStruct v2.0 - 基于类型注册的JSON序列化框架

## 🎯 设计目标

这个新版本的设计目标是创建一个可扩展的、模块化的JSON序列化框架，主要特点：

- **解耦**: 核心框架不依赖于具体的类型实现
- **可扩展**: 用户可以轻松注册新类型而不修改核心代码
- **模块化**: Qt类型、自定义类型可以在独立的文件中注册
- **向后兼容**: 保持原有的使用方式和API
- **标准C++支持**: 提供不依赖Qt的纯标准C++版本

## 📁 文件结构

### Qt版本（基于QJsonValue）
```
json_type_registry.h         # 核心类型注册系统（Qt版本）
jsonstruct_v2.h             # 重构后的核心JSON框架（Qt版本）
qt_types_registration.h     # Qt类型注册实现
jsonstruct_with_qt.h        # 便利头文件(包含Qt支持)
custom_types_example.h      # 自定义类型注册示例（Qt版本）
test_registry_system.cpp    # 新系统测试（Qt版本）
```

### 标准C++版本（不依赖Qt）
```
json_value.h                # 标准C++ JSON值实现
std_type_registry.h         # 核心类型注册系统（标准C++版本）
std_jsonstruct.h            # 核心JSON框架（标准C++版本）
std_types_registration.h   # 标准C++类型注册实现
jsonstruct_std.h            # 便利头文件(标准C++版本)
std_custom_types_example.h # 自定义类型注册示例（标准C++版本）
test_std_system.cpp         # 标准C++版本测试
```

## 🚀 快速开始

### Qt版本（推荐用于Qt项目）

```cpp
#include "jsonstruct_with_qt.h"

struct MyConfig {
    QString name = "default";
    QStringList items = {"a", "b", "c"};
    QPointF position = {10.0, 20.0};
    QColor color = Qt::red;
    bool enabled = true;
    
    JSON_AUTO(name, items, position, color, enabled)
};

// 使用
MyConfig config;
QJsonObject json = config.toJson();
config.fromJson(json);
```

### 标准C++版本（推荐用于非Qt项目）

```cpp
#include "jsonstruct_std.h"

struct MyConfig {
    std::string name = "default";
    std::vector<std::string> items = {"a", "b", "c"};
    Point3D position = {10.0, 20.0, 30.0};  // 自定义类型
    bool enabled = true;
    
    JSON_AUTO(name, items, position, enabled)
};

// 使用
MyConfig config;
JsonStruct::JsonObject json = config.toJson();
std::string jsonStr = JsonStruct::JsonValue(json).dump(2);

// 从JSON字符串恢复
JsonStruct::JsonValue parsed = JsonStruct::JsonValue::parse(jsonStr);
config.fromJson(parsed.toObject());
```

### 注册自定义类型

#### Qt版本 - 方法1: 使用宏（推荐）

```cpp
#include "json_type_registry.h"

struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

// 注册类型
REGISTER_JSON_TYPE(Point3D,
    // 序列化函数
    [](const Point3D& point) -> QJsonValue {
        QJsonArray arr;
        arr.append(point.x);
        arr.append(point.y);
        arr.append(point.z);
        return arr;
    },
    // 反序列化函数
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

#### 标准C++版本 - 方法1: 使用宏（推荐）

```cpp
#include "std_type_registry.h"

struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

// 注册类型
REGISTER_JSON_TYPE(Point3D,
    // 序列化函数
    [](const Point3D& point) -> JsonStruct::JsonValue {
        JsonStruct::JsonValue::ArrayType arr;
        arr.push_back(JsonStruct::JsonValue(point.x));
        arr.push_back(JsonStruct::JsonValue(point.y));
        arr.push_back(JsonStruct::JsonValue(point.z));
        return JsonStruct::JsonValue(arr);
    },
    // 反序列化函数
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
    // 反序列化函数
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

#### 方法2: 手动注册

```cpp
// 在程序初始化时
JsonStruct::TypeRegistry::instance().registerType<Point3D>(
    // toJson
    [](const Point3D& point) -> QJsonValue {
        QJsonArray arr;
        arr.append(point.x);
        arr.append(point.y);
        arr.append(point.z);
        return arr;
    },
    // fromJson
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

## 🔧 高级功能

### 1. 检查类型是否已注册

```cpp
if (JsonStruct::TypeRegistry::instance().isRegistered<MyType>()) {
    // 类型已注册，可以使用
}
```

### 2. 直接使用类型注册器

```cpp
auto& registry = JsonStruct::TypeRegistry::instance();

// 直接序列化
QJsonValue json = registry.toJson(myObject);

// 直接反序列化
MyType restored = registry.fromJson<MyType>(json, defaultValue);
```

### 3. 获取所有已注册类型

```cpp
auto types = JsonStruct::TypeRegistry::instance().getRegisteredTypes();
std::cout << "已注册 " << types.size() << " 种类型" << std::endl;
```

### 4. 组合使用已注册类型

```cpp
REGISTER_JSON_TYPE(ComplexType,
    [](const ComplexType& obj) -> QJsonValue {
        QJsonObject json;
        // 使用已注册的类型
        json["point"] = JsonStruct::TypeRegistry::instance().toJson(obj.point3d);
        json["colors"] = JsonStruct::TypeRegistry::instance().toJson(obj.colorList);
        return json;
    },
    [](const QJsonValue& json, const ComplexType& defaultValue) -> ComplexType {
        // 类似的反序列化逻辑
    }
);
```

## 📋 已预注册的类型

### Qt类型 (qt_types_registration.h)
- `QStringList` - JSON数组格式
- `QPointF` - JSON对象格式 `{"x": 1.0, "y": 2.0}`
- `QRectF` - JSON数组格式 `[x, y, width, height]`
- `QRect` - JSON数组格式 `[x, y, width, height]`
- `QColor` - JSON数组格式 `[r, g, b, a]`
- `QList<QPointF>` - JSON数组，元素为QPointF对象

### 标准C++类型 (std_types_registration.h)
- `std::vector<int>` - JSON数组格式
- `std::vector<std::string>` - JSON数组格式
- `std::list<int>` - JSON数组格式
- `std::map<std::string, int>` - JSON对象格式
- `std::map<std::string, std::string>` - JSON对象格式

### 基础类型 (内置支持)
- `bool`, `int`, `double`, `float`等算术类型
- `QString`(Qt版本), `std::string`(标准C++版本)
- 枚举类型 (`enum` / `enum class`)
- 具有 `JSON_AUTO` 宏的自定义结构体

## 🔍 最佳实践

### 1. 模块化注册

将相关类型的注册放在独立的文件中：

```cpp
// graphics_types.h
namespace GraphicsTypes {
    void registerAllGraphicsTypes() {
        registerPoint3D();
        registerMatrix4x4();
        registerBoundingBox();
        // ...
    }
    
private:
    static void registerPoint3D() { /* ... */ }
    static void registerMatrix4x4() { /* ... */ }
    // ...
}
```

### 2. 错误处理

注册函数应该包含适当的错误处理：

```cpp
REGISTER_JSON_TYPE(MyType,
    [](const MyType& obj) -> QJsonValue {
        try {
            // 序列化逻辑
            return result;
        } catch (...) {
            return QJsonValue(); // 返回空值
        }
    },
    [](const QJsonValue& json, const MyType& defaultValue) -> MyType {
        if (!json.isObject()) {
            return defaultValue; // 输入无效时返回默认值
        }
        // 反序列化逻辑
    }
);
```

### 3. 版本兼容性

为复杂类型考虑版本兼容性：

```cpp
REGISTER_JSON_TYPE(VersionedType,
    [](const VersionedType& obj) -> QJsonValue {
        QJsonObject json;
        json["version"] = 1;
        json["data"] = /* 实际数据 */;
        return json;
    },
    [](const QJsonValue& json, const VersionedType& defaultValue) -> VersionedType {
        if (json.isObject()) {
            auto obj = json.toObject();
            int version = obj["version"].toInt(1);
            
            switch (version) {
                case 1:
                    return parseV1(obj["data"]);
                case 2:
                    return parseV2(obj["data"]);
                default:
                    return defaultValue;
            }
        }
        return defaultValue;
    }
);
```

## 🐛 与旧版本的区别

### 空容器处理
新版本正确处理空容器：
```cpp
QStringList empty;  // 空列表
QJsonObject json = toJson(empty);  // 序列化为 []
QStringList restored = fromJson(json);  // 正确恢复为空列表
```

### 数组边界检查
新版本包含严格的边界检查：
```cpp
QJsonArray shortArray = {1, 2};  // 只有2个元素
QRect rect = fromJsonValue<QRect>(shortArray, QRect());  // 安全地返回默认值
```

### 类型安全
通过注册系统，类型处理更加安全和可预测。

## 🧪 测试

### Qt版本测试
```bash
# 编译Qt版本测试
qmake && make
./test_registry_system

# 或运行原有测试验证兼容性
./test_jsonstruct
```

### 标准C++版本测试
```bash
# 编译标准C++版本测试
g++ -std=c++11 -I. test_std_system.cpp -o test_std_system
./test_std_system

# 或使用CMake
mkdir build && cd build
cmake ..
make
./test_std_system
```

## 📈 性能考虑

- 类型查找使用 `std::unordered_map`，查找复杂度为 O(1)
- 注册发生在程序启动时，运行时性能无影响
- `std::any` 的使用有一定开销，但在可接受范围内
- 标准C++版本的JsonValue实现针对性能进行了优化
- 对于高频使用的类型，建议提供特化的优化实现

## 🔮 未来扩展

这个设计为未来扩展提供了基础：

1. **插件系统**: 可以创建动态加载的类型注册插件
2. **反射支持**: 可以集成C++反射库实现自动注册
3. **模式验证**: 可以为注册类型添加JSON Schema验证
4. **性能优化**: 可以添加类型特化的快速路径
5. **更多序列化格式**: 可以扩展支持MessagePack、CBOR等二进制格式
6. **流式处理**: 为大型数据集提供流式序列化/反序列化

## 📞 使用建议

### 选择版本
1. **Qt项目**: 使用 `jsonstruct_with_qt.h`，享受完整的Qt类型支持
2. **非Qt项目**: 使用 `jsonstruct_std.h`，获得纯标准C++实现
3. **混合项目**: 可以同时使用两个版本，但注意避免命名冲突

### 项目集成
1. **简单项目**: 直接包含对应的便利头文件
2. **复杂项目**: 创建专门的类型注册模块
3. **库开发**: 提供独立的类型注册头文件
4. **性能关键**: 考虑为热点类型提供专门的优化实现

### 编译要求
- **Qt版本**: Qt 5.12+ (支持C++17特性)
- **标准C++版本**: C++11及以上（不依赖外部库）
- **推荐**: C++14及以上，获得更好的性能和特性支持
