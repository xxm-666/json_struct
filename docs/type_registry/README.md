# Type 模块

## 功能概述
- 提供类型注册功能，支持自定义类型的序列化和反序列化。
- 支持自动序列化和字段宏定义。

## 使用示例

### 注册自定义类型
```cpp
#include "type_registry/auto_serializer.h"
using namespace JsonStruct;

struct Person {
    std::string name;
    int age;
    JS_OBJ(name, age);
};

Person alice = {"Alice", 30};
JsonValue json = AutoSerializer::serialize(alice);
std::cout << json.serialize(2) << std::endl;
```

## 核心功能

### 自动序列化
`auto_serializer.h` 提供了自动序列化功能，支持基本类型和注册类型。

#### 示例
```cpp
#include "type_registry/auto_serializer.h"
using namespace JsonStruct;

struct Person {
    std::string name;
    int age;
    JS_OBJ(name, age);
};

Person alice = {"Alice", 30};
JsonValue json = AutoSerializer::serialize(alice);
std::cout << json.serialize(2) << std::endl;
```

### 字段宏定义
`field_macros.h` 提供了核心宏 `JSON_FIELDS`，用于定义类型的字段。

#### 示例
```cpp
#include "type_registry/field_macros.h"
using namespace JsonStruct;

struct Point {
    double x, y, z;
    JSON_FIELDS(x, y, z);
};

Point p = {1.0, 2.0, 3.0};
JsonValue json = AutoSerializer::serialize(p);
std::cout << json.serialize(2) << std::endl;
```

### 类型注册核心
`registry_core.h` 提供了类型注册的核心功能，支持自定义序列化和反序列化。

#### 示例
```cpp
#include "type_registry/registry_core.h"
using namespace JsonStruct;

struct CustomType {
    int id;
    std::string name;
};

TypeRegistry::instance().registerType<CustomType>(
    [](const CustomType& obj) -> JsonValue {
        JsonValue::ObjectType json;
        json["id"] = obj.id;
        json["name"] = obj.name;
        return JsonValue(json);
    },
    [](const JsonValue& json, const CustomType& defaultValue) -> CustomType {
        CustomType obj;
        obj.id = json["id"].toInt();
        obj.name = json["name"].toString();
        return obj;
    }
);

CustomType custom = {1, "Example"};
JsonValue json = TypeRegistry::instance().toJson(custom);
std::cout << json.serialize(2) << std::endl;
```
