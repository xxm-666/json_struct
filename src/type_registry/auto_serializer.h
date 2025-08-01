#pragma once
#include "../json_engine/json_value.h"
#include "registry_core.h"
#include "field_macros.h"
#include "../std_types/std_registry.h"
#include <type_traits>
#include <tuple>
#include <sstream>
#include <vector>

#ifdef QT_CORE_LIB
// Include the Qt ultimate registry for type detection
#include "../qt_types/qt_ultimate_registry.h"
// Forward declarations to avoid circular dependency
namespace JsonStruct {
    namespace QtUniversal {
        template<typename T> JsonValue ultimateToJson(const T& value);
        template<typename T> T ultimateFromJson(const JsonValue& json, const T& defaultValue);
    }
}
#endif

template<typename T>
constexpr bool is_json_primitive_v =
    std::is_arithmetic_v<T> ||
    std::is_same_v<T, std::string>;

// Chjeck if a type has a json_fields method
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

// Forward declarations
template<typename T> T getJsonField(const JsonStruct::JsonValue::ObjectType& json, const std::string& key, const T& defaultValue = T{});
template<typename T> void setJsonField(JsonStruct::JsonValue::ObjectType& json, const std::string& key, const T& value);

template<typename T>
JsonStruct::JsonValue toJsonValue(const T& value) {
    // First check if the type is registered in the type registry
    if (JsonStruct::TypeRegistry::instance().isRegistered<T>()) {
        return JsonStruct::TypeRegistry::instance().toJson(value);
    }
    
    // Basic type handling
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
        return JsonStruct::JsonValue(toJson(value));
    }
    // Common container support - supports any nested container type
    else if constexpr (JsonStruct::is_container_v<T>) {
        return JsonStruct::containerToJson(value);
    }
    // pair support
    else if constexpr (JsonStruct::is_pair<T>::value) {
        return JsonStruct::pairToJson(value);
    }
#ifdef QT_CORE_LIB
    // Qt type support - use Qt universal system as primary fallback
    else {
        try {
            // Try to use Qt ultimate system for Qt types
            return JsonStruct::QtUniversal::ultimateToJson(value);
        } catch (...) {
            // If Qt system fails, try existing registry
            try {
                return JsonStruct::TypeRegistry::instance().toJson(value);
            } catch (...) {
                return JsonStruct::JsonValue();
            }
        }
    }
#else
    else {
        return JsonStruct::JsonValue();
    }
#endif
}

// For supporting recursive calls, we need to define genericToJsonValue
template<typename T>
JsonStruct::JsonValue JsonStruct::genericToJsonValue(const T& value) {
    return ::toJsonValue(value);
}

// Helper to get tuple size using SFINAE
template<typename T>
struct tuple_size_helper {
    using type = decltype(std::declval<T>().json_fields());
    static constexpr std::size_t value = std::tuple_size_v<type>;
};

// Helper for serialization using index sequence
template<typename T, std::size_t... I>
JsonStruct::JsonValue::ObjectType toJsonImpl(const T& obj, std::index_sequence<I...>) {
    JsonStruct::JsonValue::ObjectType json;
    auto names_vec = T::get_field_names();
    auto fields = obj.json_fields();
    
    // Use fold expression (C++17) to set all fields
    ((setJsonField(json, names_vec[I], std::get<I>(fields))), ...);
    
    return json;
}

// Main serialization function
template<typename T>
typename std::enable_if<has_json_fields_v<T>, JsonStruct::JsonValue::ObjectType>::type
toJson(const T& obj) {
    constexpr auto size = tuple_size_helper<T>::value;
    return toJsonImpl(obj, std::make_index_sequence<size>{});
}

// Helper for deserialization using index sequence
template<typename T, std::size_t... I>
void fromJsonImpl(T& obj, const JsonStruct::JsonValue::ObjectType& json, std::index_sequence<I...>) {
    auto names_vec = T::get_field_names();
    auto fields = obj.json_fields();
    
    // Use fold expression to deserialize all fields
    ((std::get<I>(fields) = getJsonField(json, names_vec[I], std::get<I>(fields))), ...);
}

// Main deserialization function
template<typename T>
void fromJson(T& obj, const JsonStruct::JsonValue& json) {
    if constexpr (has_json_fields_v<T>) {
        if (json.isObject()) {
            constexpr auto size = tuple_size_helper<T>::value;
            if(const auto& objOpt = json.toObject()) {
                // Deserialize using index sequence
                fromJsonImpl(obj, objOpt->get(), std::make_index_sequence<size>{});
            }
        }
    }
}

template<typename T>
T fromJson(const JsonStruct::JsonValue& json) {
    T obj;
    if (json.isObject()) {
        fromJson(obj, json);
    }
    return obj;
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

inline float fromJsonValue(const JsonStruct::JsonValue& value, const float& defaultValue) {
    return static_cast<float>(value.toDouble(defaultValue));
}

inline double fromJsonValue(const JsonStruct::JsonValue& value, const double& defaultValue) {
    return value.toDouble(defaultValue);
}

inline std::string fromJsonValue(const JsonStruct::JsonValue& value, const std::string& defaultValue) {
    return value.toString(defaultValue);
}

template<typename T>
typename std::enable_if<!is_json_primitive_v<T>, T>::type
fromJsonValue(const JsonStruct::JsonValue& value, const T& defaultValue) {
    if (value.isNull()) {
        return defaultValue;
    }
    
    // First check if the type is registered in the type registry
    if (JsonStruct::TypeRegistry::instance().isRegistered<T>()) {
        return JsonStruct::TypeRegistry::instance().fromJson(value, defaultValue);
    }
    
    if constexpr (std::is_enum_v<T>) {
        return static_cast<T>(value.toInt(static_cast<int>(defaultValue)));
    }
    else if constexpr (has_json_fields_v<T>) {
        T obj = defaultValue;
        if (value.isObject()) {
            fromJson(obj, value);
        }
        return obj;
    }
    // Common container support - supports any nested container type
    else if constexpr (JsonStruct::is_container_v<T>) {
        return JsonStruct::containerFromJson(value, defaultValue);
    }
    else if constexpr (JsonStruct::is_pair<T>::value) {
        return JsonStruct::pairFromJson(value, defaultValue);
    }
#ifdef QT_CORE_LIB
    // Qt type support - use Qt universal system as primary fallback
    else {
        try {
            // Try to use Qt ultimate system for Qt types
            return JsonStruct::QtUniversal::template ultimateFromJson<T>(value, defaultValue);
        } catch (...) {
            // If Qt system fails, try existing registry
            try {
                return JsonStruct::TypeRegistry::instance().fromJson<T>(value, defaultValue);
            } catch (...) {
                return defaultValue;
            }
        }
    }
#else
    else {
        return defaultValue;
    }
#endif
}

// Additional overloads for specific integer types to avoid ambiguity
inline char fromJsonValue(const JsonStruct::JsonValue& value, const char& defaultValue) {
    return static_cast<char>(value.toInt(defaultValue));
}

inline signed char fromJsonValue(const JsonStruct::JsonValue& value, const signed char& defaultValue) {
    return static_cast<signed char>(value.toInt(defaultValue));
}

inline unsigned char fromJsonValue(const JsonStruct::JsonValue& value, const unsigned char& defaultValue) {
    return static_cast<unsigned char>(value.toInt(defaultValue));
}

inline short fromJsonValue(const JsonStruct::JsonValue& value, const short& defaultValue) {
    return static_cast<short>(value.toInt(defaultValue));
}

inline unsigned short fromJsonValue(const JsonStruct::JsonValue& value, const unsigned short& defaultValue) {
    return static_cast<unsigned short>(value.toInt(defaultValue));
}

inline unsigned int fromJsonValue(const JsonStruct::JsonValue& value, const unsigned int& defaultValue) {
    return static_cast<unsigned int>(value.toInt(defaultValue));
}

inline long fromJsonValue(const JsonStruct::JsonValue& value, const long& defaultValue) {
    return static_cast<long>(value.toLongLong(defaultValue));
}

inline unsigned long fromJsonValue(const JsonStruct::JsonValue& value, const unsigned long& defaultValue) {
    return static_cast<unsigned long>(value.toLongLong(defaultValue));
}

inline unsigned long long fromJsonValue(const JsonStruct::JsonValue& value, const unsigned long long& defaultValue) {
    return static_cast<unsigned long long>(value.toLongLong(defaultValue));
}

template<typename T>
T JsonStruct::genericFromJsonValue(const JsonStruct::JsonValue& json, const T& defaultValue) {
    return ::fromJsonValue(json, defaultValue);
}

// Helper functions for field operations
template<typename T>
T getJsonField(const JsonStruct::JsonValue::ObjectType& json, const std::string& key, const T& defaultValue) {
    auto it = json.find(key);
    if (it == json.end()) {
        return defaultValue;
    }
    return fromJsonValue(it->second, defaultValue);
}

template<typename T>
void setJsonField(JsonStruct::JsonValue::ObjectType& json, const std::string& key, const T& value) {
    json[key] = toJsonValue(value);
}

// Improved JSON_AUTO macro with proper method definitions
#define JSON_AUTO(...) \
JSON_FIELDS(__VA_ARGS__) \
public: \
    JsonStruct::JsonValue toJson() const { \
        return JsonStruct::JsonValue(::toJson(*this)); \
    } \
    void fromJson(const JsonStruct::JsonValue& json) { \
        ::fromJson(*this, json); \
    }
