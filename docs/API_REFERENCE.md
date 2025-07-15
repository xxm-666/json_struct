# JsonStruct Registry - API 参考手册

## JsonValue 核心类

### 构造函数

```cpp
// 默认构造函数 - 创建null值
JsonValue();

// 基本类型构造函数
JsonValue(bool value);
JsonValue(int value);
JsonValue(long long value);
JsonValue(double value);
JsonValue(const std::string& value);
JsonValue(const char* value);

// 容器构造函数
JsonValue(const ArrayType& array);
JsonValue(const ObjectType& object);
JsonValue(std::initializer_list<JsonValue> values);
```

### 类型检查方法

```cpp
bool isNull() const noexcept;
bool isBool() const noexcept;
bool isNumber() const noexcept;
bool isString() const noexcept;
bool isArray() const noexcept;
bool isObject() const noexcept;

// 数值类型细分检查
bool isInteger() const noexcept;
bool isFloatingPoint() const noexcept;
```

### 值获取方法

```cpp
bool toBool() const;
int toInt() const;
long long toLongLong() const;
double toDouble() const;
std::string toString() const;
const ArrayType& toArray() const;
const ObjectType& toObject() const;

// 安全获取方法
std::optional<bool> getBool() const noexcept;
std::optional<int> getInt() const noexcept;
std::optional<double> getDouble() const noexcept;
std::optional<std::string> getString() const noexcept;
```

### 容器操作

```cpp
// 数组操作
size_t size() const;
bool empty() const;
void append(const JsonValue& value);
JsonValue& operator[](size_t index);
const JsonValue& operator[](size_t index) const;

// 对象操作
bool contains(const std::string& key) const;
JsonValue& operator[](const std::string& key);
const JsonValue& operator[](const std::string& key) const;
```

### 序列化和解析

```cpp
// 静态解析方法
static JsonValue parse(const std::string& json);
static JsonValue parse(const std::string& json, const ParseOptions& options);

// 序列化方法
std::string serialize() const;
std::string serialize(const SerializeOptions& options) const;
std::string serialize(int indent) const; // 简化版本，仅指定缩进
```

## JsonNumber 高精度数值类

```cpp
class JsonNumber {
public:
    // 构造函数
    JsonNumber();
    JsonNumber(long long value);
    JsonNumber(double value);
    JsonNumber(const std::string& value);
    
    // 类型检查
    bool isInteger() const;
    bool isFloatingPoint() const;
    bool isSpecial() const; // NaN, Infinity
    
    // 值获取
    long long toInteger() const;
    double toDouble() const;
    std::string toString() const;
    
    // 特殊值检查
    bool isNaN() const;
    bool isInfinity() const;
    bool isNegativeInfinity() const;
};
```

## 类型注册系统

### TypeRegistry 类

```cpp
class TypeRegistry {
public:
    static TypeRegistry& instance();
    
    // 类型注册
    template<typename T>
    void registerType(
        std::function<JsonValue(const T&)> toJson,
        std::function<T(const JsonValue&, const T&)> fromJson
    );
    
    // 序列化
    template<typename T>
    JsonValue toJson(const T& value) const;
    
    // 反序列化
    template<typename T>
    T fromJson(const JsonValue& json, const T& defaultValue) const;
};
```

### 类型注册宏

```cpp
// 简单的类型注册宏
REGISTER_JSON_TYPE(TypeName, toJsonLambda, fromJsonLambda)

// 示例
REGISTER_JSON_TYPE(Point3D,
    [](const Point3D& point) -> JsonValue {
        JsonValue::ArrayType arr;
        arr.push_back(JsonValue(point.x));
        arr.push_back(JsonValue(point.y));
        arr.push_back(JsonValue(point.z));
        return JsonValue(arr);
    },
    [](const JsonValue& json, const Point3D& defaultValue) -> Point3D {
        if (json.isArray()) {
            const auto& arr = json.toArray();
            if (arr.size() == 3) {
                return Point3D(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
            }
        }
        return defaultValue;
    }
);
```

## JSONPath 查询

JsonStruct支持完整的JSONPath查询语言，用于复杂的JSON数据提取。

### 基本查询方法

```cpp
// 检查路径是否存在
bool pathExists(const std::string& jsonpath_expression) const;

// 选择第一个匹配项
const JsonValue* selectFirst(const std::string& jsonpath_expression) const;

// 选择所有匹配项 (返回指针)
std::vector<const JsonValue*> selectAll(const std::string& jsonpath_expression) const;

// 选择所有匹配项 (返回值副本)
std::vector<JsonValue> selectValues(const std::string& jsonpath_expression) const;
```

### 支持的JSONPath语法

```cpp
JsonValue json = JsonValue::parse(R"({
    "store": {
        "book": [
            {"title": "Book 1", "price": 10.99, "category": "fiction"},
            {"title": "Book 2", "price": 15.99, "category": "science"},
            {"title": "Book 3", "price": 8.99, "category": "fiction"}
        ],
        "bicycle": {
            "color": "red",
            "price": 19.95
        }
    },
    "numbers": [10, 20, 30, 40, 50]
})");

// 1. 基本属性访问
auto title = json.selectFirst("$.store.book[0].title");  // "Book 1"

// 2. 数组索引
auto price = json.selectFirst("$.store.book[1].price");  // 15.99
auto number = json.selectFirst("$.numbers[2]");          // 30

// 3. 数组切片
auto slice = json.selectAll("$.numbers[1:4]");           // [20, 30, 40]

// 4. 通配符选择
auto allTitles = json.selectAll("$.store.book[*].title"); // ["Book 1", "Book 2", "Book 3"]
auto rootProps = json.selectAll("$.*");                   // [store对象, numbers数组]

// 5. 递归下降
auto allPrices = json.selectAll("$..price");             // [10.99, 15.99, 8.99, 19.95]

// 6. 路径存在性检查
bool exists = json.pathExists("$.store.bicycle.color");  // true
bool missing = json.pathExists("$.store.car");           // false

// 7. 多值选择与复制
auto pricePointers = json.selectAll("$..price");         // 返回指向原数据的指针
auto priceValues = json.selectValues("$..price");        // 返回数据副本
```

### JSONPath功能完整性

| 功能 | 语法示例 | 支持状态 |
|------|----------|----------|
| 根节点 | `$` | ✅ 完全支持 |
| 属性访问 | `$.prop` | ✅ 完全支持 |
| 嵌套属性 | `$.prop.subprop` | ✅ 完全支持 |
| 数组索引 | `$.arr[0]` | ✅ 完全支持 |
| 数组切片 | `$.arr[1:3]` | ✅ 完全支持 |
| 数组通配符 | `$.arr[*]` | ✅ 完全支持 |
| 对象通配符 | `$.*` | ✅ 完全支持 |
| 递归下降 | `$..prop` | ✅ 完全支持 |
| 复合表达式 | `$.books[*].title` | ✅ 完全支持 |
| 路径验证 | `pathExists()` | ✅ 完全支持 |
| 多值查询 | `selectAll()` | ✅ 完全支持 |

## 配置选项

### ParseOptions

```cpp
struct ParseOptions {
    size_t maxDepth = 512;              // 最大嵌套深度
    bool allowComments = false;         // 允许注释
    bool allowTrailingCommas = false;   // 允许尾随逗号
    bool strictMode = true;             // 严格模式
    bool validateUtf8 = true;           // UTF-8验证
    bool allowSpecialNumbers = false;   // 允许NaN/Infinity
    bool allowRecovery = false;         // 错误恢复
};
```

### SerializeOptions

```cpp
struct SerializeOptions {
    int indent = -1;                    // 缩进级别
    bool sortKeys = false;              // 排序对象键
    bool escapeUnicode = false;         // 转义Unicode
    bool compactArrays = false;         // 紧凑数组格式
    size_t maxPrecision = 15;           // 浮点数精度
    bool allowSpecialNumbers = false;   // 序列化特殊数值
};
```

## 错误处理

```cpp
// 解析异常
class JsonParseException : public std::runtime_error {
public:
    size_t position() const;
    size_t line() const;
    size_t column() const;
    std::string locationInfo() const;
};

// 类型异常
class JsonTypeException : public std::runtime_error {
public:
    Type expectedType() const;
    Type actualType() const;
};
```

## 流式解析器

```cpp
class JsonStreamParser {
public:
    enum class Event {
        ObjectStart, ObjectEnd,
        ArrayStart, ArrayEnd,
        Key, Value, Error
    };
    
    void feedData(const std::string& chunk);
    Event nextEvent();
    JsonValue getCurrentValue() const;
    std::string getErrorMessage() const;
};
```

## JsonFilter 类

### 方法

```cpp
// 添加过滤条件
void addCondition(const std::string& path, std::function<bool(const JsonValue&)> condition);

// 应用过滤器
JsonValue apply(const JsonValue& input) const;
```

### 示例

```cpp
JsonFilter filter;
filter.addCondition("$.items[*].id", [](const JsonValue& value) {
    return value.toInt() > 1;
});
JsonValue filtered = filter.apply(data);
```

## JsonQueryGenerator 类

### 方法

```cpp
// 添加查询路径
void addPath(const std::string& path);

// 生成查询
std::vector<std::string> generate() const;
```

### 示例

```cpp
JsonQueryGenerator generator;
generator.addPath("$.store.book[*].title");
generator.addPath("$.store.bicycle.price");
std::vector<std::string> queries = generator.generate();
```
