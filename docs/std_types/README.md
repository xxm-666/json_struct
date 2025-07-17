
# Std Types 模块

## 功能概述
- 提供对标准 C++ STL 容器类型的自动序列化和反序列化支持。
- 支持常见的顺序容器、集合容器、映射容器、pair 等。

## 支持的类型一览

| 类型类别     | 支持的类型                                                         |
| ------------ | ------------------------------------------------------------------- |
| 顺序容器     | `std::vector`, `std::list`, `std::deque`                            |
| 集合容器     | `std::set`, `std::unordered_set`                                    |
| 映射容器     | `std::map`, `std::unordered_map`                                    |
| 元组类型     | `std::pair`                                                         |

## 序列化与反序列化规则

### 顺序/集合容器
- 序列化为 JSON 数组。例如：
  ```json
  [1, 2, 3]
  ```

### 映射容器（map/unordered_map）
- 如果 key 类型为 `std::string`，序列化为 JSON 对象：
  ```json
  {"a": 1, "b": 2}
  ```
- 其它 key 类型，序列化为 `[[key, value], ...]` 的数组：
  ```json
  [[1, "one"], [2, "two"]]
  ```

### pair
- 序列化为 `[first, second]` 数组：
  ```json
  ["foo", 42]
  ```

## 使用示例

### 序列化标准容器
```cpp
#include "std_types/std_registry.h"
using namespace JsonStruct;

std::vector<int> vec = {1, 2, 3};
JsonValue json = TypeRegistry::instance().toJson(vec);
std::cout << json.dump(2) << std::endl;
```

### 反序列化标准容器
```cpp
std::string jsonStr = "[10, 20, 30]";
std::vector<int> vec = TypeRegistry::instance().fromJson<std::vector<int>>(JsonValue::parse(jsonStr));
```

### 映射容器序列化
```cpp
std::map<std::string, double> settings = {{"alpha", 1.0}, {"beta", 2.5}};
JsonValue json = TypeRegistry::instance().toJson(settings);
// 输出: {"alpha":1.0,"beta":2.5}
```

### pair 序列化
```cpp
std::pair<std::string, int> p = {"foo", 42};
JsonValue json = TypeRegistry::instance().toJson(p);
// 输出: ["foo", 42]
```

## 注意事项
- 反序列化时，类型必须与目标 STL 容器元素类型兼容，否则会使用默认值。
- 非 string-key 的 map/unordered_map 只能以数组形式序列化/反序列化。
- 支持嵌套容器和嵌套 pair/map/vector 等任意组合。
