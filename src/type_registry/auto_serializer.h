#pragma once
#include "../json_engine/json_value.h"
#include "registry_core.h"
#include "field_macros.h"
#include <type_traits>
#include <tuple>
#include <sstream>
#include <vector>

// JSON_FIELDS 宏现在在 field_macros.h 中定义

template<typename T>
constexpr bool is_json_primitive_v =
    std::is_arithmetic_v<T> ||
    std::is_same_v<T, std::string>;

// 检查类型是否有json_fields方法
template<typename T>
struct has_json_fields {
    template<typename U>
    static auto test(int) -> decltype(std::declval<const U&>().json_fields(), std::true_type{});
    
    template<typename>
    static std::false_type test(...);
    
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
constexpr bool has_json_fields_v = has_json_fields<T>::value;

template<typename T>
JsonStruct::JsonValue toJsonValue(const T& value) {
    // 首先检查是否在类型注册表中
    if (JsonStruct::TypeRegistry::instance().isRegistered<T>()) {
        return JsonStruct::TypeRegistry::instance().toJson(value);
    }
    
    // 基础类型处理
    if constexpr (std::is_same_v<T, bool>) {
        return JsonStruct::JsonValue(value);
    }
    else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        return JsonStruct::JsonValue(static_cast<long long>(value));
    }
    else if constexpr (std::is_floating_point_v<T>) {
        return JsonStruct::JsonValue(static_cast<double>(value));
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        return JsonStruct::JsonValue(value);
    }
    else if constexpr (std::is_enum_v<T>) {
        return JsonStruct::JsonValue(static_cast<int>(value));
    }
    else if constexpr (has_json_fields_v<T>) {
        return toJson(value);
    }
    else {
        return JsonStruct::JsonValue();
    }
}

// Simplified version, handle simple cases first
template<typename T>
typename std::enable_if<has_json_fields_v<T>, JsonStruct::JsonObject>::type
toJson(const T& obj) {
    JsonStruct::JsonObject json;
    auto names_str = std::string(T::json_field_names());
    auto names_vec = split_names(names_str);
    auto fields = obj.json_fields();
    
    // Simplified version - handle common field counts
    if (names_vec.size() >= 1 && names_vec.size() <= 4) {
        if (names_vec.size() >= 1) {
            setJsonField(json, names_vec[0], std::get<0>(fields));
        }
        if (names_vec.size() >= 2) {
            setJsonField(json, names_vec[1], std::get<1>(fields));
        }
        if (names_vec.size() >= 3) {
            setJsonField(json, names_vec[2], std::get<2>(fields));
        }
        if (names_vec.size() >= 4) {
            setJsonField(json, names_vec[3], std::get<3>(fields));
        }
    }
    
    return json;
}

template<typename T>
typename std::enable_if<!is_json_primitive_v<T>, T>::type
fromJsonValue(const JsonStruct::JsonValue& value, const T& defaultValue) {
    if (value.isNull()) {
        return defaultValue;
    }
    
    // 首先检查是否在类型注册表中
    if (JsonStruct::TypeRegistry::instance().isRegistered<T>()) {
        return JsonStruct::TypeRegistry::instance().fromJson(value, defaultValue);
    }
    
    if constexpr (std::is_enum_v<T>) {
        return static_cast<T>(value.toInt(static_cast<int>(defaultValue)));
    }
    else if constexpr (has_json_fields_v<T>) {
        T obj = defaultValue;
        if (value.isObject()) {
            fromJson(obj, value.toObject());
        }
        return obj;
    }
    else {
        return defaultValue;
    }
}

// Basic type overloads - use overloads instead of specializations
inline bool fromJsonValue(const JsonStruct::JsonValue& value, const bool& defaultValue) {
    return value.toBool(defaultValue);
}

inline int fromJsonValue(const JsonStruct::JsonValue& value, const int& defaultValue) {
    return value.toInt(defaultValue);
}

inline long long fromJsonValue(const JsonStruct::JsonValue& value, const long long& defaultValue) {
    return value.toLongLong(defaultValue);
}

inline double fromJsonValue(const JsonStruct::JsonValue& value, const double& defaultValue) {
    return value.toDouble(defaultValue);
}

inline float fromJsonValue(const JsonStruct::JsonValue& value, const float& defaultValue) {
    return static_cast<float>(value.toDouble(defaultValue));
}

inline std::string fromJsonValue(const JsonStruct::JsonValue& value, const std::string& defaultValue) {
    return value.toString(defaultValue);
}

template<typename T>
void fromJson(T& obj, const JsonStruct::JsonObject& json) {
    if constexpr (has_json_fields_v<T>) {
        auto names_str = std::string(T::json_field_names());
        auto names_vec = split_names(names_str);
        auto fields = obj.json_fields();
        
        // Simplified version - handle common field counts
        if (names_vec.size() >= 1 && names_vec.size() <= 4) {
            if (names_vec.size() >= 1) {
                std::get<0>(fields) = getJsonField(json, names_vec[0], std::get<0>(fields));
            }
            if (names_vec.size() >= 2) {
                std::get<1>(fields) = getJsonField(json, names_vec[1], std::get<1>(fields));
            }
            if (names_vec.size() >= 3) {
                std::get<2>(fields) = getJsonField(json, names_vec[2], std::get<2>(fields));
            }
            if (names_vec.size() >= 4) {
                std::get<3>(fields) = getJsonField(json, names_vec[3], std::get<3>(fields));
            }
        }
    }
}

template<typename T>
T getJsonField(const JsonStruct::JsonObject& json, const std::string& key, const T& defaultValue = T{}) {
    auto it = json.find(key);
    if (it == json.end()) {
        return defaultValue;
    }
    return fromJsonValue(it->second, defaultValue);
}

template<typename T>
void setJsonField(JsonStruct::JsonObject& json, const std::string& key, const T& value) {
    json[key] = toJsonValue(value);
}

#define JSON_AUTO(...) \
JSON_FIELDS(__VA_ARGS__) \
    JsonStruct::JsonObject toJson() const { \
        return ::toJson(*this); \
} \
    void fromJson(const JsonStruct::JsonObject& json) { \
        ::fromJson(*this, json); \
}
