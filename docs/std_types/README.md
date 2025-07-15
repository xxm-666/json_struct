# Std Types 模块

## 功能概述
- 提供对标准容器类型的支持，包括序列化和反序列化。
- 支持常见的 STL 容器类型，如`std::vector`、`std::map`等。

## 使用示例

### 序列化标准容器
```cpp
#include "std_types/std_registry.h"
using namespace JsonStruct;

std::vector<int> vec = {1, 2, 3};
JsonValue json = TypeRegistry::instance().toJson(vec);
std::cout << json.serialize(2) << std::endl;
```
