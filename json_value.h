#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace JsonStruct {

// 简化的JSON值类型，避免复杂的union操作
class JsonValue {
public:
    enum Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    using ArrayType = std::vector<JsonValue>;
    using ObjectType = std::map<std::string, JsonValue>;

private:
    Type type_;
    union {
        bool bool_value;
        double number_value;
        std::string* string_value;
        ArrayType* array_value;
        ObjectType* object_value;
    };

    void clear();
    void copyFrom(const JsonValue& other);
    void moveFrom(JsonValue&& other) noexcept;

public:
    // 构造函数
    JsonValue() : type_(Null) {}
    
    JsonValue(std::nullptr_t) : type_(Null) {}
    
    JsonValue(bool b) : type_(Bool), bool_value(b) {}
    
    JsonValue(int i) : type_(Number), number_value(static_cast<double>(i)) {}
    
    JsonValue(long long i) : type_(Number), number_value(static_cast<double>(i)) {}
    
    JsonValue(double d) : type_(Number), number_value(d) {}
    
    JsonValue(float f) : type_(Number), number_value(static_cast<double>(f)) {}
    
    JsonValue(const char* s) : type_(String) {
        string_value = new std::string(s);
    }
    
    JsonValue(const std::string& s) : type_(String) {
        string_value = new std::string(s);
    }
    
    JsonValue(const ArrayType& arr) : type_(Array) {
        array_value = new ArrayType(arr);
    }
    
    JsonValue(const ObjectType& obj) : type_(Object) {
        object_value = new ObjectType(obj);
    }

    // 拷贝构造函数
    JsonValue(const JsonValue& other) : type_(other.type_) {
        copyFrom(other);
    }

    // 移动构造函数
    JsonValue(JsonValue&& other) noexcept : type_(other.type_) {
        moveFrom(std::move(other));
    }

    // 析构函数
    ~JsonValue() {
        clear();
    }

    // 赋值操作符
    JsonValue& operator=(const JsonValue& other) {
        if (this != &other) {
            clear();
            type_ = other.type_;
            copyFrom(other);
        }
        return *this;
    }

    JsonValue& operator=(JsonValue&& other) noexcept {
        if (this != &other) {
            clear();
            type_ = other.type_;
            moveFrom(std::move(other));
        }
        return *this;
    }

    // 类型查询
    Type type() const { return type_; }
    bool isNull() const { return type_ == Null; }
    bool isBool() const { return type_ == Bool; }
    bool isNumber() const { return type_ == Number; }
    bool isString() const { return type_ == String; }
    bool isArray() const { return type_ == Array; }
    bool isObject() const { return type_ == Object; }

    // 值获取（带默认值）
    bool toBool(bool defaultValue = false) const {
        if (isBool()) return bool_value;
        return defaultValue;
    }

    int toInt(int defaultValue = 0) const {
        if (isNumber()) return static_cast<int>(number_value);
        return defaultValue;
    }

    long long toLongLong(long long defaultValue = 0) const {
        if (isNumber()) return static_cast<long long>(number_value);
        return defaultValue;
    }

    double toDouble(double defaultValue = 0.0) const {
        if (isNumber()) return number_value;
        return defaultValue;
    }

    std::string toString(const std::string& defaultValue = "") const {
        if (isString()) return *string_value;
        return defaultValue;
    }

    const ArrayType& toArray() const {
        if (isArray()) return *array_value;
        throw std::runtime_error("JsonValue is not an array");
    }

    ArrayType& toArray() {
        if (isArray()) return *array_value;
        throw std::runtime_error("JsonValue is not an array");
    }

    const ObjectType& toObject() const {
        if (isObject()) return *object_value;
        throw std::runtime_error("JsonValue is not an object");
    }

    ObjectType& toObject() {
        if (isObject()) return *object_value;
        throw std::runtime_error("JsonValue is not an object");
    }

    // 数组操作
    void append(const JsonValue& value) {
        if (!isArray()) {
            clear();
            type_ = Array;
            array_value = new ArrayType();
        }
        array_value->push_back(value);
    }

    JsonValue& operator[](size_t index) {
        if (!isArray()) {
            clear();
            type_ = Array;
            array_value = new ArrayType();
        }
        auto& arr = *array_value;
        if (index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }

    const JsonValue& operator[](size_t index) const {
        static JsonValue nullValue;
        if (!isArray()) return nullValue;
        const auto& arr = *array_value;
        if (index >= arr.size()) return nullValue;
        return arr[index];
    }

    // 对象操作
    JsonValue& operator[](const std::string& key) {
        if (!isObject()) {
            clear();
            type_ = Object;
            object_value = new ObjectType();
        }
        return (*object_value)[key];
    }

    const JsonValue& operator[](const std::string& key) const {
        static JsonValue nullValue;
        if (!isObject()) return nullValue;
        const auto& obj = *object_value;
        auto it = obj.find(key);
        return it != obj.end() ? it->second : nullValue;
    }

    bool contains(const std::string& key) const {
        if (!isObject()) return false;
        const auto& obj = *object_value;
        return obj.find(key) != obj.end();
    }

    // 大小
    size_t size() const {
        if (isArray()) return array_value->size();
        if (isObject()) return object_value->size();
        return 0;
    }

    // 序列化为字符串
    std::string dump(int indent = -1) const {
        std::ostringstream oss;
        dumpImpl(oss, indent, 0);
        return oss.str();
    }

    // 从字符串解析
    static JsonValue parse(const std::string& str) {
        size_t pos = 0;
        return parseValue(str, pos);
    }

    // 比较操作
    bool operator==(const JsonValue& other) const {
        if (type_ != other.type_) return false;
        
        switch (type_) {
            case Null:
                return true;
            case Bool:
                return bool_value == other.bool_value;
            case Number:
                return number_value == other.number_value;
            case String:
                return *string_value == *other.string_value;
            case Array:
                return *array_value == *other.array_value;
            case Object:
                return *object_value == *other.object_value;
        }
        return false;
    }

    bool operator!=(const JsonValue& other) const {
        return !(*this == other);
    }

private:
    void dumpImpl(std::ostream& os, int indent, int currentIndent) const;
    static std::string escapeString(const std::string& str);
    static JsonValue parseValue(const std::string& str, size_t& pos);
    static void skipWhitespace(const std::string& str, size_t& pos);
    static JsonValue parseNull(const std::string& str, size_t& pos);
    static JsonValue parseBool(const std::string& str, size_t& pos);
    static JsonValue parseNumber(const std::string& str, size_t& pos);
    static JsonValue parseString(const std::string& str, size_t& pos);
    static JsonValue parseArray(const std::string& str, size_t& pos);
    static JsonValue parseObject(const std::string& str, size_t& pos);
};

// 实现内联函数
inline void JsonValue::clear() {
    switch (type_) {
        case String:
            delete string_value;
            break;
        case Array:
            delete array_value;
            break;
        case Object:
            delete object_value;
            break;
        default:
            break;
    }
    type_ = Null;
}

inline void JsonValue::copyFrom(const JsonValue& other) {
    switch (other.type_) {
        case Null:
            break;
        case Bool:
            bool_value = other.bool_value;
            break;
        case Number:
            number_value = other.number_value;
            break;
        case String:
            string_value = new std::string(*other.string_value);
            break;
        case Array:
            array_value = new ArrayType(*other.array_value);
            break;
        case Object:
            object_value = new ObjectType(*other.object_value);
            break;
    }
}

inline void JsonValue::moveFrom(JsonValue&& other) noexcept {
    switch (other.type_) {
        case Null:
            break;
        case Bool:
            bool_value = other.bool_value;
            break;
        case Number:
            number_value = other.number_value;
            break;
        case String:
            string_value = other.string_value;
            other.string_value = nullptr;
            break;
        case Array:
            array_value = other.array_value;
            other.array_value = nullptr;
            break;
        case Object:
            object_value = other.object_value;
            other.object_value = nullptr;
            break;
    }
    other.type_ = Null;
}

// 便利别名
using JsonObject = JsonValue::ObjectType;
using JsonArray = JsonValue::ArrayType;

} // namespace JsonStruct
