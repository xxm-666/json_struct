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
#include <functional>
#include "json_number.h"

namespace JsonStruct {

// Forward declarations
class JsonFilter;

class JsonValue {
public:
    enum class Type {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    using ArrayType = std::vector<JsonValue>;
    using ObjectType = std::unordered_map<std::string, JsonValue>;
    
    // Uses std::variant for type safety, supports more C++17 features
    using ValueType = std::variant<
        std::monostate,  // Null
        bool,            // Bool
        JsonNumber,      // Number (new high-precision number type)
        std::string,     // String
        ArrayType,       // Array
        ObjectType       // Object
    >;

    // Parse options
    struct ParseOptions{
        size_t maxDepth = 512;          // Maximum nesting depth
        bool allowComments = false;     // Allow comments (JSON5 style)
        bool allowTrailingCommas = false; // Allow trailing commas
        bool strictMode = true;         // Strict mode
        bool validateUtf8 = true;       // Validate UTF-8 encoding
        bool allowSpecialNumbers = false; // Allow NaN/Infinity and other special numbers
        bool allowRecovery = false;     // Allow error recovery (lenient parsing)

        ParseOptions()
        : maxDepth(512), allowComments(false),
          allowTrailingCommas(false), strictMode(true),
          validateUtf8(true), allowSpecialNumbers(false),
          allowRecovery(false) {}
    };

    // Serialization options
    struct SerializeOptions {
        int indent = -1;                // Indentation level, -1 for compact mode
        bool sortKeys = false;          // Sort object keys
        bool escapeUnicode = false;     // Escape Unicode characters
        bool compactArrays = false;     // Compact array formatting
        size_t maxPrecision = 15;       // Floating point precision
        bool allowSpecialNumbers = false; // Serialize special numbers

        SerializeOptions()
        : indent(-1), sortKeys(false),
          escapeUnicode(false), compactArrays(false),
          maxPrecision(15), allowSpecialNumbers(false) {}
    };

private:
    ValueType value_;
    
    // Parse context, provides detailed error information
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
    // Default constructor - constructs null
    JsonValue() : value_(std::monostate{}) {}
    
    // Basic type constructors
    explicit JsonValue(std::nullptr_t) : value_(std::monostate{}) {}
    explicit JsonValue(bool b) : value_(b) {}
    explicit JsonValue(int i) : value_(JsonNumber(static_cast<int64_t>(i))) {}
    explicit JsonValue(long long ll) : value_(JsonNumber(ll)) {}  // No more precision loss
    explicit JsonValue(float f) : value_(JsonNumber(static_cast<double>(f))) {}
    explicit JsonValue(double d) : value_(JsonNumber(d)) {}
    explicit JsonValue(const JsonNumber& num) : value_(num) {}  // Support direct construction from JsonNumber
    explicit JsonValue(JsonNumber&& num) : value_(std::move(num)) {}  // Support move semantics
    explicit JsonValue(std::string s) : value_(std::move(s)) {}
    explicit JsonValue(std::string_view sv) : value_(std::string(sv)) {}
    explicit JsonValue(const char* str) : value_(std::string(str)) {}
    explicit JsonValue(ArrayType arr) : value_(std::move(arr)) {}
    explicit JsonValue(ObjectType obj) : value_(std::move(obj)) {}

    // Container constructors - support for more standard containers
    template<typename T, size_t N>
    explicit JsonValue(const std::array<T, N>& arr) {
        ArrayType jsonArr;
        jsonArr.reserve(N);
        for (const auto& item : arr) {
            jsonArr.emplace_back(makeValue(item));
        }
        value_ = std::move(jsonArr);
    }
    
    template<typename T>
    explicit JsonValue(const std::vector<T>& vec) {
        ArrayType jsonArr;
        jsonArr.reserve(vec.size());
        for (const auto& item : vec) {
            jsonArr.emplace_back(makeValue(item));
        }
        value_ = std::move(jsonArr);
    }
    
    template<typename T>
    explicit JsonValue(const std::set<T>& s) {
        ArrayType jsonArr;
        jsonArr.reserve(s.size());
        for (const auto& item : s) {
            jsonArr.emplace_back(makeValue(item));
        }
        value_ = std::move(jsonArr);
    }
    
    template<typename K, typename V>
    explicit JsonValue(const std::map<K, V>& m) {
        ObjectType jsonObj;
        for (const auto& [key, val] : m) {
            jsonObj.emplace(toString(key), makeValue(val));
        }
        value_ = std::move(jsonObj);
    }

    // Move/copy constructors and assignment
    JsonValue(JsonValue&&) = default;
    JsonValue& operator=(JsonValue&&) = default;
    
    // Copy constructor and assignment
    JsonValue(const JsonValue&) = default;
    JsonValue& operator=(const JsonValue&) = default;

    // Destructor
    ~JsonValue() = default;

    // Type queries
    Type type() const noexcept {
        return static_cast<Type>(value_.index());
    }
    
    constexpr bool isNull() const noexcept { return std::holds_alternative<std::monostate>(value_); }
    constexpr bool isBool() const noexcept { return std::holds_alternative<bool>(value_); }
    constexpr bool isNumber() const noexcept { return std::holds_alternative<JsonNumber>(value_); }
    constexpr bool isString() const noexcept { return std::holds_alternative<std::string>(value_); }
    constexpr bool isArray() const noexcept { return std::holds_alternative<ArrayType>(value_); }
    constexpr bool isObject() const noexcept { return std::holds_alternative<ObjectType>(value_); }

    // New: fine-grained number type queries
    constexpr bool isInteger() const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->isInteger();
        }
        return false;
    }
    
    constexpr bool isDouble() const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->isDouble();
        }
        return false;
    }
    
    // IEEE 754 special value queries
    bool isNaN() const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->isNaN();
        }
        return false;
    }
    
    bool isInfinity() const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->isInfinity();
        }
        return false;
    }
    
    bool isFinite() const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->isFinite();
        }
        return false;
    }

    // Safe value access - uses std::optional
    std::optional<bool> getBool() const noexcept {
        if (auto* val = std::get_if<bool>(&value_)) {
            return *val;
        }
        return std::nullopt;
    }
    
    std::optional<double> getNumber() const noexcept {
        if (auto* val = std::get_if<JsonNumber>(&value_)) {
            return val->getDouble();
        }
        return std::nullopt;
    }
    
    // New: safe integer access
    std::optional<int64_t> getInteger() const noexcept {
        if (auto* val = std::get_if<JsonNumber>(&value_)) {
            return val->getInteger();
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

    // Get with default value
    bool toBool(bool defaultValue = false) const noexcept {
        return getBool().value_or(defaultValue);
    }

    int toInt(int defaultValue = 0) const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->toInt32(defaultValue);
        }
        return defaultValue;
    }

    long long toLongLong(long long defaultValue = 0) const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->toInt64(defaultValue);
        }
        return defaultValue;
    }

    double toDouble(double defaultValue = 0.0) const noexcept {
        if (auto* num = std::get_if<JsonNumber>(&value_)) {
            return num->toDouble();
        }
        return defaultValue;
    }

    std::string toString(const std::string& defaultValue = "") const {
        auto str_opt = getString();
        if (str_opt.has_value()) {
            return std::string(str_opt.value());
        }
        return defaultValue;
    }
    
    std::string toString(const char* defaultValue) const {
        return toString(std::string(defaultValue));
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

    // Array operations
    void append(JsonValue value) {
        if (!isArray()) {
            value_ = ArrayType{};
        }
        std::get<ArrayType>(value_).emplace_back(std::move(value));
    }
    
    template<typename T>
    void append(T&& value) {
        append(makeValue(std::forward<T>(value)));
    }

    JsonValue& operator[](size_t index) {
        if (!isArray()) {
            value_ = ArrayType{};
        }
        auto& arr = std::get<ArrayType>(value_);
        if (index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }

    const JsonValue& operator[](size_t index) const {
        static const JsonValue nullValue;
        if (auto* arr = getArray()) {
            if (index < arr->size()) {
                return (*arr)[index];
            }
        }
        return nullValue;
    }

    // Object operations
    JsonValue& operator[](std::string_view key) {
        if (!isObject()) {
            value_ = ObjectType{};
        }

        return std::get<ObjectType>(value_)[std::string(key)];
    }

    const JsonValue& operator[](std::string_view key) const {
        static const JsonValue nullValue;
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

    // Size and empty checks
    size_t size() const noexcept {
        if (auto* arr = getArray()) return arr->size();
        if (auto* obj = getObject()) return obj->size();
        return 0;
    }
    
    bool empty() const noexcept {
        return size() == 0;
    }

    // Serialization
    std::string dump(const SerializeOptions& options = {}) const {
        std::ostringstream oss;
        dumpImpl(oss, options, 0);
        return oss.str();
    }
    
    // Compatibility overload: accepts indent parameter
    std::string dump(int indent) const {
        SerializeOptions options;
        options.indent = indent;
        return dump(options);
    }

    // Parsing
    static JsonValue parse(std::string_view str, const ParseOptions& options = {}) {
        ParseContext ctx{str, 0, 1, 1, 0, options};
        auto result = parseValue(ctx);
        skipWhitespace(ctx);
        if (ctx.hasMore()) {
            throw std::runtime_error("Extra characters after JSON at " + ctx.locationInfo());
        }
        return result;
    }

    // Static factory methods for convenience
    static JsonValue object() {
        return JsonValue(ObjectType{});
    }
    
    static JsonValue object(std::initializer_list<std::pair<const std::string, JsonValue>> init) {
        ObjectType obj;
        for (auto&& pair : init) {
            obj.emplace(pair.first, pair.second);
        }
        return JsonValue(std::move(obj));
    }
    
    static JsonValue array() {
        return JsonValue(ArrayType{});
    }
    
    static JsonValue array(std::initializer_list<JsonValue> init = {}) {
        ArrayType arr;
        arr.reserve(init.size());
        for (auto&& item : init) {
            arr.emplace_back(item);
        }
        return JsonValue(std::move(arr));
    }

    // Alias for dump() for compatibility
    std::string toJson(const SerializeOptions& options = {}) const {
        return dump(options);
    }
    
    // Compatibility overload with boolean pretty print parameter
    std::string toJson(bool pretty) const {
        SerializeOptions options;
        options.indent = pretty ? 2 : -1;
        return dump(options);
    }
    
    // Comparison operators
    bool operator==(const JsonValue& other) const noexcept {
        return value_ == other.value_;
    }

    bool operator!=(const JsonValue& other) const noexcept {
        return !(*this == other);
    }

    // Visitor pattern support
    template<typename Visitor>
    constexpr auto visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }

    template<typename Visitor>
    constexpr auto visit(Visitor&& visitor) {
        return std::visit(std::forward<Visitor>(visitor), value_);
    }

    // JSON Pointer support (RFC 6901)
    JsonValue& at(std::string_view jsonPointer);
    const JsonValue& at(std::string_view jsonPointer) const;

    // JSON Query support - delegated to JsonFilter for better separation of concerns
    // Note: For advanced filtering and querying, use JsonFilter class directly
    // These methods are provided for backward compatibility and convenience
    bool pathExists(const std::string& jsonpath_expression) const;
    const JsonValue* selectFirst(const std::string& jsonpath_expression) const;
    std::vector<const JsonValue*> selectAll(const std::string& jsonpath_expression) const;
    std::vector<JsonValue> selectValues(const std::string& jsonpath_expression) const;

    // === Streaming and Lazy Query Support ===
    // Note: For full streaming functionality, use JsonStreamingQuery class directly
    // These methods provide convenient access for backward compatibility
    
    /**
     * @brief Find first matching result efficiently (early termination)
     * @param expression JSONPath expression to evaluate
     * @return Pair of (pointer to value, path) or nullopt if not found
     */
    std::optional<std::pair<const JsonValue*, std::string>> findFirst(const std::string& expression) const;
    
    /**
     * @brief Count matching results without materializing them
     * @param expression JSONPath expression to evaluate
     * @param maxCount Maximum count to check (0 = unlimited)
     * @return Number of matching results
     */
    size_t countMatches(const std::string& expression, size_t maxCount = 0) const;
    
    // Static helper functions
    template<typename T>
    static std::string toString(const T& value) {
        if constexpr (std::is_convertible_v<T, std::string>) {
            return std::string(value);
        } else {
            return std::to_string(value);
        }
    }

private:
    // Helper functions
    template<typename T>
    static JsonValue makeValue(T&& value) {
        if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
            return JsonValue(value);
        } else if constexpr (std::is_integral_v<std::decay_t<T>>) {
            return JsonValue(static_cast<long long>(value));  // Use long long to maintain precision
        } else if constexpr (std::is_floating_point_v<std::decay_t<T>>) {
            return JsonValue(static_cast<double>(value));
        } else if constexpr (std::is_convertible_v<T, std::string>) {
            return JsonValue(std::string(std::forward<T>(value)));
        } else {
            return JsonValue(std::forward<T>(value));
        }
    }
    
    // Serialization implementation
    void dumpImpl(std::ostream& os, const SerializeOptions& options, int currentIndent) const;
    static std::string escapeString(std::string_view str, bool escapeUnicode = false);
    
    // Parsing implementation
    static JsonValue parseValue(ParseContext& ctx);
    static void skipWhitespace(ParseContext& ctx);
    static JsonValue parseNull(ParseContext& ctx);
    static JsonValue parseBool(ParseContext& ctx);
    static JsonValue parseNumber(ParseContext& ctx);
    static JsonValue parseSpecialNumber(ParseContext& ctx);  // New: parse special numbers
    static JsonValue parseString(ParseContext& ctx);
    static JsonValue parseArray(ParseContext& ctx);
    static JsonValue parseObject(ParseContext& ctx);
    
    // Enhanced error recovery parsing functions
    static JsonValue parseArrayWithRecovery(ParseContext& ctx);
    static JsonValue parseObjectWithRecovery(ParseContext& ctx);
    
    static std::string parseUnicodeEscape(std::string_view str, size_t& pos);
    static bool isValidUtf8(std::string_view str);
    
    // JSON Pointer implementation
    static std::vector<std::string> parseJsonPointer(std::string_view pointer);
    static std::string unescapeJsonPointer(std::string_view token);
};

// Factory and convenience functions
template<typename T>
JsonValue makeJson(T&& value) {
    return JsonValue(std::forward<T>(value));
}

// Literal operator (C++14+)
namespace literals {
    JsonValue operator""_json(const char* str, size_t len);
}

// Type aliases for compatibility
using JsonObject = JsonValue::ObjectType;
using JsonArray = JsonValue::ArrayType;

// Stream operators
std::ostream& operator<<(std::ostream& os, const JsonValue& value);
std::istream& operator>>(std::istream& is, JsonValue& value);

} // namespace JsonStruct
