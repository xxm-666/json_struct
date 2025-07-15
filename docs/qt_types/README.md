# Qt Types 模块

## 功能概述
- 提供对 Qt 类型的支持，包括序列化和反序列化。
- 支持常见的 Qt 容器类型。

## 使用示例

### 序列化 Qt 类型
```cpp
#include "qt_types/qt_registry.h"
using namespace JsonStruct;

QMap<QString, int> map;
map["one"] = 1;
map["two"] = 2;
JsonValue json = TypeRegistry::instance().toJson(map);
std::cout << json.serialize(2) << std::endl;
```
