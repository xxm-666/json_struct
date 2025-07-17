# JsonStruct Registry - C++17+ 类型注册与自动序列化系统

> 让C++对象与JSON转换变得简单自动 - 专为STL和Qt类型设计的零侵入序列化框架

## 📁 项目结构

```
jsonstruct_registry/
├── src/                           # 核心源码
│   ├── jsonstruct.h               # 🌟 统一入口头文件 (推荐使用)
│   ├── type_registry/             # 🥇 类型注册系统核心
│   │   ├── registry_core.h        # 类型注册表核心
│   │   ├── auto_serializer.h      # 自动序列化引擎
│   │   └── field_macros.h         # JSON_FIELDS宏定义
│   ├── std_types/                 # 🥈 STL类型注册 (重要)
│   │   ├── std_registry.h         # STL类型注册主头文件
│   │   ├── container_registry.h   # 容器类型注册
│   │   └── custom_types_example.h # 自定义类型示例
│   ├── qt_types/                  # 🖼️ Qt类型注册 (可选)
│   │   ├── qt_registry.h          # 基础Qt类型注册实现
│   │   ├── qt_ultimate_registry.h # 通用的Qt类型注册系统
│   │   └── qt_common.h            # Qt公共类型注册
│   └── json_engine/               # 📦 JSON支撑引擎 (内部使用)
│       ├── json_value.h           # JSON值表示
│       ├── json_value.cpp         # JSON核心实现
│       ├── json_number.h          # 数值精度处理
│       ├── json_stream_parser.h   # 流式解析器
│       ├── json_path.h            # JSONPath查询引擎
│       ├── json_path.cpp          # JSONPath实现
│       ├── json_query_generator.h # 🚀 流式查询生成器
│       └── json_query_generator.cpp # 懒加载查询实现
├── tests/                         # 测试套件
│   ├── test_std_system.cpp        # 🎯 STL类型注册测试 (重点)
│   ├── test_enhanced_features.cpp # 高级特性测试
│   ├── test_comprehensive_demo.cpp# 综合功能演示
│   ├── test_streaming.cpp         # 流式解析测试
│   ├── test_json_parsing.cpp      # JSON解析测试
│   ├── test_jsonpath.cpp          # JSONPath功能测试
│   ├── test_jsonpath_clean.cpp    # JSONPath清洁测试
│   ├── test_jsonpath_unified.cpp  # JSONPath统一测试
│   ├── test_advanced_jsonpath.cpp # JSONPath高级测试
│   ├── test_json_query_generator.cpp # 🚀 流式查询生成器测试
│   ├── test_precision_fix_en.cpp  # 数值精度测试
│   ├── test_special_numbers.cpp   # 特殊数值测试
│   └── test_error_recovery.cpp    # 错误恢复测试
├── examples/                      # 示例程序
│   ├── example_enhanced_simple.cpp   # 🎨 基础类型注册演示 (重点)
│   └── example_jsonpath_complete.cpp # JSONPath完整演示
├── docs/                          # 分层文档
│   ├── type_registry/             # 🎯 类型注册文档 (核心)
│   │   ├── QUICK_START.md         # 5分钟快速开始
│   │   ├── JSON_FIELDS_MACRO.md   # 宏的详细用法
│   │   ├── CUSTOM_TYPES.md        # 自定义类型注册
│   │   └── BEST_PRACTICES.md      # 最佳实践
│   ├── std_types/                 # STL类型文档
│   │   ├── CONTAINER_GUIDE.md     # 容器类型使用指南
│   │   ├── POINTER_GUIDE.md       # 智能指针序列化
│   │   └── STRING_HANDLING.md     # 字符串处理
│   ├── qt_types/                  # Qt类型文档
│   │   ├── QT_INTEGRATION.md      # Qt集成指南
│   │   ├── GEOMETRY_TYPES.md      # 几何类型序列化
│   │   └── GRAPHICS_TYPES.md      # 图形类型序列化
│   └── json_engine/               # JSON引擎文档 (支撑)
│       ├── JSON_BASICS.md         # JSON基础操作
│       ├── PARSER_REFERENCE.md    # 解析器参考
│       └── SERIALIZER_REFERENCE.md # 序列化器参考
├── CMakeLists.txt                 # CMake构建配置
└── README.md                      # 项目说明
```
## 🚀 快速开始 - 一行代码搞定序列化

### ✨ STL类型自动序列化

```cpp
#include "jsonstruct.h"                    // 🌟 新的统一入口
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

### 🛠️ Qt类型无缝集成

```cpp
#include "jsonstruct.h"                    // 统一入口自动包含Qt支持
using namespace JsonStruct;

struct WindowConfig {
    QString title = "My Window";
    QPointF position = {100.0, 200.0};
    QRectF geometry = {0, 0, 800, 600};
    QStringList recentFiles = {"file1.txt", "file2.txt"};
    QColor backgroundColor = QColor(255, 255, 255);
    
    JSON_FIELDS(title, position, geometry, recentFiles, backgroundColor)
};

// 使用方法完全相同
WindowConfig config;
QString jsonStr = QString::fromStdString(config.toJsonString(2));
WindowConfig restored = WindowConfig::fromJsonString(jsonStr.toStdString());
```

### 📦 项目构建

```bash
# 配置CMake
cmake -B build -S .

# 构建所有目标
cmake --build build --config Release

# 运行类型注册系统测试
./build/Release/test_std_system          # STL类型测试
./build/Release/test_json_query_generator # 流式查询生成器测试
./build/Release/example_enhanced_simple  # 基础功能演示
```

## 🏆 核心特性

### 🎯 零侵入类型注册
- **一行宏搞定**: 使用 `JSON_FIELDS()` 宏即可完成类型注册
- **无需修改现有代码**: 对现有类结构零侵入
- **编译时类型安全**: 所有类型检查在编译期完成
- **自动推导**: 自动处理复杂嵌套类型

### 📦 全面的STL类型支持
- **基础容器**: `std::vector`, `std::list`, `std::deque`, `std::array`
- **关联容器**: `std::map`, `std::unordered_map`, `std::set`, `std::unordered_set`
- **智能指针**: `std::shared_ptr`, `std::unique_ptr`, `std::optional`
- **元组类型**: `std::tuple`, `std::pair`
- **字符串类型**: `std::string`, `std::string_view`

**STL类型示例:**
```cpp
struct DataContainer {
    std::vector<std::string> names;
    std::map<std::string, std::vector<int>> groups;
    std::optional<std::string> description;
    std::shared_ptr<std::map<std::string, double>> settings;
    
    JSON_FIELDS(names, groups, description, settings)
};

// 复杂嵌套类型自动处理！
DataContainer data;
std::string json = data.toJsonString();
DataContainer restored = DataContainer::fromJsonString(json);
```

### 🖼️ Qt类型生态支持
- **字符串类型**: `QString`, `QStringList`
- **几何类型**: `QPointF`, `QRectF`, `QRect`, `QSizeF`
- **图形类型**: `QColor`, `QBrush`, `QPen`
- **容器类型**: `QList<T>`, `QVector<T>`, `QMap<K,V>`

### 🔧 高级类型注册机制
- **自定义序列化器**: 为特殊类型提供自定义转换逻辑
- **类型注册表**: 运行时类型注册和查询系统
- **递归类型支持**: 自动处理嵌套对象和循环引用
- **默认值支持**: 反序列化时的智能默认值处理

### 🚀 流式查询生成器 (JsonQueryGenerator)
- **内存高效**: 懒加载模式，只在需要时生成结果
- **早期终止**: 支持查找第一个匹配项后立即停止
- **批量处理**: 可配置的批量大小，适合大数据处理
- **迭代器接口**: 现代C++范围for循环支持
- **自定义处理**: 支持自定义结果处理函数

**流式查询示例:**
```cpp
#include "jsonstruct.h"
using namespace JsonStruct;

// 创建大型JSON数据
JsonValue largeData = JsonValue::Array{...}; // 假设有10万条记录

// 使用流式查询找到第一个匹配项 (内存高效)
auto firstMatch = JsonStreamingQuery::findFirst(largeData, "$.users[?(@.age > 30)]");
if (firstMatch) {
    std::cout << "找到匹配用户: " << firstMatch->first->toString() << std::endl;
}

// 懒加载处理大量结果
JsonStreamingQuery::lazyQuery(largeData, "$.products[*]", 
    [](const JsonValue* value, const std::string& path) -> bool {
        // 处理每个产品，返回false可提前终止
        std::cout << "处理产品: " << path << std::endl;
        return true; // 继续处理下一个
    });

// 批量流式处理
auto generator = JsonStreamingQuery::createGenerator(largeData, "$.items[*]", 
    {.maxResults = 1000, .batchSize = 50});

for (auto it = generator.begin(); it != generator.end(); ++it) {
    // 自动按批次处理，内存占用恒定
    processItem(it->first, it->second);
}
```

**自定义类型注册:**
```cpp
// 注册自定义类型
TypeRegistry::registerType<MyCustomType>(
    // 序列化函数
    [](const MyCustomType& obj) -> JsonValue {
        return JsonValue::Object{{"data", obj.getData()}};
    },
    // 反序列化函数
    [](const JsonValue& json, const MyCustomType& defaultVal) -> MyCustomType {
        if (json.hasKey("data")) {
            return MyCustomType(json["data"].toString());
        }
        return defaultVal;
    }
);
```

### 🚀 现代C++17+特性
- **constexpr支持**: 编译时常量表达式优化
- **完美转发**: 高效的参数传递
- **结构化绑定**: 现代C++语法支持
- **类型推导**: auto和decltype的广泛使用
- **移动语义**: 零拷贝优化和RAII资源管理

## 📚 文档与指南

### 🎯 类型注册系统 (核心文档)
- **[5分钟快速开始](docs/type_registry/QUICK_START.md)** - 立即上手类型注册系统
- **[JSON_FIELDS宏详解](docs/type_registry/JSON_FIELDS_MACRO.md)** - 核心宏的完整用法
- **[自定义类型注册](docs/type_registry/CUSTOM_TYPES.md)** - 注册您自己的类型
- **[最佳实践](docs/type_registry/BEST_PRACTICES.md)** - 性能优化和使用建议

### 📦 STL类型支持
- **[容器类型指南](docs/std_types/CONTAINER_GUIDE.md)** - vector, map, set等容器的序列化
- **[智能指针处理](docs/std_types/POINTER_GUIDE.md)** - shared_ptr, unique_ptr, optional的使用
- **[字符串处理](docs/std_types/STRING_HANDLING.md)** - string, string_view的处理方式

### 🖼️ Qt类型集成
- **[Qt集成指南](docs/qt_types/QT_INTEGRATION.md)** - Qt环境设置和基础使用
- **[几何类型序列化](docs/qt_types/GEOMETRY_TYPES.md)** - QPointF, QRectF等几何类型
- **[图形类型序列化](docs/qt_types/GRAPHICS_TYPES.md)** - QColor, QBrush等图形类型

### 🔧 JSON引擎 (支撑文档)
- **[JSON基础操作](docs/json_engine/JSON_BASICS.md)** - 底层JSON操作参考
- **[解析器参考](docs/json_engine/PARSER_REFERENCE.md)** - JSON解析器API
- **[序列化器参考](docs/json_engine/SERIALIZER_REFERENCE.md)** - JSON序列化器API

## 🎯 使用场景

**理想选择适用于**:
- 🏢 **企业级应用**: 需要可靠配置管理和数据持久化的生产环境
- 📊 **数据交换**: 微服务间通信、API数据传输
- ⚙️ **配置管理**: 应用配置文件的序列化和反序列化
- 🎮 **游戏开发**: 游戏数据、存档文件的自动序列化  
- 🖥️ **桌面应用**: Qt应用的设置和状态保存
- 🌐 **跨平台项目**: 需要统一数据格式的多平台应用
- 📈 **大数据处理**: 使用流式查询处理大型JSON数据集
- 🔍 **实时查询**: 需要高效JSONPath查询和结果流式处理的场景

## 📈 性能特性

- **零拷贝优化**: 移动语义避免不必要的数据复制
- **编译时优化**: 模板和constexpr的广泛使用
- **类型注册缓存**: 运行时类型查询O(1)复杂度
- **内存高效**: 最小化序列化过程中的内存分配
- **流式处理**: JsonQueryGenerator支持大数据懒加载处理
- **早期终止**: 查询可在找到结果后立即停止，节省计算资源

## 🆚 对比其他方案

| 特性 | JsonStruct Registry | 手写序列化 | 其他JSON库 |
|------|--------------------|-----------|---------| 
| **学习成本** | ⭐⭐⭐⭐⭐ 极低 | ⭐⭐ 高 | ⭐⭐⭐ 中等 |
| **代码维护** | ⭐⭐⭐⭐⭐ 自动化 | ⭐ 手动维护 | ⭐⭐⭐ 需要适配 |
| **STL支持** | ⭐⭐⭐⭐⭐ 完整 | ⭐⭐ 需手写 | ⭐⭐⭐ 部分支持 |
| **Qt支持** | ⭐⭐⭐⭐⭐ 原生 | ⭐ 需手写 | ⭐ 基本不支持 |
| **类型安全** | ⭐⭐⭐⭐⭐ 编译时 | ⭐⭐⭐ 运行时 | ⭐⭐⭐ 运行时 |
| **大数据处理** | ⭐⭐⭐⭐⭐ 流式懒加载 | ⭐⭐ 需手动优化 | ⭐⭐⭐ 内存密集 |
| **性能** | ⭐⭐⭐⭐ 优秀 | ⭐⭐⭐⭐⭐ 最优 | ⭐⭐⭐ 良好 |

## 🎨 设计理念

### 📏 简洁至上
```cpp
// 其他方案可能需要：
class User {
    std::string name;
    int age;
public:
    void serialize(JsonWriter& writer) { /* 20+ lines */ }
    void deserialize(JsonReader& reader) { /* 20+ lines */ }
};

// JsonStruct Registry只需要：
struct User {
    std::string name;
    int age;
    JSON_FIELDS(name, age)  // 一行搞定！
};
```

### 🔒 类型安全第一
- 编译时类型检查，运行时零意外
- 自动类型推导，无需手动指定类型
- 强类型转换，避免隐式转换错误

## 📝 开源协议

本项目采用MIT开源协议。详情请参见项目根目录的LICENSE文件。

## 🤝 贡献

欢迎提交Issue和Pull Request来完善这个项目！

特别欢迎：
- 新的STL类型支持
- 更多Qt类型注册
- 性能优化建议
- 使用案例分享

## 🔮 路线图

### 短期目标
- [ ] 完善更多STL容器类型支持
- [ ] 增加Qt6新类型支持
- [ ] 优化序列化性能
- [ ] 添加更多使用示例

### 长期愿景
- [ ] 支持自定义序列化格式（XML、YAML等）
- [ ] 提供可视化类型注册工具
- [ ] 集成到主流IDE中
- [ ] 建立类型注册生态系统

---

**JsonStruct Registry** - 让C++对象序列化变得简单自动，专为STL和Qt设计的现代C++17+类型注册框架。
