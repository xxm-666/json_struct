#pragma once
#include "../type_registry/registry_core.h"
#include <QList>
#include <QVector>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QSize>
#include <QSizeF>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <type_traits>

namespace JsonStruct {

// Common Qt type traits detection
template<typename T>
struct qt_type_traits {
    static constexpr bool is_std_container = false;
    static constexpr bool is_qt_type = false;
    static constexpr bool requires_registration = false;
};

// Qt type traits specializations
template<> struct qt_type_traits<QString> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct qt_type_traits<QStringList> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct qt_type_traits<QPointF> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct qt_type_traits<QRectF> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct qt_type_traits<QRect> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct qt_type_traits<QColor> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct qt_type_traits<QSize> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct qt_type_traits<QSizeF> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

// Common Qt container detection
template<typename T>
struct is_qt_list : std::false_type {};

template<typename T>
struct is_qt_list<QList<T>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_qt_vector : std::false_type {};

template<typename T>
struct is_qt_vector<QVector<T>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_qt_set : std::false_type {};

template<typename T>
struct is_qt_set<QSet<T>> : std::true_type {
    using value_type = T;
};

template<typename T>
struct is_qt_map : std::false_type {};

template<typename K, typename V>
struct is_qt_map<QMap<K, V>> : std::true_type {
    using key_type = K;
    using mapped_type = V;
};

template<typename T>
struct is_qt_hash : std::false_type {};

template<typename K, typename V>
struct is_qt_hash<QHash<K, V>> : std::true_type {
    using key_type = K;
    using mapped_type = V;
};

template<typename T>
struct is_qt_pair : std::false_type {};

template<typename T1, typename T2>
struct is_qt_pair<QPair<T1, T2>> : std::true_type {
    using first_type = T1;
    using second_type = T2;
};

// Container type categories
template<typename T>
constexpr bool is_qt_sequence_container_v = 
    is_qt_list<T>::value || 
    is_qt_vector<T>::value;

template<typename T>
constexpr bool is_qt_associative_container_v = 
    is_qt_set<T>::value;

template<typename T>
constexpr bool is_qt_map_container_v = 
    is_qt_map<T>::value || 
    is_qt_hash<T>::value;

template<typename T>
constexpr bool is_qt_container_v = 
    is_qt_sequence_container_v<T> || 
    is_qt_associative_container_v<T> || 
    is_qt_map_container_v<T> ||
    is_qt_pair<T>::value;

// Common Qt basic type serialization functions
namespace QtBasic {

JsonValue qtBasicToJson(const QString& value);
JsonValue qtBasicToJson(const QStringList& value);
JsonValue qtBasicToJson(const QPointF& value);
JsonValue qtBasicToJson(const QRectF& value);
JsonValue qtBasicToJson(const QRect& value);
JsonValue qtBasicToJson(const QColor& value);
JsonValue qtBasicToJson(const QSize& value);
JsonValue qtBasicToJson(const QSizeF& value);

QString qtBasicFromJson(const JsonValue& json, const QString& defaultValue);
QStringList qtBasicFromJson(const JsonValue& json, const QStringList& defaultValue);
QPointF qtBasicFromJson(const JsonValue& json, const QPointF& defaultValue);
QRectF qtBasicFromJson(const JsonValue& json, const QRectF& defaultValue);
QRect qtBasicFromJson(const JsonValue& json, const QRect& defaultValue);
QColor qtBasicFromJson(const JsonValue& json, const QColor& defaultValue);
QSize qtBasicFromJson(const JsonValue& json, const QSize& defaultValue);
QSizeF qtBasicFromJson(const JsonValue& json, const QSizeF& defaultValue);

} // namespace QtBasic

// Utility functions for Qt/JsonStruct conversion
inline QJsonValue toQJsonValue(const JsonStruct::JsonValue& value) {
    if (value.isBool()) {
        return QJsonValue(value.toBool());
    } else if (value.isNumber()) {
        return QJsonValue(value.toDouble());
    } else if (value.isString()) {
        return QJsonValue(QString::fromStdString(value.toString()));
    } else if (const auto& array = value.toArray()) {
        QJsonArray arr;
        for (const auto& item : array->get()) {
            arr.append(toQJsonValue(item));
        }
        return arr;
    } else if (const auto& object = value.toObject()) {
        QJsonObject obj;
        for (const auto& [key, val] : object->get()) {
            obj[QString::fromStdString(key)] = toQJsonValue(val);
        }
        return obj;
    }
    return QJsonValue();
}

inline JsonStruct::JsonValue fromQJsonValue(const QJsonValue& value) {
    if (value.isBool()) {
        return JsonStruct::JsonValue(value.toBool());
    } else if (value.isDouble()) {
        return JsonStruct::JsonValue(value.toDouble());
    } else if (value.isString()) {
        return JsonStruct::JsonValue(value.toString().toStdString());
    } else if (value.isArray()) {
        JsonStruct::JsonValue::ArrayType arr;
        for (const auto& item : value.toArray()) {
            arr.push_back(fromQJsonValue(item));
        }
        return JsonStruct::JsonValue(arr);
    } else if (value.isObject()) {
        JsonStruct::JsonValue::ObjectType obj;
        for (const auto& key : value.toObject().keys()) {
            obj[key.toStdString()] = fromQJsonValue(value.toObject().value(key));
        }
        return JsonStruct::JsonValue(obj);
    }
    return JsonStruct::JsonValue();
}

} // namespace JsonStruct
