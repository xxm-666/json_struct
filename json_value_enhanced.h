#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <variant>
#include <optional>
#include <string_view>
#include <utility>
#include <type_traits>
#include <algorithm>
#include <array>
#include <set>
#include <map>
#include <charconv>
#include <cmath>

namespace JsonStruct {

// 增强版的JSON值类型，全面支持C++17及以上版本
class JsonValueEnhanced {
public:
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    using ArrayType = std::vector<JsonValueEnhanced>;
    using ObjectType = std::unordered_map<std::string, JsonValueEnhanced>;
    
    // 使用std::variant提供类型安全，支持更多C++17特性
    using ValueType = std::variant<
        std::monostate,  // Null
        bool,            // Bool
        double,          // Number
        std::string,     // String
        ArrayType,       // Array
        ObjectType       // Object
    >;

    // 解析选项
    struct ParseOptions {
        size_t maxDepth = 512;          // 最大嵌套深度
        bool allowComments = false;     // 是否允许注释（JSON5风格）
        bool allowTrailingCommas = false; // 是否允许尾随逗号
        bool strictMode = true;         // 严格模式
        bool validateUtf8 = true;       // 验证UTF-8编码
    };

    // 序列化选项
    struct SerializeOptions {
        int indent = -1;                // 缩进级别，-1表示紧凑模式
        bool sortKeys = false;          // 是否排序对象键
        bool escapeUnicode = false;     // 是否转义Unicode字符
        bool compactArrays = false;     // 是否紧凑数组格式
        size_t maxPrecision = 15;       // 浮点数精度
    };

private:
    ValueType value_;
    
    // 解析上下文，提供详细错误信息
    struct ParseContext {
        std::string_view source;
        size_t position = 0;
        size_t line = 1;
        size_t column = 1;
        size_t depth = 0;
        ParseOptions options;
        
        void advance(char c) {
            ++position;
            if (c == '\n') {
                ++line;
                column = 1;
            } else {
                ++column;
            }
        }
        
        std::string locationInfo() const {
            return "line " + std::to_string(line) + ", column " + std::to_string(column);
        }
        
        char peek(size_t offset = 0) const {
            size_t pos = position + offset;
            return pos < source.length() ? source[pos] : '\0';
        }
        
        bool hasMore() const {
            return position < source.length();
        }
        
        void validateDepth() {
            if (depth >= options.maxDepth) {
                throw std::runtime_error("Maximum nesting depth (" + 
                    std::to_string(options.maxDepth) + ") exceeded at " + locationInfo());
            }
        }
    };

public:
    // 默认构造函数 - 构造null值
    JsonValueEnhanced() : value_(std::monostate{}) {}
    
    // 基本类型构造函数
    explicit JsonValueEnhanced(std::nullptr_t) : value_(std::monostate{}) {}
    explicit JsonValueEnhanced(bool b) : value_(b) {}
    explicit JsonValueEnhanced(int i) : value_(static_cast<double>(i)) {}
    explicit JsonValueEnhanced(long long ll) : value_(static_cast<double>(ll)) {}
    explicit JsonValueEnhanced(float f) : value_(static_cast<double>(f)) {}
    explicit JsonValueEnhanced(double d) : value_(d) {}
    explicit JsonValueEnhanced(std::string s) : value_(std::move(s)) {}
    explicit JsonValueEnhanced(std::string_view sv) : value_(std::string(sv)) {}
    explicit JsonValueEnhanced(const char* str) : value_(std::string(str)) {}
    explicit JsonValueEnhanced(ArrayType arr) : value_(std::move(arr)) {}
    explicit JsonValueEnhanced(ObjectType obj) : value_(std::move(obj)) {}

    // 容器构造函数 - 支持更多标准容器
    template<typename T, size_t N>
    explicit JsonValueEnhanced(const std::array<T, N>& arr) {
        ArrayType jsonArr;
        jsonArr.reserve(N);
        for (const auto& item : arr) {
            jsonArr.emplace_back(makeValue(item));
        }
        value_ = std::move(jsonArr);
    }
    
    template<typename T>
    explicit JsonValueEnhanced(const std::vector<T>& vec) {
        ArrayType jsonArr;
        jsonArr.reserve(vec.size());
        for (const auto& item : vec) {
            jsonArr.emplace_back(makeValue(item));
        }
        value_ = std::move(jsonArr);
    }
    
    template<typename T>
    explicit JsonValueEnhanced(const std::set<T>& s) {
        ArrayType jsonArr;
        jsonArr.reserve(s.size());
        for (const auto& item : s) {
            jsonArr.emplace_back(makeValue(item));
        }
        value_ = std::move(jsonArr);
    }
    
    template<typename K, typename V>
    explicit JsonValueEnhanced(const std::map<K, V>& m) {
        ObjectType jsonObj;
        for (const auto& [key, val] : m) {
            jsonObj.emplace(toString(key), makeValue(val));
        }
        value_ = std::move(jsonObj);
    }

    // 移动构造和赋值
    JsonValueEnhanced(JsonValueEnhanced&&) = default;
    JsonValueEnhanced& operator=(JsonValueEnhanced&&) = default;
    
    // 拷贝构造和赋值
    JsonValueEnhanced(const JsonValueEnhanced&) = default;
    JsonValueEnhanced& operator=(const JsonValueEnhanced&) = default;

    // 析构函数
    ~JsonValueEnhanced() = default;

    // 类型查询
    Type type() const noexcept {
        return static_cast<Type>(value_.index());
    }
    
    constexpr bool isNull() const noexcept { return std::holds_alternative<std::monostate>(value_); }
    constexpr bool isBool() const noexcept { return std::holds_alternative<bool>(value_); }
    constexpr bool isNumber() const noexcept { return std::holds_alternative<double>(value_); }
    constexpr bool isString() const noexcept { return std::holds_alternative<std::string>(value_); }
    constexpr bool isArray() const noexcept { return std::holds_alternative<ArrayType>(value_); }
    constexpr bool isObject() const noexcept { return std::holds_alternative<ObjectType>(value_); }

    // 安全的值获取 - 使用std::optional
    std::optional<bool> getBool() const noexcept {
        if (auto* val = std::get_if<bool>(&value_)) {
            return *val;
        }
        return std::nullopt;
    }
    
    std::optional<double> getNumber() const noexcept {
        if (auto* val = std::get_if<double>(&value_)) {
            return *val;
        }
        return std::nullopt;
    }
    
    std::optional<std::string_view> getString() const noexcept {
        if (auto* val = std::get_if<std::string>(&value_)) {
            return *val;
        }
        return std::nullopt;
    }
    
    const ArrayType* getArray() const noexcept {
        return std::get_if<ArrayType>(&value_);
    }
    
    ArrayType* getArray() noexcept {
        return std::get_if<ArrayType>(&value_);
    }
    
    const ObjectType* getObject() const noexcept {
        return std::get_if<ObjectType>(&value_);
    }
    
    ObjectType* getObject() noexcept {
        return std::get_if<ObjectType>(&value_);
    }

    // 带默认值的获取方法
    bool toBool(bool defaultValue = false) const noexcept {
        return getBool().value_or(defaultValue);
    }

    int toInt(int defaultValue = 0) const noexcept {
        if (auto num = getNumber()) {
            return static_cast<int>(*num);
        }
        return defaultValue;
    }

    long long toLongLong(long long defaultValue = 0) const noexcept {
        if (auto num = getNumber()) {
            return static_cast<long long>(*num);
        }
        return defaultValue;
    }

    double toDouble(double defaultValue = 0.0) const noexcept {
        return getNumber().value_or(defaultValue);
    }

    std::string toString(const std::string& defaultValue = "") const {
        if (auto str = getString()) {
            return std::string(*str);
        }
        return defaultValue;
    }

    const ArrayType& toArray() const {
        if (auto* arr = getArray()) {
            return *arr;
        }
        throw std::runtime_error("JsonValue is not an array");
    }

    ArrayType& toArray() {
        if (auto* arr = getArray()) {
            return *arr;
        }
        throw std::runtime_error("JsonValue is not an array");
    }

    const ObjectType& toObject() const {
        if (auto* obj = getObject()) {
            return *obj;
        }
        throw std::runtime_error("JsonValue is not an object");
    }

    ObjectType& toObject() {
        if (auto* obj = getObject()) {
            return *obj;
        }
        throw std::runtime_error("JsonValue is not an object");
    }

    // 数组操作
    void append(JsonValueEnhanced value) {
        if (!isArray()) {
            value_ = ArrayType{};
        }
        std::get<ArrayType>(value_).emplace_back(std::move(value));
    }
    
    template<typename T>
    void append(T&& value) {
        append(makeValue(std::forward<T>(value)));
    }

    JsonValueEnhanced& operator[](size_t index) {
        if (!isArray()) {
            value_ = ArrayType{};
        }
        auto& arr = std::get<ArrayType>(value_);
        if (index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }

    const JsonValueEnhanced& operator[](size_t index) const {
        static const JsonValueEnhanced nullValue;
        if (auto* arr = getArray()) {
            if (index < arr->size()) {
                return (*arr)[index];
            }
        }
        return nullValue;
    }

    // 对象操作
    JsonValueEnhanced& operator[](std::string_view key) {
        if (!isObject()) {
            value_ = ObjectType{};
        }
        return std::get<ObjectType>(value_)[std::string(key)];
    }

    const JsonValueEnhanced& operator[](std::string_view key) const {
        static const JsonValueEnhanced nullValue;
        if (auto* obj = getObject()) {
            auto it = obj->find(std::string(key));
            if (it != obj->end()) {
                return it->second;
            }
        }
        return nullValue;
    }

    bool contains(std::string_view key) const noexcept {
        if (auto* obj = getObject()) {
            return obj->find(std::string(key)) != obj->end();
        }
        return false;
    }
    
    void erase(std::string_view key) {
        if (auto* obj = getObject()) {
            obj->erase(std::string(key));
        }
    }

    // 大小和空检查
    size_t size() const noexcept {
        if (auto* arr = getArray()) return arr->size();
        if (auto* obj = getObject()) return obj->size();
        return 0;
    }
    
    bool empty() const noexcept {
        return size() == 0;
    }

    // 序列化
    std::string dump(const SerializeOptions& options = {}) const {
        std::ostringstream oss;
        dumpImpl(oss, options, 0);
        return oss.str();
    }
    
    // 兼容性重载：接受缩进参数
    std::string dump(int indent) const {
        SerializeOptions options;
        options.indent = indent;
        return dump(options);
    }

    // 解析
    static JsonValueEnhanced parse(std::string_view str, const ParseOptions& options = {}) {
        ParseContext ctx{str, 0, 1, 1, 0, options};
        auto result = parseValue(ctx);
        skipWhitespace(ctx);
        if (ctx.hasMore()) {
            throw std::runtime_error("Extra characters after JSON at " + ctx.locationInfo());
        }
        return result;
    }

    // 比较操作
    bool operator==(const JsonValueEnhanced& other) const noexcept {
        return value_ == other.value_;
    }

    bool operator!=(const JsonValueEnhanced& other) const noexcept {
        return !(*this == other);
    }

    // 访问者模式支持
    template<typename Visitor>
    constexpr auto visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }

    template<typename Visitor>
    constexpr auto visit(Visitor&& visitor) {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }

    // JSON指针支持 (RFC 6901)
    JsonValueEnhanced& at(std::string_view jsonPointer);
    const JsonValueEnhanced& at(std::string_view jsonPointer) const;

private:
    // 辅助函数
    template<typename T>
    static JsonValueEnhanced makeValue(T&& value) {
        if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
            return JsonValueEnhanced(value);
        } else if constexpr (std::is_integral_v<std::decay_t<T>>) {
            return JsonValueEnhanced(static_cast<double>(value));
        } else if constexpr (std::is_floating_point_v<std::decay_t<T>>) {
            return JsonValueEnhanced(static_cast<double>(value));
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            return JsonValueEnhanced(std::string(std::forward<T>(value)));
        } else {
            return JsonValueEnhanced(std::forward<T>(value));
        }
    }
    
    template<typename T>
    static std::string toString(const T& value) {
        if constexpr (std::is_convertible_v<T, std::string>) {
            return std::string(value);
        } else {
            return std::to_string(value);
        }
    }

    // 序列化实现
    void dumpImpl(std::ostream& os, const SerializeOptions& options, int currentIndent) const;
    static std::string escapeString(std::string_view str, bool escapeUnicode = false);
    
    // 解析实现
    static JsonValueEnhanced parseValue(ParseContext& ctx);
    static void skipWhitespace(ParseContext& ctx);
    static JsonValueEnhanced parseNull(ParseContext& ctx);
    static JsonValueEnhanced parseBool(ParseContext& ctx);
    static JsonValueEnhanced parseNumber(ParseContext& ctx);
    static JsonValueEnhanced parseString(ParseContext& ctx);
    static JsonValueEnhanced parseArray(ParseContext& ctx);
    static JsonValueEnhanced parseObject(ParseContext& ctx);
    static std::string parseUnicodeEscape(std::string_view str, size_t& pos);
    static bool isValidUtf8(std::string_view str);
    
    // JSON指针实现
    static std::vector<std::string> parseJsonPointer(std::string_view pointer);
    static std::string unescapeJsonPointer(std::string_view token);
};

// 工厂函数和便利函数
template<typename T>
JsonValueEnhanced makeJson(T&& value) {
    return JsonValueEnhanced(std::forward<T>(value));
}

// 字面量操作符 (C++14+)
namespace literals {
    JsonValueEnhanced operator""_json(const char* str, size_t len);
}

// 流操作符
std::ostream& operator<<(std::ostream& os, const JsonValueEnhanced& value);
std::istream& operator>>(std::istream& is, JsonValueEnhanced& value);

} // namespace JsonStruct
