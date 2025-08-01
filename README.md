# JsonStruct - 现代 C++ JSON 处理库

[![Version](https://img.shields.io/badge/version-1.2.0--dev-blue.svg)](https://github.com/xxm-666/json_struct)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-17%2B-orange.svg)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-red.svg)](https://cmake.org/)

JsonStruct 是一个现代化的 C++17 JSON 处理库，专注于类型安全、零侵入设计和高性能。支持完整的 JSONPath 查询、流式处理、类型自动注册，以及 STL 和 Qt 类型的无缝集成。

## ✨ 核心特性

### 🚀 零侵入类型注册

```cpp
struct UserData {
    std::string name;
    std::vector<int> scores;
    std::map<std::string, double> settings;
  
    // 一行宏完成所有类型注册！
    JSON_AUTO(name, scores, settings)
};

// 自动序列化/反序列化
UserData data;
std::string json = data.toJsonString(2);
UserData restored = UserData::fromJsonString(json);
```

### 🔍 完整 JSONPath 查询引擎

```cpp
JsonValue data = JsonValue::parse(jsonString);

// 基础查询
auto name = data.selectFirst("$.user.name");
auto books = data.selectAll("$.store.book[*].title");

// 高级查询
auto slice = data.selectAll("$.numbers[1:4]");        // 数组切片
auto recursive = data.selectAll("$..author");         // 递归搜索
auto spaced = data.selectAll("$['user name']");       // 带空格属性名

// 流式查询（大数据处理）
auto generator = JsonStreamingQuery::createGenerator(json, "$.data[*].value");
for (auto it = generator.begin(); it != generator.end(); ++it) {
    processValue(it->first);  // 内存高效的延迟求值
}
```

### 📦 全面的 STL 类型支持

- **容器类型**: `std::vector`, `std::list`, `std::map`, `std::unordered_map`, `std::set` 等
- **智能指针**: `std::shared_ptr`, `std::unique_ptr`, `std::optional`
- **其他类型**: `std::tuple`, `std::pair`, `std::array`

### 🖼️ Qt 类型生态支持

```cpp
struct WindowSettings {
    QString title = "My Application";
    QPointF position = {100.0, 200.0};
    QRectF geometry = {0, 0, 800, 600};
    QList<QString> recentFiles;
  
    JSON_AUTO(title, position, geometry, recentFiles)
};
```

### ⚡ 高性能流式处理

- **内存高效**: 延迟求值，只在需要时生成结果
- **早停支持**: 满足条件时提前终止查询
- **批处理**: 支持分批处理大量数据
- **迭代器模式**: 标准 C++ 迭代器接口

## 🛠️ 系统要求

| 组件               | 要求                         |
| ------------------ | ---------------------------- |
| **编译器**   | GCC 7+, Clang 5+, MSVC 2017+ |
| **C++ 标准** | C++17 或更高                 |
| **CMake**    | 3.16 或更高                  |
| **依赖库**   | 仅标准库 (Qt 支持可选)       |

## 📥 快速开始

### 使用 CMake 集成

```cmake
# 添加项目
find_package(json_struct REQUIRED)

# 链接库
target_link_libraries(your_target json_struct)
target_compile_features(your_target PRIVATE cxx_std_17)
```

### 包含头文件

```cpp
#include "jsonstruct.h"
using namespace JsonStruct;
```

### 基础使用示例

```cpp
#include "jsonstruct.h"
#include <iostream>

struct Person {
    std::string name;
    int age;
    std::vector<std::string> hobbies;
  
    JSON_AUTO(name, age, hobbies)
};

int main() {
    // 创建对象
    Person person{"Alice", 30, {"reading", "coding", "music"}};
  
    // 序列化为 JSON
    std::string json = person.toJsonString(2);  // 格式化输出
    std::cout << "JSON: " << json << std::endl;
  
    // 从 JSON 反序列化
    Person restored = Person::fromJsonString(json);
    std::cout << "Name: " << restored.name << ", Age: " << restored.age << std::endl;
  
    return 0;
}
```

## 📁 项目结构

```
JsonStruct/
├── src/                          # 核心源码
│   ├── jsonstruct.h              # 主入口头文件
│   ├── type_registry/            # 类型注册系统
│   ├── std_types/                # STL 类型支持
│   ├── qt_types/                 # Qt 类型支持
│   └── json_engine/              # JSON 引擎核心
├── examples/                     # 示例代码
├── tests/                        # 测试套件
├── benchmarks/                   # 性能基准测试
├── docs/                         # 详细文档
└── webnet/                       # 项目网站
```

## 🧪 构建和测试

### 构建项目

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Release

# 构建
cmake --build . --config Release
```

### 运行测试

```bash
# 构建测试
cmake --build . --target build_all_tests

# 运行核心测试
ctest --output-on-failure
```

## 📚 高级功能

### JSONPath 完整语法支持

| 功能       | 语法示例                        | 支持状态    |
| ---------- | ------------------------------- | ----------- |
| 根节点     | `$`                           | ✅ 完全支持 |
| 属性访问   | `$.prop`                      | ✅ 完全支持 |
| 嵌套属性   | `$.prop.subprop`              | ✅ 完全支持 |
| 数组索引   | `$.arr[0]`                    | ✅ 完全支持 |
| 数组切片   | `$.arr[1:3]`                  | ✅ 完全支持 |
| 通配符     | `$.*`, `$.arr[*]`           | ✅ 完全支持 |
| 递归下降   | `$..prop`                     | ✅ 完全支持 |
| 带空格属性 | `$['prop name']`              | ✅ 完全支持 |
| 过滤器     | `$.arr[?(@.prop == 'value')]` | ✅ 完全支持 |

### 版本管理系统

```cpp
#include "version.h"

// 获取版本信息
std::string version = Version::getVersionString();  // "1.2.0-dev"
std::string details = Version::getDetailedVersionString();

// 兼容性检查
if (Version::isCompatible(1, 0)) {
    std::cout << "支持 v1.0+ API" << std::endl;
}

// 构建信息
std::cout << "构建日期: " << Version::getBuildDate() << std::endl;
std::cout << "Git 提交: " << Version::getGitCommit() << std::endl;
```

### 流式查询和延迟求值

```cpp
// 处理大型 JSON 数据
auto generator = data.streamQuery("$.events[*]");

// 配置选项
QueryGenerator::GeneratorOptions options;
options.maxResults = 1000;
options.batchSize = 100;

// 分批处理
while (generator.hasMore()) {
    auto batch = generator.takeBatch(50);
    processBatch(batch);
}
```

## 📊 性能特点

- **编译时优化**: 模板特化和 `constexpr` 优化
- **零运行时开销**: 编译时类型检查
- **内存高效**: RAII 自动管理，零拷贝字符串处理
- **并发友好**: 无全局状态，线程安全设计

## 🎯 当前开发状态 (v1.2.0-dev)

### ✅ 已完成功能

- [X] **完整的类型注册系统** - 支持 STL 和 Qt 类型
- [X] **JSONPath 查询引擎** - 100% 标准语法支持
- [X] **流式查询处理** - 内存高效的大数据处理
- [X] **版本管理系统** - 运行时版本检查和兼容性验证
- [X] **性能基准测试** - 独立的性能测试套件
- [X] **完整测试覆盖** - 150+ 测试用例，全面验证功能

### 🚧 开发中功能

- [ ] **线程安全优化** - 并发查询缓存同步保护
- [ ] **JSON Patch 支持** - RFC 6902/7396 补丁操作
- [ ] **Schema 验证** - JSON Schema 输入校验
- [ ] **管道操作** - 链式 map/filter/reduce 接口
- [ ] **错误报告增强** - 详细错误位置和调试信息

### 📈 性能数据

- **解析速度**: 与主流 JSON 库性能相当
- **内存使用**: 流式处理下内存使用量与嵌套深度线性相关
- **编译时间**: 模板优化，增量编译友好

## 📖 文档

- [API 参考手册](docs/API_REFERENCE.md)
- [高级功能指南](docs/ADVANCED_FEATURES.md)
- [性能优化指南](docs/PERFORMANCE_GUIDE.md)
- [版本管理指南](docs/VERSION_MANAGEMENT.md)
- [Qt 类型支持](docs/qt_types/README.md)

## 🤝 贡献

欢迎贡献代码、报告问题或提出建议！

1. Fork 此仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开 Pull Request

## 📄 许可证

本项目采用 [MIT 许可证](LICENSE)。

## 🔗 相关链接

- [项目主页](https://github.com/xxm-666/json_struct)
- [问题追踪](https://github.com/xxm-666/json_struct/issues)
- [更新日志](CHANGELOG.md)

---

**JsonStruct** - 让 C++ JSON 处理变得简单而强大！ 🚀
