# 🚀 类型注册系统 - 5分钟快速开始

> 学会使用JsonStruct 的类型注册系统，让C++对象序列化变得简单自动！

## 🎯 核心概念

**JsonStruct ** 的核心思想很简单：

1. **一行宏** `JSON_FIELDS(...)` 完成类型注册
2. **自动生成** `toJsonString()` 和 `fromJsonString()` 方法  
3. **零侵入** 无需修改现有代码结构
4. **类型安全** 编译时类型检查

## ⚡ 30秒入门示例

```cpp
#include "jsonstruct.h"
using namespace JsonStruct;

// 1️⃣ 定义数据结构 + 一行宏
struct User {
    std::string name = "Alice";
    int age = 25;
    std::vector<std::string> hobbies = {"reading", "coding"};
    
    JSON_FIELDS(name, age, hobbies)  // 🎯 就是这一行！
};

int main() {
    User user;
    
    // 2️⃣ 自动序列化
    std::string json = user.toJsonString(2);
    std::cout << json << std::endl;
    
    // 3️⃣ 自动反序列化  
    User restored = User::fromJsonString(json);
    std::cout << "Name: " << restored.name << std::endl;
    
    return 0;
}
```

**输出结果**：
```json
{
  "name": "Alice",
  "age": 25,
  "hobbies": [
    "reading", 
    "coding"
  ]
}
Name: Alice
```

## 🏗️ 项目设置

### 包含头文件
```cpp
// 方式1: 全功能入口 (推荐)
#include "jsonstruct.h"

// 方式2: 按需包含
#include "type_registry/field_macros.h"      // 核心宏
#include "std_types/std_registry.h"          // STL类型支持
#include "qt_types/qt_registry.h"            // Qt类型支持 (可选)
```

### CMake配置
```cmake
# 添加包含路径
include_directories(path/to/jsonstruct_registry/src)

# 链接库 (如果需要)
target_link_libraries(your_target jsonstruct_core)
```

## 📦 STL类型自动支持

**所有常用STL类型都自动支持**，无需额外配置：

```cpp
struct DataContainer {
    // 基础类型
    std::string title;
    int count;
    double ratio;
    bool enabled;
    
    // 容器类型  
    std::vector<int> numbers;
    std::list<std::string> names;
    std::map<std::string, double> settings;
    std::unordered_set<std::string> tags;
    
    // 智能指针
    std::shared_ptr<std::vector<int>> shared_data;
    std::unique_ptr<std::string> unique_name;
    std::optional<int> maybe_value;
    
    // 元组类型
    std::pair<std::string, int> key_value;
    std::tuple<std::string, int, double> triple;
    
    JSON_FIELDS(title, count, ratio, enabled, 
                numbers, names, settings, tags,
                shared_data, unique_name, maybe_value,
                key_value, triple)
};

// 使用完全相同！
DataContainer data;
std::string json = data.toJsonString();
```

## 🖼️ Qt类型集成 (可选)

如果使用Qt，Qt类型也自动支持：

```cpp
#include "qt_types/qt_registry.h"

struct WindowConfig {
    QString title = "My Window";
    QPointF position = {100.0, 200.0};
    QRectF geometry = {0, 0, 800, 600};
    QColor backgroundColor = QColor(255, 255, 255);
    QStringList recentFiles = {"file1.txt", "file2.txt"};
    
    JSON_FIELDS(title, position, geometry, backgroundColor, recentFiles)
};

// Qt类型也是一行搞定！
WindowConfig config;
QString json = QString::fromStdString(config.toJsonString(2));
```

## 🔗 嵌套对象支持

**复杂嵌套对象自动处理**：

```cpp
struct Address {
    std::string street;
    std::string city;
    int zipCode;
    JSON_FIELDS(street, city, zipCode)
};

struct Company {
    std::string name;
    Address headquarters;  // 嵌套对象
    JSON_FIELDS(name, headquarters)
};

struct Employee {
    std::string name;
    Company employer;      // 多层嵌套
    std::vector<Address> addresses;  // 对象数组
    JSON_FIELDS(name, employer, addresses)
};

// 无论多复杂的嵌套，使用方法都一样！
Employee emp;
std::string json = emp.toJsonString();
Employee restored = Employee::fromJsonString(json);
```

## 🎨 自定义序列化行为

### 默认值处理
```cpp
struct Config {
    std::string name = "default";  // 设置默认值
    int timeout = 30;
    bool debug = false;
    
    JSON_FIELDS(name, timeout, debug)
};

// 反序列化时，缺失字段将使用默认值
```

### 可选字段
```cpp
struct UserProfile {
    std::string username;              // 必需字段
    std::optional<std::string> email;  // 可选字段
    std::optional<int> age;           // 可选字段
    
    JSON_FIELDS(username, email, age)
};
```


## ⚙️ 高级特性预览

### 自定义类型注册
```cpp
// 为自定义类型注册序列化器
TypeRegistry::registerType<MyCustomType>(
    [](const MyCustomType& obj) { /* 序列化逻辑 */ },
    [](const Json& json) { /* 反序列化逻辑 */ }
);
```

### 错误处理
```cpp
try {
    User user = User::fromJsonString(invalidJson);
} catch (const JsonParseException& e) {
    std::cout << "解析失败: " << e.what() << std::endl;
}
```

**核心记住一点**: `JSON_FIELDS()` 宏是一切的开始！
