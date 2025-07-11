# JsonStruct Registry - 快速开始示例

本文档提供了 JsonStruct Registry 的基本使用示例，帮助您快速上手。

## 基础示例

### 1. 简单的JSON操作

```cpp
#include "jsonstruct_std.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    // 创建JSON对象
    JsonValue person;
    person["name"] = "Alice";
    person["age"] = 30;
    person["city"] = "Beijing";
    
    // 创建数组
    JsonValue hobbies;
    hobbies.append("reading");
    hobbies.append("coding");
    hobbies.append("traveling");
    person["hobbies"] = hobbies;
    
    // 序列化为字符串
    std::string jsonString = person.serialize(2); // 2空格缩进
    std::cout << "生成的JSON:\n" << jsonString << std::endl;
    
    // 解析JSON字符串
    JsonValue parsed = JsonValue::parse(jsonString);
    std::cout << "\n解析结果:" << std::endl;
    std::cout << "姓名: " << parsed["name"].toString() << std::endl;
    std::cout << "年龄: " << parsed["age"].toInt() << std::endl;
    std::cout << "爱好数量: " << parsed["hobbies"].toArray().size() << std::endl;
    
    return 0;
}
```

### 2. 类型注册示例

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

### 3. STL容器支持示例

```cpp
#include "jsonstruct_std.h"
#include <vector>
#include <map>
#include <iostream>

using namespace JsonStruct;

int main() {
    // std::vector 支持
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    JsonValue numbersJson = TypeRegistry::instance().toJson(numbers);
    std::cout << "Vector JSON: " << numbersJson.serialize() << std::endl;
    
    // std::map 支持
    std::map<std::string, std::string> config;
    config["host"] = "localhost";
    config["port"] = "8080";
    config["database"] = "mydb";
    
    JsonValue configJson = TypeRegistry::instance().toJson(config);
    std::cout << "Map JSON: " << configJson.serialize(2) << std::endl;
    
    // 反序列化
    auto restoredNumbers = TypeRegistry::instance().fromJson<std::vector<int>>(numbersJson, {});
    std::cout << "恢复的数组大小: " << restoredNumbers.size() << std::endl;
    
    return 0;
}
```

### 4. 高精度数值示例

```cpp
#include "jsonstruct_std.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    // 大整数测试
    long long bigNumber = 9007199254740993LL; // 超过JavaScript安全整数范围
    JsonValue bigJson(bigNumber);
    
    std::cout << "原始大整数: " << bigNumber << std::endl;
    std::cout << "JSON序列化: " << bigJson.serialize() << std::endl;
    std::cout << "是否为整数: " << (bigJson.isInteger() ? "是" : "否") << std::endl;
    std::cout << "恢复的值: " << bigJson.toLongLong() << std::endl;
    std::cout << "精度保持: " << (bigJson.toLongLong() == bigNumber ? "是" : "否") << std::endl;
    
    // 特殊数值处理
    JsonValue::ParseOptions options;
    options.allowSpecialNumbers = true;
    
    JsonValue nanValue = JsonValue::parse("NaN", options);
    JsonValue infValue = JsonValue::parse("Infinity", options);
    
    std::cout << "\n特殊数值测试:" << std::endl;
    std::cout << "NaN JSON: " << nanValue.serialize() << std::endl;
    std::cout << "Infinity JSON: " << infValue.serialize() << std::endl;
    
    return 0;
}
```

### 5. 错误处理示例

```cpp
#include "jsonstruct_std.h"
#include <iostream>

using namespace JsonStruct;

int main() {
    // 解析错误处理
    try {
        JsonValue invalid = JsonValue::parse("{invalid json}");
    } catch (const JsonParseException& e) {
        std::cout << "解析错误: " << e.what() << std::endl;
        std::cout << "位置: " << e.locationInfo() << std::endl;
    }
    
    // 类型错误处理
    JsonValue stringValue("hello");
    try {
        int number = stringValue.toInt(); // 会抛出异常
    } catch (const JsonTypeException& e) {
        std::cout << "类型错误: " << e.what() << std::endl;
    }
    
    // 安全的类型获取
    auto maybeNumber = stringValue.getInt();
    if (maybeNumber) {
        std::cout << "数字: " << *maybeNumber << std::endl;
    } else {
        std::cout << "不是数字类型" << std::endl;
    }
    
    return 0;
}
```

## 编译和运行

### 使用CMake

```bash
# 配置项目
cmake -B build -S .

# 编译
cmake --build build --config Release

# 运行示例（如果添加了上述示例到项目中）
./build/Release/basic_example
./build/Release/type_registration_example
./build/Release/stl_container_example
```

### 手动编译

```bash
# 确保包含必要的头文件路径
g++ -std=c++17 -I./src -I./src/std example.cpp ./src/json_value.cpp ./src/json_path.cpp -o example

# 运行
./example
```

## 更多信息

- [API参考手册](API_REFERENCE.md) - 完整的API文档
- [用户指南](USER_GUIDE.md) - 详细的使用说明
- [高级功能](ADVANCED_FEATURES.md) - JSONPath、流式解析等高级特性
- [性能指南](PERFORMANCE_GUIDE.md) - 性能优化建议
