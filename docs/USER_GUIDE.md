# JsonStruct - 用户指南与快速入门

> 本文档合并了原 USER_GUIDE.md 和 QUICK_START.md 的内容，提供完整的用户指南。

## 1. 快速入门

### 1.1 基本使用

```cpp
#include "jsonstruct_std.h"
using namespace JsonStruct;

int main() {
    // 创建JSON值
    JsonValue json;
    json["name"] = "Alice";
    json["age"] = 30;
    json["active"] = true;
    
    // 序列化为字符串
    std::string jsonString = json.serialize(2);
    std::cout << jsonString << std::endl;
    
    return 0;
}
```

输出：
```json
{
  "active": true,
  "age": 30,
  "name": "Alice"
}
```

### 2. 解析JSON字符串

```cpp
std::string jsonText = R"({
    "name": "Bob",
    "age": 25,
    "skills": ["C++", "Python", "JSON"]
})";

JsonValue parsed = JsonValue::parse(jsonText);
std::cout << "Name: " << parsed["name"].toString() << std::endl;
std::cout << "Skills count: " << parsed["skills"].size() << std::endl;
```

### 3. 处理数组

```cpp
JsonValue array = JsonValue::parse(R"(["C++", "Python", "JSON"])");
for (const auto& item : array.toArray()) {
    std::cout << item.toString() << std::endl;
}
```

### 4. 嵌套对象解析

```cpp
JsonValue nested = JsonValue::parse(R"({"person": {"name": "Alice", "age": 30}})");
std::cout << "Name: " << nested["person"]["name"].toString() << std::endl;
std::cout << "Age: " << nested["person"]["age"].toInt() << std::endl;
```

### 5. 使用类型注册模块

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

## 高级功能

### 1. 高精度数值处理

```cpp
// 处理大整数（超过JavaScript安全整数范围）
JsonValue bigInt(9007199254740993LL); // 2^53 + 1
std::cout << "Is integer: " << bigInt.isInteger() << std::endl;
std::cout << "Value: " << bigInt.toLongLong() << std::endl;

// 特殊数值
JsonValue::ParseOptions options;
options.allowSpecialNumbers = true;

JsonValue nanValue = JsonValue::parse("NaN", options);
JsonValue infValue = JsonValue::parse("Infinity", options);
```

### 2. 自定义类型注册

```cpp
// 定义自定义结构
struct Point3D {
    double x, y, z;
    Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

// 注册类型
REGISTER_JSON_TYPE(Point3D,
    // 序列化函数
    [](const Point3D& point) -> JsonValue {
        JsonValue::ArrayType arr;
        arr.push_back(JsonValue(point.x));
        arr.push_back(JsonValue(point.y));
        arr.push_back(JsonValue(point.z));
        return JsonValue(arr);
    },
    // 反序列化函数
    [](const JsonValue& json, const Point3D& defaultValue) -> Point3D {
        if (json.isArray()) {
            const auto& arr = json.toArray();
            if (arr.size() == 3) {
                return Point3D(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
            }
        }
        return defaultValue;
    }
);

// 使用类型注册
Point3D point(1.0, 2.0, 3.0);
JsonValue pointJson = TypeRegistry::instance().toJson(point);
Point3D restored = TypeRegistry::instance().fromJson<Point3D>(pointJson, Point3D());
```

### 3. STL容器支持

```cpp
// 自动支持标准容器
std::vector<int> intVec = {1, 2, 3, 4, 5};
JsonValue vecJson = TypeRegistry::instance().toJson(intVec);

std::map<std::string, int> intMap;
intMap["first"] = 1;
intMap["second"] = 2;
JsonValue mapJson = TypeRegistry::instance().toJson(intMap);
```

### 4. JSON指针查询

```cpp
JsonValue data = JsonValue::parse(R"({
    "users": [
        {"name": "Alice", "profile": {"age": 30}},
        {"name": "Bob", "profile": {"age": 25}}
    ]
})");

// 使用JSON指针查询
JsonValue name = query(data, "/users/0/name");           // "Alice"
JsonValue age = query(data, "/users/0/profile/age");     // 30
```

## 错误处理

### 1. 解析错误

```cpp
try {
    JsonValue json = JsonValue::parse("{invalid json}");
} catch (const JsonParseException& e) {
    std::cout << "Parse error: " << e.what() << std::endl;
    std::cout << "At " << e.locationInfo() << std::endl;
}
```

### 2. 类型转换错误

```cpp
JsonValue stringValue("hello");
try {
    int number = stringValue.toInt(); // 抛出异常
} catch (const JsonTypeException& e) {
    std::cout << "Type error: " << e.what() << std::endl;
}

// 安全的类型获取
auto number = stringValue.getInt();
if (number) {
    std::cout << "Number: " << *number << std::endl;
} else {
    std::cout << "Not a number" << std::endl;
}
```

## 性能优化

### 1. 移动语义

```cpp
// 使用移动语义避免拷贝
JsonValue createLargeArray() {
    JsonValue array;
    for (int i = 0; i < 10000; ++i) {
        array.append(JsonValue(i));
    }
    return array; // 自动移动
}

JsonValue large = createLargeArray(); // 不会发生拷贝
```

### 2. 预分配内存

```cpp
// 对于已知大小的对象，可以预先设置
JsonValue obj;
// 插入已知的键值对，避免多次哈希表扩容
obj["key1"] = "value1";
obj["key2"] = "value2";
// ...
```

### 3. 流式解析大文件

```cpp
JsonStreamParser parser;
std::ifstream file("large.json");
std::string chunk;

while (std::getline(file, chunk)) {
    parser.feedData(chunk);
    
    JsonStreamParser::Event event;
    while ((event = parser.nextEvent()) != JsonStreamParser::Event::Error) {
        switch (event) {
            case JsonStreamParser::Event::ObjectStart:
                // 处理对象开始
                break;
            case JsonStreamParser::Event::Value:
                // 处理值
                break;
            // 其他事件...
        }
    }
}
```

## 最佳实践

### 1. 异常安全

```cpp
// 总是捕获可能的异常
try {
    JsonValue json = JsonValue::parse(userInput);
    // 处理数据
} catch (const JsonParseException& e) {
    // 处理解析错误
} catch (const JsonTypeException& e) {
    // 处理类型错误
}
```

### 2. 类型检查

```cpp
JsonValue value = getValueFromSomewhere();

// 在访问前检查类型
if (value.isString()) {
    std::string str = value.toString();
} else if (value.isNumber()) {
    double num = value.toDouble();
}

// 或使用安全访问
auto str = value.getString();
if (str) {
    // 使用 *str
}
```

### 3. 配置选项

```cpp
// 根据需要配置解析选项
JsonValue::ParseOptions options;
options.allowComments = true;        // 允许注释
options.allowTrailingCommas = true;  // 允许尾随逗号
options.maxDepth = 100;              // 限制嵌套深度

JsonValue json = JsonValue::parse(jsonString, options);
```

### 4. 序列化选项

```cpp
// 配置序列化格式
JsonValue::SerializeOptions options;
options.indent = 4;                  // 4空格缩进
options.sortKeys = true;             // 排序对象键
options.escapeUnicode = false;       // 保持Unicode字符

std::string result = json.serialize(options);
```

## 2. 类型注册与自定义类型

### 2.1 定义和注册自定义类型

```cpp
#include "jsonstruct_std.h"
#include <iostream>

using namespace JsonStruct;

// 定义自定义结构
struct Point {
    double x, y;
    Point(double x = 0, double y = 0) : x(x), y(y) {}
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

// 注册自定义类型
REGISTER_JSON_TYPE(Point,
    // 序列化函数
    [](const Point& p) -> JsonValue {
        JsonValue obj;
        obj["x"] = p.x;
        obj["y"] = p.y;
        return obj;
    },
    // 反序列化函数
    [](const JsonValue& json, const Point& defaultValue) -> Point {
        if (json.isObject()) {
            double x = json.contains("x") ? json["x"].toDouble() : 0.0;
            double y = json.contains("y") ? json["y"].toDouble() : 0.0;
            return Point(x, y);
        }
        return defaultValue;
    }
);

int main() {
    // 使用类型注册
    Point original(3.14, 2.71);
    
    // 序列化
    JsonValue pointJson = TypeRegistry::instance().toJson(original);
    std::string jsonStr = pointJson.serialize();
    std::cout << "Point JSON: " << jsonStr << std::endl;
    
    // 反序列化
    Point restored = TypeRegistry::instance().fromJson<Point>(pointJson, Point());
    std::cout << "Restored Point: (" << restored.x << ", " << restored.y << ")" << std::endl;
    std::cout << "相等: " << (original == restored ? "是" : "否") << std::endl;
    
    return 0;
}
```

### 2.2 STL 容器支持

JsonStruct 自动支持常用的 STL 容器类型：

```cpp
#include "jsonstruct_std.h"
#include <vector>
#include <map>
#include <set>

using namespace JsonStruct;

int main() {
    // Vector 示例
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    JsonValue numbersJson = TypeRegistry::instance().toJson(numbers);
    std::cout << "Vector JSON: " << numbersJson.serialize() << std::endl;
    
    // Map 示例
    std::map<std::string, std::string> userInfo = {
        {"name", "Alice"},
        {"city", "Beijing"},
        {"job", "Engineer"}
    };
    JsonValue mapJson = TypeRegistry::instance().toJson(userInfo);
    std::cout << "Map JSON: " << mapJson.serialize(2) << std::endl;
    
    // Set 示例
    std::set<std::string> tags = {"cpp", "json", "library"};
    JsonValue setJson = TypeRegistry::instance().toJson(tags);
    std::cout << "Set JSON: " << setJson.serialize() << std::endl;
    
    return 0;
}
```

### 2.3 嵌套容器支持

```cpp
// 复杂嵌套结构
std::map<std::string, std::vector<Point>> pointGroups = {
    {"group1", {Point(1.0, 2.0), Point(3.0, 4.0)}},
    {"group2", {Point(5.0, 6.0), Point(7.0, 8.0)}}
};

JsonValue complexJson = TypeRegistry::instance().toJson(pointGroups);
std::cout << "Complex structure: " << complexJson.serialize(2) << std::endl;

// 反序列化
auto restored = TypeRegistry::instance().fromJson<decltype(pointGroups)>(complexJson, {});
```

## 3. 数组和对象操作详解

### 3.1 数组操作示例

```cpp
// 创建和操作数组
JsonValue fruits;
fruits.append("apple");
fruits.append("banana");
fruits.append("orange");

// 访问数组元素
for (size_t i = 0; i < fruits.size(); ++i) {
    std::cout << "Fruit " << i << ": " << fruits[i].toString() << std::endl;
}

// 修改数组元素
fruits[1] = "grape";

// 遍历数组
for (const auto& fruit : fruits.toArray()) {
    std::cout << "Fruit: " << fruit.toString() << std::endl;
}
```

### 3.2 对象操作示例

```cpp
// 创建对象
JsonValue person;
person["name"] = "Bob";
person["age"] = 25;
person["married"] = false;

// 嵌套对象
JsonValue address;
address["street"] = "123 Main St";
address["city"] = "New York";
person["address"] = address;

// 检查键是否存在
if (person.contains("address")) {
    JsonValue addr = person["address"];
    std::cout << "City: " << addr["city"].toString() << std::endl;
}

// 遍历对象的所有键值对
for (const auto& [key, value] : person.toObject()) {
    std::cout << key << ": ";
    if (value.isString()) {
        std::cout << value.toString();
    } else if (value.isNumber()) {
        std::cout << value.toDouble();
    } else if (value.isBool()) {
        std::cout << (value.toBool() ? "true" : "false");
    } else {
        std::cout << value.serialize();
    }
    std::cout << std::endl;
}
```
