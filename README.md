# JsonStruct - C++17+ JSON序列化与JSONPath查询框架

> 现代C++17+JSON处理框架 - 提供类型注册、自动序列化、JSONPath查询和版本管理的完整解决方案

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![C++17](https://img.shields.io/badge/C%2B%2B-17%2B-blue)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()
[![Version](https://img.shields.io/badge/version-1.2.0--dev-orange)]()

## 🌟 主要特性

### 🔧 版本管理系统
- **语义化版本控制**: 遵循 [Semantic Versioning](https://semver.org/) 规范
- **API兼容性检查**: 运行时版本兼容性验证
- **构建信息集成**: 自动注入Git提交、分支、构建时间
- **命令行工具**: 独立的版本查询和兼容性检查工具

### 🚀 JSONPath查询引擎
- **完整JSONPath语法**: 支持标准JSONPath表达式
- **带空格属性名**: 支持 `['property name']` 语法查询包含空格的属性
- **过滤器表达式**: 高级过滤查询 `$.array[?(@.price < 10)]`
- **可变操作**: 支持查询结果的就地修改
- **递归查询**: 深度搜索 `$..property` 语法

### 📦 类型注册与序列化
- **零侵入设计**: 使用 `JSON_FIELDS()` 宏一行完成注册
- **STL类型支持**: 完整的标准库容器和智能指针支持
- **Qt类型集成**: 原生支持Qt几何、图形、容器类型
- **自定义类型**: 灵活的自定义类型注册机制

### 🎯 现代C++特性
- **编译时类型安全**: 模板和constexpr优化
- **零拷贝优化**: 移动语义和完美转发
- **异常安全**: RAII资源管理和异常安全保证

## 📁 项目结构

```
jsonstruct_registry/
├── src/                           # 核心源码
│   ├── jsonstruct.h               # 🌟 统一入口头文件
│   ├── version.h                  # 🆕 版本管理系统
│   ├── version.cpp                # 版本管理实现
│   ├── type_registry/             # 🥇 类型注册系统核心
│   │   ├── registry_core.h        # 类型注册表核心
│   │   ├── auto_serializer.h      # 自动序列化引擎
│   │   └── field_macros.h         # JSON_FIELDS宏定义
│   ├── std_types/                 # 🥈 STL类型注册
│   │   └── std_registry.h         # STL类型注册主头文件
│   ├── qt_types/                  # 🖼️ Qt类型注册
│   │   ├── qt_registry.h          # 基础Qt类型注册
│   │   ├── qt_ultimate_registry.h # 高级Qt类型注册
│   │   └── qt_common.h            # Qt公共类型注册
│   └── json_engine/               # 📦 JSON处理引擎
│       ├── json_value.h           # JSON值表示
│       ├── json_value.cpp         # JSON核心实现
│       ├── json_number.h          # 数值精度处理
│       ├── json_stream_parser.h   # 流式解析器
│       ├── json_path.h            # 🚀 JSONPath查询引擎
│       ├── json_path.cpp          # JSONPath实现
│       ├── json_filter.h          # JSONPath过滤器
│       └── json_query_generator.h # 查询生成器
├── tests/                         # 🧪 测试套件
│   ├── test_version.cpp           # � 版本管理测试
│   ├── test_spaced_properties.cpp # 🆕 带空格属性名测试
│   ├── test_json_path_mutable.cpp # 🆕 可变JSONPath测试
│   ├── test_comprehensive_spaced.cpp # 综合空格属性测试
│   ├── test_core_functionality.cpp # 核心功能测试
│   ├── test_json_parsing.cpp      # JSON解析测试
│   ├── test_json_filter_basic.cpp # 过滤器测试
│   └── ...                       # 更多测试文件
├── examples/                      # 📋 示例程序
│   ├── version_demo.cpp           # 🆕 版本管理演示
│   ├── example_enhanced_simple.cpp # 基础功能演示
│   ├── example_jsonpath_complete.cpp # JSONPath完整演示
│   ├── example_json_filter.cpp    # 过滤器演示
│   └── streaming_query_example.cpp # 流式查询演示
├── docs/                          # 📚 分层文档
│   ├── VERSION_MANAGEMENT.md      # 🆕 版本管理指南
│   ├── USER_GUIDE.md              # 用户指南
│   ├── API_REFERENCE.md           # API参考
│   ├── ADVANCED_FEATURES.md       # 高级特性
│   └── PERFORMANCE_GUIDE.md       # 性能指南
├── tools/                         # 🛠️ 工具程序
│   └── version_info               # 版本信息命令行工具
├── CMakeLists.txt                 # CMake构建配置
├── CHANGELOG.md                   # 🆕 版本变更日志
└── README.md                      # 项目说明
```
## 🚀 快速开始

### ✨ 基础序列化示例

```cpp
#include "jsonstruct.h"
using namespace JsonStruct;

// 定义数据结构
struct UserConfig {
    std::string name = "Alice";
    std::vector<int> scores = {95, 87, 92};
    std::map<std::string, double> settings = {{"volume", 0.8}, {"brightness", 0.6}};
    bool enabled = true;
    
    // 一行宏完成类型注册
    JSON_FIELDS(name, scores, settings, enabled)
};

int main() {
    UserConfig config;
    
    // 自动序列化到JSON
    std::string jsonStr = config.toJsonString(2);
    std::cout << jsonStr << std::endl;
    /*
    输出:
    {
      "name": "Alice",
      "scores": [95, 87, 92],
      "settings": {
        "volume": 0.8,
        "brightness": 0.6
      },
      "enabled": true
    }
    */
    
    // 自动从JSON反序列化
    UserConfig restored = UserConfig::fromJsonString(jsonStr);
    std::cout << "Name: " << restored.name << std::endl;
    
    return 0;
}
```

### � JSONPath查询示例

```cpp
#include "jsonstruct.h"
using namespace JsonStruct;

int main() {
    // 创建测试数据
    JsonValue data = JsonValue::fromString(R"({
        "users": [
            {"name": "Alice", "age": 30, "test score": 95},
            {"name": "Bob", "age": 25, "test score": 87},
            {"name": "Charlie", "age": 35, "test score": 92}
        ]
    })");
    
    // 基础查询
    auto names = jsonvalue_jsonpath::selectAll(data, "$.users[*].name");
    for (const auto& name : names) {
        std::cout << "User: " << name.get().toString() << std::endl;
    }
    
    // 带空格的属性名查询
    auto scores = jsonvalue_jsonpath::selectAll(data, "$.users[*]['test score']");
    for (const auto& score : scores) {
        std::cout << "Score: " << score.get().toInt() << std::endl;
    }
    
    // 过滤器查询
    auto adults = jsonvalue_jsonpath::selectAll(data, "$.users[?(@.age >= 30)]");
    std::cout << "Found " << adults.size() << " adult users" << std::endl;
    
    // 可变操作
    auto mutableUsers = jsonvalue_jsonpath::selectAllMutable(data, "$.users[*]");
    for (auto& user : mutableUsers) {
        user.get()["active"] = true;  // 为每个用户添加active字段
    }
    
    return 0;
}
```

### 🆕 版本管理示例

```cpp
#include "version.h"
using namespace JsonStruct;

int main() {
    // 获取版本信息
    std::cout << "Library Version: " << Version::getVersionString() << std::endl;
    std::cout << "Build Info: " << Version::getDetailedVersionString() << std::endl;
    
    // 检查API兼容性
    if (Version::isCompatible(1, 0)) {
        std::cout << "Supports v1.0+ API" << std::endl;
    }
    
    // 获取构建元数据
    std::cout << "Built on: " << Version::getBuildDate() << std::endl;
    std::cout << "Git Commit: " << Version::getGitCommit() << std::endl;
    
    // JSON格式输出
    std::cout << Version::toJson() << std::endl;
    
    return 0;
}
```

### 📦 项目构建

```bash
# 克隆项目
git clone https://github.com/xxm-666/json_struct.git
cd json_struct

# 配置CMake (启用示例构建)
cmake -B build -S . -DBUILD_EXAMPLES=ON -DBUILD_TESTS=ON

# 构建所有目标
cmake --build build --config Release

# 运行核心测试
./build/Release/test_core_functionality     # 核心功能测试
./build/Release/test_version                # 版本管理测试
./build/Release/test_spaced_properties      # 带空格属性测试

# 运行示例程序
./build/Release/example_enhanced_simple     # 基础功能演示
./build/Release/version_demo                # 版本管理演示
./build/Release/example_jsonpath_complete   # JSONPath演示

# 使用版本信息工具
./build/Release/version_info --version      # 显示版本
./build/Release/version_info --detailed     # 详细版本信息
./build/Release/version_info --json         # JSON格式输出
```

## 🏆 核心特性详解

### � 版本管理系统
- **语义化版本控制**: 遵循 [Semantic Versioning](https://semver.org/) 的 MAJOR.MINOR.PATCH 格式
- **API兼容性检查**: 运行时检查版本兼容性，避免API不匹配问题
- **构建信息集成**: 自动集成Git提交哈希、分支、构建时间等元数据
- **命令行工具**: 独立的版本查询工具，支持JSON格式输出
- **CMake集成**: 构建时自动生成版本信息头文件

**版本管理API:**
```cpp
// 版本信息查询
std::string version = Version::getVersionString();          // "1.2.0-dev"
std::string detailed = Version::getDetailedVersionString(); // 包含构建信息
auto [major, minor, patch] = Version::getVersionTuple();    // 版本元组

// 兼容性检查
bool compatible = Version::isCompatible(1, 0);   // 检查是否支持v1.0+ API
int comparison = Version::compareVersion(1, 2, 0); // 版本比较

// 构建信息
std::string buildDate = Version::getBuildDate();  // 构建日期
std::string gitCommit = Version::getGitCommit();  // Git提交哈希
std::string gitBranch = Version::getGitBranch();  // Git分支
bool isRelease = Version::isReleaseVersion();     // 是否发布版本
```

### 🚀 JSONPath查询引擎
- **完整语法支持**: 实现标准JSONPath规范的核心功能
- **带空格属性名**: 支持 `['property name']` 和 `["property name"]` 语法
- **过滤器表达式**: 强大的条件过滤 `$.array[?(@.field operator value)]`
- **递归查询**: 深度搜索 `$..property` 语法，遍历所有层级
- **可变操作**: 支持查询结果的就地修改和更新

**JSONPath语法示例:**
```cpp
// 基础路径查询
"$.store.book[0].title"           // 第一本书的标题
"$.store.book[*].author"          // 所有书的作者
"$.store.book[-1]"                // 最后一本书

// 带空格属性名
"$.data['user info']['first name']"  // 包含空格的属性
"$['complex property']['nested']"    // 多层空格属性

// 过滤器表达式  
"$.store.book[?(@.price < 10)]"      // 价格小于10的书
"$.users[?(@.age >= 18)]"            // 成年用户
"$.products[?(@['in stock'] == true)]" // 有库存的产品

// 递归查询
"$..author"                          // 所有层级的author字段
"$..book[?(@.price)]"               // 所有有价格的书

// 可变操作
auto results = selectAllMutable(data, "$.users[*]");
for (auto& user : results) {
    user.get()["lastUpdate"] = getCurrentTime();
}
```

### 📦 类型注册与序列化
### 📦 类型注册与序列化
- **零侵入设计**: 使用 `JSON_FIELDS()` 宏一行完成类型注册
- **编译时类型安全**: 所有类型检查在编译期完成，零运行时开销
- **自动类型推导**: 自动处理复杂嵌套类型和模板类型
- **递归类型支持**: 支持自引用结构和循环引用处理

### 📦 全面的STL类型支持
- **序列容器**: `std::vector`, `std::list`, `std::deque`, `std::array`
- **关联容器**: `std::map`, `std::unordered_map`, `std::set`, `std::unordered_set`
- **智能指针**: `std::shared_ptr`, `std::unique_ptr`, `std::optional`
- **元组类型**: `std::tuple`, `std::pair`
- **字符串类型**: `std::string`, `std::string_view`

**STL类型示例:**
```cpp
struct ComplexData {
    std::vector<std::string> tags;
    std::map<std::string, std::vector<int>> groups;
    std::optional<std::string> description;
    std::shared_ptr<std::unordered_map<std::string, double>> metrics;
    std::tuple<int, std::string, double> summary;
    
    JSON_FIELDS(tags, groups, description, metrics, summary)
};

// 复杂嵌套类型自动处理！
ComplexData data;
std::string json = data.toJsonString();
ComplexData restored = ComplexData::fromJsonString(json);
```

### 🖼️ Qt类型生态支持
- **字符串类型**: `QString`, `QStringList`
- **几何类型**: `QPointF`, `QRectF`, `QRect`, `QSizeF`, `QSize`
- **图形类型**: `QColor`, `QBrush`, `QPen`
- **容器类型**: `QList<T>`, `QVector<T>`, `QMap<K,V>`

**Qt类型示例:**
```cpp
struct WindowSettings {
    QString title = "My Application";
    QPointF position = {100.0, 200.0};
    QRectF geometry = {0, 0, 800, 600};
    QStringList recentFiles = {"file1.txt", "file2.txt"};
    QColor backgroundColor = QColor(240, 240, 240);
    
    JSON_FIELDS(title, position, geometry, recentFiles, backgroundColor)
};

// Qt类型序列化示例
WindowSettings settings;
QString jsonStr = QString::fromStdString(settings.toJsonString(2));
// 输出:
// {
//   "title": "My Application",
//   "position": {"x": 100.0, "y": 200.0},
//   "geometry": {"x": 0, "y": 0, "width": 800, "height": 600},
//   "recentFiles": ["file1.txt", "file2.txt"],
//   "backgroundColor": {"r": 240, "g": 240, "b": 240, "a": 255}
// }
```

### � 高级功能特性
- **自定义序列化器**: 为特殊类型提供自定义转换逻辑
- **错误恢复**: 解析失败时的智能错误恢复和默认值处理
- **流式处理**: 支持大型JSON数据的内存高效处理
- **性能优化**: 零拷贝操作和编译时优化

## 📚 文档与指南

### 🎯 核心文档
- **[用户指南](docs/USER_GUIDE.md)** - 完整的使用说明和最佳实践
- **[API参考](docs/API_REFERENCE.md)** - 详细的API文档和接口说明
- **[高级特性](docs/ADVANCED_FEATURES.md)** - 高级功能和扩展用法
- **[性能指南](docs/PERFORMANCE_GUIDE.md)** - 性能优化建议和基准测试

### 🆕 版本管理
- **[版本管理指南](docs/VERSION_MANAGEMENT.md)** - 版本系统使用说明和集成指南
- **[变更日志](CHANGELOG.md)** - 详细的版本更新记录

### 🚀 JSONPath系统
- **[JSONPath查询语法](docs/jsonpath_mutable_implementation.md)** - 完整的JSONPath语法参考
- **[空格属性名支持报告](SPACED_PROPERTIES_REPORT.md)** - 带空格属性名的实现细节
- **[可变操作完成报告](JSONPATH_MUTABLE_COMPLETION_REPORT.md)** - 可变JSONPath的实现说明

### 📦 类型系统
- **[类型注册快速开始](docs/type_registry/QUICK_START.md)** - 5分钟上手类型注册
- **[STL类型支持](docs/std_types/README.md)** - 标准库类型的序列化支持
- **[Qt类型集成](docs/qt_types/README.md)** - Qt框架类型的集成说明
- **[JSON引擎](docs/json_engine/README.md)** - 底层JSON处理引擎文档

## 🎯 使用场景

**JsonStruct 适用于以下场景**:

### 🏢 企业级应用
- **配置管理**: 应用程序配置文件的自动序列化和反序列化
- **数据持久化**: 对象状态的可靠保存和恢复
- **API数据交换**: 微服务间的结构化数据通信
- **日志和审计**: 结构化日志数据的生成和处理

### 🖥️ 桌面应用开发
- **Qt应用设置**: 窗口状态、用户偏好设置的自动保存
- **会话恢复**: 应用程序状态的快照和恢复
- **插件配置**: 动态插件的配置数据管理
- **项目文件**: 复杂项目数据的序列化存储

### 🎮 游戏开发
- **存档系统**: 游戏进度和状态的自动保存
- **配置管理**: 游戏设置、键位绑定的持久化
- **关卡数据**: 关卡设计和游戏内容的序列化
- **玩家档案**: 玩家数据和成就系统

### 📊 数据处理
- **大数据查询**: 使用JSONPath进行复杂数据筛选
- **数据转换**: 不同格式间的数据转换和映射
- **批量处理**: 大量结构化数据的高效处理
- **实时分析**: 流式数据的实时查询和分析

### 🌐 Web和网络服务
- **RESTful API**: JSON数据的自动序列化和解析
- **配置服务**: 分布式配置中心的数据管理
- **消息队列**: 结构化消息的序列化传输
- **缓存系统**: 对象缓存的序列化存储

## 📈 性能特性

### ⚡ 编译时优化
- **零运行时开销**: 类型注册和检查在编译期完成
- **模板特化**: 针对不同类型的优化代码生成
- **constexpr优化**: 编译时常量表达式计算
- **内联展开**: 关键路径的函数内联优化

### 🚀 内存效率
- **零拷贝设计**: 移动语义避免不必要的数据复制
- **智能内存管理**: RAII确保资源自动释放
- **引用传递**: 避免大对象的值拷贝
- **懒加载**: JSONPath查询结果按需生成

### 🔍 查询性能
- **早期终止**: 查询在找到结果后可立即停止
- **路径缓存**: 重复查询路径的优化缓存
- **批量处理**: 大数据集的分批处理减少内存压力
- **并行处理**: 支持多线程并行查询(规划中)

### 📊 性能特性说明
> ⚠️ **注意**: 本项目尚未进行系统化的性能基准测试。以下是设计目标和理论特性：

**设计目标**:
- 编译时类型注册，减少运行时开销
- 零拷贝移动语义，提升内存效率  
- JSONPath查询优化，支持早期终止
- 模板特化优化，针对不同类型生成最优代码

**待完成的性能测试**:
- [ ] 系统化基准测试套件
- [ ] 与主流JSON库的性能对比
- [ ] 内存使用分析和优化
- [ ] 大数据集处理性能测试
- [ ] 多线程并发性能测试

*我们计划在v1.3.0版本中加入完整的性能基准测试套件。*

## 🆚 功能特性对比

> ⚠️ **免责声明**: 以下对比基于公开文档和特性分析，不代表性能基准测试结果。

| 特性 | JsonStruct | nlohmann/json | RapidJSON | jsoncpp |
|------|--------------------|--------------|-----------|---------| 
| **学习成本** | 低 (宏驱动) | 中等 (现代API) | 高 (底层API) | 中等 (传统API) |
| **代码维护** | 自动序列化 | 手动适配器 | 手动序列化 | 手动序列化 |
| **STL支持** | 内置支持 | 良好支持 | 需要适配 | 需要适配 |
| **Qt支持** | 内置支持 | 需要适配 | 需要适配 | 需要适配 |
| **类型安全** | 编译时检查 | 运行时检查 | 手动检查 | 手动检查 |
| **JSONPath** | ✅ 完整支持 | ❌ 不支持 | ❌ 不支持 | ❌ 不支持 |
| **版本管理** | ✅ 内置 | ❌ 无 | ❌ 无 | ❌ 无 |
| **文档完整性** | 🚧 开发中 | ✅ 完善 | ✅ 完善 | ✅ 完善 |
| **生态成熟度** | 🆕 新项目 | ✅ 成熟 | ✅ 成熟 | ✅ 成熟 |
| **社区支持** | 🆕 起步 | ✅ 活跃 | ✅ 活跃 | ✅ 稳定 |

### 🎯 JsonStruct 的优势与不足

**✅ 优势**:
- 简洁的API设计，`JSON_FIELDS()`一行搞定
- 原生支持STL和Qt类型，无需额外适配
- 独特的JSONPath查询功能
- 现代C++17+设计理念
- 内置版本管理系统

**⚠️ 当前限制**:
- 新项目，生态和社区还在建设中
- 缺乏大规模生产环境验证
- 性能基准测试尚未完成
- 文档和示例还在完善中
- 第三方库集成需要更多测试

**🎯 适用场景**:
- 新项目希望使用现代C++特性
- 需要JSONPath查询功能的应用
- STL/Qt重度使用的桌面应用
- 对类型安全有较高要求的项目
- 希望减少序列化样板代码的团队

## 🎨 设计理念

### 📏 简洁至上
```cpp
// 传统方式可能需要数十行代码:
class User {
    std::string name;
    int age;
    std::vector<std::string> hobbies;
public:
    nlohmann::json to_json() const {
        nlohmann::json j;
        j["name"] = name;
        j["age"] = age;
        j["hobbies"] = hobbies;
        return j;
    }
    void from_json(const nlohmann::json& j) {
        name = j.at("name").get<std::string>();
        age = j.at("age").get<int>();
        hobbies = j.at("hobbies").get<std::vector<std::string>>();
    }
};

// JsonStruct 只需要：
struct User {
    std::string name;
    int age;
    std::vector<std::string> hobbies;
    JSON_FIELDS(name, age, hobbies)  // 一行搞定！
};
```

### 🔒 类型安全第一
- **编译时检查**: 所有类型错误在编译期发现
- **自动类型推导**: 无需手动指定模板参数
- **强类型转换**: 避免隐式转换导致的错误
- **异常安全**: RAII保证资源正确释放

### 🚀 现代C++设计
- **C++17+特性**: 结构化绑定、if constexpr、类模板参数推导
- **STL兼容**: 完美融入现代C++生态
- **零依赖**: 仅依赖标准库，可选Qt支持
- **跨平台**: Windows、Linux、macOS全平台支持

## 🔧 实际应用示例

### 游戏存档系统
```cpp
struct GameSave {
    std::string playerName;
    int level;
    QPointF position;
    std::vector<std::string> inventory;
    std::map<std::string, int> achievements;
    std::optional<std::string> currentQuest;
    
    JSON_FIELDS(playerName, level, position, inventory, achievements, currentQuest)
};

// 保存游戏
GameSave save;
std::ofstream file("savegame.json");
file << save.toJsonString(2);

// 加载游戏  
std::ifstream loadFile("savegame.json");
std::string saveData((std::istreambuf_iterator<char>(loadFile)),
                     std::istreambuf_iterator<char>());
GameSave loadedSave = GameSave::fromJsonString(saveData);
```

### 配置管理系统
```cpp
struct AppConfig {
    struct WindowSettings {
        QRectF geometry = {100, 100, 800, 600};
        bool maximized = false;
        QString theme = "default";
        JSON_FIELDS(geometry, maximized, theme)
    };
    
    struct NetworkSettings {
        std::string serverUrl = "https://api.example.com";
        int timeout = 30;
        bool useProxy = false;
        JSON_FIELDS(serverUrl, timeout, useProxy)
    };
    
    WindowSettings window;
    NetworkSettings network;
    std::vector<std::string> recentFiles;
    
    JSON_FIELDS(window, network, recentFiles)
};

// 应用启动时加载配置
AppConfig config = AppConfig::fromFile("config.json");

// 应用退出时保存配置
config.toFile("config.json", 2);
```

### 微服务API数据
```cpp
struct ApiResponse {
    struct User {
        int id;
        std::string username;
        std::string email;
        std::optional<std::string> avatar;
        JSON_FIELDS(id, username, email, avatar)
    };
    
    bool success;
    std::string message;
    std::vector<User> users;
    std::map<std::string, std::string> metadata;
    
    JSON_FIELDS(success, message, users, metadata)
};

// 处理API响应
std::string jsonResponse = httpClient.get("/api/users");
ApiResponse response = ApiResponse::fromJsonString(jsonResponse);

// 使用JSONPath查询
auto activeUsers = jsonvalue_jsonpath::selectAll(
    response.toJsonValue(), 
    "$.users[?(@.email)]"
);
```

## 📝 开源协议

本项目采用MIT开源协议。详情请参见项目根目录的LICENSE文件。

## 🤝 贡献指南

我们欢迎各种形式的贡献！无论是功能请求、Bug报告、文档改进还是代码贡献。

### 🚀 快速开始贡献

1. **Fork仓库**并克隆到本地
```bash
git clone https://github.com/xxm-666/json_struct.git
cd json_struct
```

2. **配置开发环境**
```bash
# Windows (Visual Studio)
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"

# Linux/macOS
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

3. **运行完整测试套件**
```bash
# Windows PowerShell
.\run_all_tests.ps1

# Linux/macOS
ctest --verbose
```

### 📋 贡献类型

**🐛 Bug修复**
- 查看[Issues](https://github.com/xxm-666/json_struct/issues)中标记为`bug`的问题
- 提供最小可重现示例
- 包含修复的单元测试

**✨ 新功能开发**
- 新的STL类型支持
- 更多Qt类型注册
- JSONPath功能增强
- 性能优化

**📚 文档改进**
- API文档完善
- 使用示例增加
- 最佳实践指南
- 多语言文档

**🧪 测试用例**
- 边界情况测试
- 性能基准测试
- 跨平台兼容性测试

### 💻 代码规范

```cpp
// 类名: PascalCase
class JsonStructRegistry {
    // 成员函数: camelCase
    bool registerType() const;
    
    // 成员变量: camelCase，私有成员加下划线前缀
private:
    std::string memberVariable_;
    static constexpr int MAX_DEPTH = 100;  // 常量: UPPER_CASE
};

// 缩进: 4空格，不使用Tab
// 行长度: 最大120字符
// 注释: 公共API必须有详细文档
```

### 🔍 Pull Request检查清单

- [ ] 代码遵循项目风格规范
- [ ] 添加了相应的单元测试
- [ ] 所有测试通过(包括新增测试)
- [ ] 更新了相关文档
- [ ] 提交信息清晰描述了更改
- [ ] 没有破坏现有API兼容性

### 🏷️ 版本发布

我们遵循[语义化版本控制](https://semver.org/):
- **主版本号**: 不兼容的API更改
- **次版本号**: 向后兼容的功能性新增
- **修订号**: 向后兼容的问题修正

特别欢迎：
- 新的STL类型支持
- 更多Qt类型注册  
- 性能优化建议
- 使用案例分享
- 跨平台测试

## 🔮 项目路线图

> 📝 **项目状态**: 这是一个活跃开发中的项目，部分功能仍在完善中。

### ✅ 当前已实现 (v1.2.0-dev)
- [x] 基础类型注册系统 (`JSON_FIELDS`宏)
- [x] JSONPath查询引擎核心功能
- [x] 带空格属性名支持 (`['property name']`语法)
- [x] 可变JSONPath操作 (`selectAllMutable`)
- [x] 版本管理系统 (语义化版本控制)
- [x] CMake构建系统集成
- [x] 基础STL类型支持
- [x] 部分Qt类型支持
- [x] 单元测试框架

### 🚧 正在开发 (v1.3.0目标)
- [ ] 完善的错误处理和异常安全
- [ ] 性能基准测试套件建设
- [ ] 更全面的STL容器支持
- [ ] 扩展Qt类型注册
- [ ] 文档完善和示例增加
- [ ] 跨平台兼容性测试

### 📋 近期计划 (v1.4.0目标)  
- [ ] JSONPath性能优化
- [ ] 自定义序列化器简化
- [ ] 流式JSON解析优化
- [ ] 多线程安全性保证
- [ ] 调试和诊断工具

### 🌟 长期愿景 (v2.0.0+)
- [ ] 完整的性能基准和优化
- [ ] 序列化格式扩展(XML/YAML)
- [ ] IDE集成和工具支持
- [ ] 社区生态建设
- [ ] 跨语言绑定探索

### ⚠️ 已知限制和待改进
- 缺乏大规模生产环境验证
- 性能基准测试尚未完成  
- 部分边界情况处理待完善
- 文档和示例需要持续增加
- 社区反馈和贡献机制待建立

---

## 📞 联系与支持

### 💬 社区交流
- **GitHub Issues**: [功能请求和Bug报告](https://github.com/xxm-666/json_struct/issues)
- **GitHub Repository**: [项目主页](https://github.com/xxm-666/json_struct)
- **Email**: 可通过GitHub Issues联系维护者

### 📖 项目资源
- **源码**: [GitHub Repository](https://github.com/xxm-666/json_struct)
- **示例**: [examples/](examples/) 目录包含使用示例
- **文档**: 项目文档正在建设中
- **测试**: [tests/](tests/) 目录包含单元测试

> ⚠️ **注意**: 本项目还在快速开发中，部分链接的文档文件可能尚未创建。

### 🏆 致谢

JsonStruct 的成功离不开以下项目和社区的支持：

- **[nlohmann/json](https://github.com/nlohmann/json)** - 现代C++ JSON库的典范
- **[Qt Project](https://www.qt.io/)** - 提供丰富的跨平台数据类型
- **[CMake](https://cmake.org/)** - 强大的跨平台构建工具
- **所有贡献者** - 感谢每一个Pull Request和Issue报告

## 📄 开源许可

```
MIT License

Copyright (c) 2024 JsonStruct Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

<div align="center">

### 🌟 **让JSON序列化变得简单而优雅**

> 🚧 **开发中项目**: 本项目正在积极开发中，欢迎试用和反馈！

[![GitHub Repository](https://img.shields.io/badge/GitHub-xxm--666%2Fjson__struct-blue?logo=github)](https://github.com/xxm-666/json_struct)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17%2B-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-blue.svg)](https://cmake.org/)

**[⬆️ 回到顶部](#jsonstruct-registry---c17-json序列化与jsonpath查询框架)** | **[🚀 快速开始](#-快速开始)** | **[📋 项目结构](#-项目结构)**

*Built with ❤️ for modern C++ developers*

</div>
