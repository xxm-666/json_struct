# JsonStruct - 现代 C++ JSON 处理库

---

## 🚦 内置增强测试框架简介

JsonStruct 项目集成了现代化的 C++ 单元测试框架，支持丰富断言、标签、过滤、自动计时和详细报告，兼容所有现有测试代码。

### 主要特性
- 丰富断言类型（字符串、容器、异常、指针、数值等）
- 测试标签与过滤，灵活管理测试集
- 跳过测试与动态跳过支持
- 自动计时与详细报告
- 完全向后兼容

### 快速示例
```cpp
#include "test_framework/test_framework.h"

TEST(BasicTest) {
    ASSERT_TRUE(true);
    ASSERT_EQ(42, 42);
}

TEST_WITH_TAGS(TagTest, "unit", "fast") {
    ASSERT_STREQ("hello", "hello");
}

TEST_SKIP(SkipTest, "暂时跳过")

int main() {
    TestFramework::TestConfig config;
    config.verbose = true;
    config.timing = true;
    config.includeTags = {"unit"};
    SET_TEST_CONFIG(config);
    return RUN_ALL_TESTS();
}
```

更多用法详见 `test_framework/ENHANCED_FRAMEWORK_DOCUMENTATION.md`。

---

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

// 增强型懒查询（大数据高效处理）
JsonFilter filter = JsonFilter::createDefault();
auto generator = EnhancedQueryFactory::createGenerator(filter, data, 
    "$.companies[*].employees[?(@.salary > 50000)]");

while (generator.hasNext()) {
    auto result = generator.next();
    processEmployee(*result.value);  // 内存高效的延迟求值
}

// 缓存优化查询
generator.enableCache(true);
// 重复查询时自动使用缓存，命中率可达50-70%
```

### ⚡ 增强型懒查询生成器

全新的 `EnhancedLazyQueryGenerator` 提供了强大的懒加载查询能力：

- **完整JSONPath支持**: 支持所有标准JSONPath语法
- **智能缓存系统**: 自动缓存查询结果，提升重复查询性能
- **内存高效**: 延迟求值，只在需要时生成结果
- **性能监控**: 内置性能统计和效率追踪
- **早停优化**: 满足条件时提前终止查询

```cpp
// 创建增强型查询生成器
auto gen = EnhancedQueryFactory::createGenerator(filter, data,
    "$..products[?(@.price > 100 && @.category == 'electronics')]");

// 启用缓存
gen.enableCache(true);

// 处理结果
std::vector<JsonValue> results;
while (gen.hasNext() && results.size() < 100) {
    results.push_back(*gen.next().value);
}

// 查看性能统计
std::cout << "效率比: " << gen.getEfficiencyRatio() << "%\n";
std::cout << "缓存命中率: " << gen.getCacheHitRatio() << "%\n";
std::cout << "处理帧数: " << gen.getFramesProcessed() << "\n";

// 重置并重用（保留缓存）
gen.reset();
// 第二次查询会利用缓存，显著提升性能
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

JsonStruct 提供两种查询生成器以满足不同需求：

#### 基础懒查询生成器
- **轻量级**: 适合简单查询和小型数据集
- **零依赖**: 无额外内存开销
- **快速**: 直接查询，无缓存开销

#### 增强型懒查询生成器
- **功能完整**: 支持所有JSONPath语法（过滤器、切片、递归等）
- **智能缓存**: 路径级别缓存，LRU式管理
- **性能监控**: 详细的执行统计和效率分析
- **内存高效**: 延迟求值，分帧处理

```cpp
// 简单查询推荐使用基础生成器
auto basicGen = data.createLazyQuery("$.users[*].name");

// 复杂查询推荐使用增强型生成器
JsonFilter filter = JsonFilter::createDefault();
auto enhancedGen = EnhancedQueryFactory::createGenerator(filter, data,
    "$.companies[*].departments[?(@.budget > 100000)].employees[?(@.active)]");

enhancedGen.enableCache(true);  // 启用智能缓存
```

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

| 功能       | 语法示例                        | 基础生成器 | 增强型生成器 |
| ---------- | ------------------------------- | ---------- | ------------ |
| 根节点     | `$`                           | ✅ 支持    | ✅ 支持      |
| 属性访问   | `$.prop`                      | ✅ 支持    | ✅ 支持      |
| 嵌套属性   | `$.prop.subprop`              | ✅ 支持    | ✅ 支持      |
| 数组索引   | `$.arr[0]`                    | ✅ 支持    | ✅ 支持      |
| 数组切片   | `$.arr[1:3]`, `$.arr[::2]`    | ❌ 不支持  | ✅ 完全支持  |
| 通配符     | `$.*`, `$.arr[*]`           | ✅ 支持    | ✅ 支持      |
| 递归下降   | `$..prop`                     | ✅ 支持    | ✅ 支持      |
| 带空格属性 | `$['prop name']`              | ✅ 支持    | ✅ 支持      |
| 过滤器     | `$.arr[?(@.prop == 'value')]` | ❌ 不支持  | ✅ 完全支持  |
| Union操作  | `$.path1,$.path2`             | ❌ 不支持  | ✅ 完全支持  |
| 负索引     | `$.arr[-1]`                   | ❌ 不支持  | ✅ 支持      |

### 增强型查询生成器特性

#### 智能缓存系统
```cpp
auto gen = EnhancedQueryFactory::createGenerator(filter, data, query);
gen.enableCache(true);

// 首次查询建立缓存
processResults(gen);

// 重置并重新查询，利用缓存提升性能
gen.reset();
processResults(gen);  // 缓存命中率: 50-70%

std::cout << "缓存大小: " << gen.getCacheSize() << " 条目\n";
std::cout << "命中率: " << gen.getCacheHitRatio() << "%\n";
```

#### 性能监控和统计
```cpp
// 查询执行统计
std::cout << "处理帧数: " << gen.getFramesProcessed() << "\n";
std::cout << "生成结果: " << gen.getResultsGenerated() << "\n";
std::cout << "效率比: " << gen.getEfficiencyRatio() << "%\n";

// 检查生成器状态
if (gen.hasNext()) {
    std::cout << "还有更多结果\n";
}
```

#### 复杂查询示例
```cpp
// 多级过滤和切片组合
auto gen1 = EnhancedQueryFactory::createGenerator(filter, data,
    "$.companies[?(@.founded > 2000)].departments[0:3].employees[?(@.salary > 50000)]");

// 递归搜索与过滤器
auto gen2 = EnhancedQueryFactory::createGenerator(filter, data,
    "$..products[?(@.price > 100 && @.category == 'electronics')]");

// Union操作
auto gen3 = EnhancedQueryFactory::createGenerator(filter, data,
    "$.user.name,$.user.email,$.user.phone");
```

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
- [X] **增强型懒查询生成器** - 智能缓存、性能监控、内存优化
- [X] **基础懒查询生成器** - 轻量级快速查询
- [X] **流式查询处理** - 内存高效的大数据处理
- [X] **版本管理系统** - 运行时版本检查和兼容性验证
- [X] **性能基准测试** - 独立的性能测试套件
- [X] **完整测试覆盖** - 200+ 测试用例，全面验证功能

### 🚧 开发中功能

- [ ] **增强型查询优化器** - 查询计划优化和重写
- [ ] **异步查询支持** - 非阻塞查询处理
- [ ] **更多Union语法** - 数组索引Union (`[0,2,4]`) 和多属性Union (`['a','b']`)
- [ ] **线程安全优化** - 并发查询缓存同步保护
- [ ] **JSON Patch 支持** - RFC 6902/7396 补丁操作
- [ ] **Schema 验证** - JSON Schema 输入校验
- [ ] **管道操作** - 链式 map/filter/reduce 接口
- [ ] **错误报告增强** - 详细错误位置和调试信息

### 📈 性能数据

#### 查询性能对比

| 查询类型 | 基础生成器 | 增强型生成器 | 增强型+缓存 |
|---------|-----------|-------------|------------|
| 简单属性访问 | ~65μs | ~89μs | ~45μs (缓存命中) |
| 数组切片 | ~79μs | ~125μs | ~60μs (缓存命中) |
| 递归搜索 | ~975μs | ~5067μs | ~2500μs (缓存命中) |
| 复杂过滤器 | 不支持 | ~2131μs | ~1000μs (缓存命中) |

#### 缓存效果

- **首次查询**: 建立缓存，性能略低于基础版本
- **重复查询**: 缓存命中率 50-70%，性能提升 40-60%
- **复杂查询**: 缓存效果更明显，性能提升高达 100%

#### 内存效率

- **解析速度**: 与主流 JSON 库性能相当
- **内存使用**: 流式处理下内存使用量与嵌套深度线性相关
- **缓存开销**: 平均每个缓存条目 < 100 字节
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
