# JSON Engine 模块

## 功能概述
- 提供 JSON 值的解析、序列化和查询功能。
- 支持 JSONPath 查询语言。
- 包含流式解析器，适用于大文件处理。

## 使用示例

### 解析 JSON 字符串
```cpp
#include "json_engine/json_value.h"
using namespace JsonStruct;

std::string jsonText = R"({"name": "Alice", "age": 30})";
JsonValue parsed = JsonValue::parse(jsonText);
std::cout << "Name: " << parsed["name"].toString() << std::endl;
```

### 使用 JSONPath 查询
```cpp
JsonValue data = JsonValue::parse(R"({"store": {"book": [{"title": "C++"}]}})");
const JsonValue* title = data.selectFirst("$.store.book[0].title");
std::cout << "Title: " << title->toString() << std::endl;
```
