#pragma once
#include "qt_common.h"

namespace JsonStruct {

namespace QtUniversal {

// Forward declarations for recursive calls
template<typename T>
JsonValue ultimateToJson(const T& value);

template<typename T>
T ultimateFromJson(const JsonValue& json, const T& defaultValue = T{});

// Universal Qt to JSON conversion
template<typename T>
JsonValue ultimateToJson(const T& value) {
    if constexpr (std::is_same_v<T, QString>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (std::is_same_v<T, QStringList>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (std::is_same_v<T, QPointF>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (std::is_same_v<T, QRectF>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (std::is_same_v<T, QRect>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (std::is_same_v<T, QColor>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (std::is_same_v<T, QSize>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (std::is_same_v<T, QSizeF>) {
        return QtBasic::qtBasicToJson(value);
    }
    else if constexpr (is_qt_sequence_container_v<T>) {
        JsonValue::ArrayType arr;
        for (const auto& item : value) {
            arr.push_back(ultimateToJson(item));
        }
        return JsonValue(arr);
    }
    else if constexpr (is_qt_map_container_v<T>) {
        JsonValue::ObjectType obj;
        for (auto it = value.begin(); it != value.end(); ++it) {
            QString keyStr;
            if constexpr (std::is_same_v<typename T::key_type, QString>) {
                keyStr = it.key();
            } else if constexpr (std::is_arithmetic_v<typename T::key_type>) {
                keyStr = QString::number(it.key());
            } else {
                // For complex keys, use their string representation
                keyStr = QString("key_%1").arg(reinterpret_cast<quintptr>(&it.key()));
            }
            obj[keyStr.toStdString()] = ultimateToJson(it.value());
        }
        return JsonValue(obj);
    }
    else if constexpr (is_qt_associative_container_v<T>) {
        JsonValue::ArrayType arr;
        for (const auto& item : value) {
            arr.push_back(ultimateToJson(item));
        }
        return JsonValue(arr);
    }
    else if constexpr (is_qt_pair<T>::value) {
        JsonValue::ArrayType arr;
        arr.push_back(ultimateToJson(value.first));
        arr.push_back(ultimateToJson(value.second));
        return JsonValue(arr);
    }
    else if constexpr (std::is_arithmetic_v<T>) {
        if constexpr (std::is_integral_v<T>) {
            return JsonValue(static_cast<int64_t>(value));
        } else {
            return JsonValue(static_cast<double>(value));
        }
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        return JsonValue(value);
    }
    else {
        // Try to use the existing type registry for custom types
        try {
            return TypeRegistry::instance().toJson(value);
        } catch (...) {
            return JsonValue();
        }
    }
}

// Universal Qt from JSON conversion
template<typename T>
T ultimateFromJson(const JsonValue& json, const T& defaultValue) {
    if constexpr (std::is_same_v<T, QString>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (std::is_same_v<T, QStringList>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (std::is_same_v<T, QPointF>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (std::is_same_v<T, QRectF>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (std::is_same_v<T, QRect>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (std::is_same_v<T, QColor>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (std::is_same_v<T, QSize>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (std::is_same_v<T, QSizeF>) {
        return QtBasic::qtBasicFromJson(json, defaultValue);
    }
    else if constexpr (is_qt_sequence_container_v<T>) {
        if (!json.isArray()) {
            return defaultValue;
        }
        
        T result;
        const auto* arr = json.getArray();
        if (!arr) return defaultValue;
        
        using ValueType = typename T::value_type;
        
        for (const auto& item : *arr) {
            ValueType val = ultimateFromJson<ValueType>(item, ValueType{});
            result.append(val);
        }
        return result;
    }
    else if constexpr (is_qt_map_container_v<T>) {
        if (!json.isObject()) {
            return defaultValue;
        }
        
        T result;
        const auto* obj = json.getObject();
        if (!obj) return defaultValue;
        
        using KeyType = typename T::key_type;
        using ValueType = typename T::mapped_type;
        
        for (const auto& pair : *obj) {
            const std::string& keyStr = pair.first;
            const JsonValue& valueJson = pair.second;
            
            KeyType key;
            if constexpr (std::is_same_v<KeyType, QString>) {
                key = QString::fromStdString(keyStr);
            } else if constexpr (std::is_arithmetic_v<KeyType>) {
                key = static_cast<KeyType>(std::stod(keyStr));
            } else {
                // For complex keys, this is a limitation - use default key
                key = KeyType{};
            }
            
            ValueType value = ultimateFromJson<ValueType>(valueJson, ValueType{});
            result[key] = value;
        }
        return result;
    }
    else if constexpr (is_qt_associative_container_v<T>) {
        if (!json.isArray()) {
            return defaultValue;
        }
        
        T result;
        const auto* arr = json.getArray();
        if (!arr) return defaultValue;
        using ValueType = typename T::value_type;
        
        for (const auto& item : *arr) {
            ValueType val = ultimateFromJson<ValueType>(item, ValueType{});
            result.insert(val);
        }
        return result;
    }
    else if constexpr (is_qt_pair<T>::value) {
        if (!json.isArray()) {
            return defaultValue;
        }
        
        const auto* arr = json.getArray();
        if (!arr || arr->size() != 2) {
            return defaultValue;
        }
        
        using FirstType = typename T::first_type;
        using SecondType = typename T::second_type;
        
        FirstType first = ultimateFromJson<FirstType>((*arr)[0], FirstType{});
        SecondType second = ultimateFromJson<SecondType>((*arr)[1], SecondType{});
        
        return T(first, second);
    }
    else if constexpr (std::is_arithmetic_v<T>) {
        if constexpr (std::is_integral_v<T>) {
            return json.isNumber() ? static_cast<T>(json.toInt()) : defaultValue;
        } else {
            return json.isNumber() ? static_cast<T>(json.toDouble()) : defaultValue;
        }
    }
    else if constexpr (std::is_same_v<T, std::string>) {
        return json.isString() ? json.toString() : defaultValue;
    }
    else {
        // Try to use the existing type registry for custom types
        try {
            return TypeRegistry::instance().fromJson<T>(json, defaultValue);
        } catch (...) {
            return defaultValue;
        }
    }
}

} // namespace QtUniversal

// Enhanced Qt type registration function using the ultimate system
template<typename T>
void registerUltimateQtType() {
    TypeRegistry::instance().registerType<T>(
        [](const T& value) -> JsonValue {
            return QtUniversal::ultimateToJson(value);
        },
        [](const JsonValue& json, const T& defaultValue) -> T {
            return QtUniversal::ultimateFromJson<T>(json, defaultValue);
        }
    );
}

// Convenience macro for registering Qt types
#define REGISTER_ULTIMATE_QT_TYPE(Type) \
    JsonStruct::registerUltimateQtType<Type>()

// For convenience, also update the existing macros
#undef REGISTER_QT_TYPE
#define REGISTER_QT_TYPE(Type) \
    JsonStruct::registerUltimateQtType<Type>()

#undef REGISTER_QT_CONTAINER  
#define REGISTER_QT_CONTAINER(Type) \
    JsonStruct::registerUltimateQtType<Type>()

} // namespace JsonStruct
