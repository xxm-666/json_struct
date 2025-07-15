#pragma once
#include "../type_registry/registry_core.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QSize>
#include <QSizeF>
#include <type_traits>

namespace JsonStruct {

// Qt type traits detection
template<typename T>
struct type_traits {
    static constexpr bool is_std_container = false;
    static constexpr bool is_qt_type = false;
    static constexpr bool requires_registration = false;
};

// Qt type traits specializations
template<> struct type_traits<QString> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct type_traits<QStringList> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct type_traits<QPointF> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct type_traits<QRectF> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct type_traits<QRect> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct type_traits<QColor> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct type_traits<QSize> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

template<> struct type_traits<QSizeF> { 
    static constexpr bool is_qt_type = true; 
    static constexpr bool requires_registration = true; 
};

// Qt container detection
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
constexpr bool is_qt_container_v = is_qt_list<T>::value || is_qt_vector<T>::value;

// Qt type registration
class QtTypesRegistration {
public:
    static void registerAllQtTypes() {
        registerQString();
        registerQStringList();
        registerQPointF();
        registerQRectF();
        registerQRect();
        registerQColor();
        registerQSize();
        registerQSizeF();
        registerQListQPointF();
    }

private:
    // QString registration
    static void registerQString() {
        TypeRegistry::instance().registerType<QString>(
            // toJson
            [](const QString& str) -> JsonValue {
                return JsonValue(str.toStdString());
            },
            // fromJson
            [](const JsonValue& json, const QString& defaultValue) -> QString {
                if (json.isString()) {
                    return QString::fromStdString(json.toString());
                }
                return defaultValue;
            }
        );
    }

    // QStringList registration
    static void registerQStringList() {
        TypeRegistry::instance().registerType<QStringList>(
            // toJson
            [](const QStringList& list) -> JsonValue {
                JsonValue::ArrayType arr;
                for (const QString& str : list) {
                    arr.push_back(JsonValue(str.toStdString()));
                }
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QStringList& defaultValue) -> QStringList {
                if (json.isArray()) {
                    QStringList result;
                    for (const auto& item : json.toArray()) {
                        if (item.isString()) {
                            result.append(QString::fromStdString(item.toString()));
                        }
                    }
                    return result;
                }
                return defaultValue;
            }
        );
    }

    // QPointF registration
    static void registerQPointF() {
        TypeRegistry::instance().registerType<QPointF>(
            // toJson
            [](const QPointF& point) -> JsonValue {
                JsonValue::ArrayType arr;
                arr.push_back(JsonValue(point.x()));
                arr.push_back(JsonValue(point.y()));
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QPointF& defaultValue) -> QPointF {
                if (json.isArray()) {
                    const auto& arr = json.toArray();
                    if (arr.size() >= 2 && arr[0].isNumber() && arr[1].isNumber()) {
                        return QPointF(arr[0].toDouble(), arr[1].toDouble());
                    }
                }
                return defaultValue;
            }
        );
    }

    // QRectF registration
    static void registerQRectF() {
        TypeRegistry::instance().registerType<QRectF>(
            // toJson
            [](const QRectF& rect) -> JsonValue {
                JsonValue::ArrayType arr;
                arr.push_back(JsonValue(rect.x()));
                arr.push_back(JsonValue(rect.y()));
                arr.push_back(JsonValue(rect.width()));
                arr.push_back(JsonValue(rect.height()));
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QRectF& defaultValue) -> QRectF {
                if (json.isArray()) {
                    const auto& arr = json.toArray();
                    if (arr.size() >= 4) {
                        return QRectF(arr[0].toDouble(), arr[1].toDouble(), 
                                      arr[2].toDouble(), arr[3].toDouble());
                    }
                }
                return defaultValue;
            }
        );
    }

    // QRect registration
    static void registerQRect() {
        TypeRegistry::instance().registerType<QRect>(
            // toJson
            [](const QRect& rect) -> JsonValue {
                JsonValue::ArrayType arr;
                arr.push_back(JsonValue(rect.x()));
                arr.push_back(JsonValue(rect.y()));
                arr.push_back(JsonValue(rect.width()));
                arr.push_back(JsonValue(rect.height()));
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QRect& defaultValue) -> QRect {
                if (json.isArray()) {
                    const auto& arr = json.toArray();
                    if (arr.size() >= 4) {
                        return QRect(arr[0].toInt(), arr[1].toInt(), 
                                     arr[2].toInt(), arr[3].toInt());
                    }
                }
                return defaultValue;
            }
        );
    }

    // QColor registration
    static void registerQColor() {
        TypeRegistry::instance().registerType<QColor>(
            // toJson
            [](const QColor& color) -> JsonValue {
                JsonValue::ArrayType arr;
                arr.push_back(JsonValue(color.red()));
                arr.push_back(JsonValue(color.green()));
                arr.push_back(JsonValue(color.blue()));
                arr.push_back(JsonValue(color.alpha()));
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QColor& defaultValue) -> QColor {
                if (json.isArray()) {
                    const auto& arr = json.toArray();
                    if (arr.size() >= 4) {
                        return QColor(arr[0].toInt(), arr[1].toInt(), 
                                      arr[2].toInt(), arr[3].toInt());
                    }
                }
                return defaultValue;
            }
        );
    }

    // QSize registration
    static void registerQSize() {
        TypeRegistry::instance().registerType<QSize>(
            // toJson
            [](const QSize& size) -> JsonValue {
                JsonValue::ArrayType arr;
                arr.push_back(JsonValue(size.width()));
                arr.push_back(JsonValue(size.height()));
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QSize& defaultValue) -> QSize {
                if (json.isArray()) {
                    const auto& arr = json.toArray();
                    if (arr.size() >= 2) {
                        return QSize(arr[0].toInt(), arr[1].toInt());
                    }
                }
                return defaultValue;
            }
        );
    }

    // QSizeF registration
    static void registerQSizeF() {
        TypeRegistry::instance().registerType<QSizeF>(
            // toJson
            [](const QSizeF& size) -> JsonValue {
                JsonValue::ArrayType arr;
                arr.push_back(JsonValue(size.width()));
                arr.push_back(JsonValue(size.height()));
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QSizeF& defaultValue) -> QSizeF {
                if (json.isArray()) {
                    const auto& arr = json.toArray();
                    if (arr.size() >= 2) {
                        return QSizeF(arr[0].toDouble(), arr[1].toDouble());
                    }
                }
                return defaultValue;
            }
        );
    }

    // QList<QPointF> registration
    static void registerQListQPointF() {
        TypeRegistry::instance().registerType<QList<QPointF>>(
            // toJson
            [](const QList<QPointF>& list) -> JsonValue {
                JsonValue::ArrayType arr;
                for (const QPointF& point : list) {
                    JsonValue::ArrayType pointArr;
                    pointArr.push_back(JsonValue(point.x()));
                    pointArr.push_back(JsonValue(point.y()));
                    arr.push_back(JsonValue(pointArr));
                }
                return JsonValue(arr);
            },
            // fromJson
            [](const JsonValue& json, const QList<QPointF>& defaultValue) -> QList<QPointF> {
                if (json.isArray()) {
                    QList<QPointF> result;
                    for (const auto& item : json.toArray()) {
                        if (item.isArray()) {
                            const auto& pointArr = item.toArray();
                            if (pointArr.size() >= 2 && pointArr[0].isNumber() && pointArr[1].isNumber()) {
                                result.append(QPointF(pointArr[0].toDouble(), pointArr[1].toDouble()));
                            }
                        }
                    }
                    return result;
                }
                return defaultValue;
            }
        );
    }
};


// Static initialization for automatic Qt type registration
namespace {
struct QtTypesAutoRegistrar {
    QtTypesAutoRegistrar() {
        QtTypesRegistration::registerAllQtTypes();
    }
};
static QtTypesAutoRegistrar qt_types_auto_registrar;
}

// 添加类型转换函数
inline QJsonValue toQJsonValue(const JsonStruct::JsonValue& value) {
    // 根据JsonStruct::JsonValue的实际内容进行转换
    if (value.isBool()) {
        return QJsonValue(value.toBool());
    } else if (value.isNumber()) {
        return QJsonValue(value.toDouble());
    } else if (value.isString()) {
        return QJsonValue(QString::fromStdString(value.toString()));
    } else if (value.isArray()) {
        QJsonArray arr;
        for (const auto& item : value.toArray()) {
            arr.append(toQJsonValue(item));
        }
        return arr;
    } else if (value.isObject()) {
        QJsonObject obj;
        for (const auto& [key, val] : value.toObject()) {
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
    return JsonStruct::JsonValue(); // Return null value
}

// Qt容器的通用支持（类似std容器）
template<typename Container>
typename std::enable_if<is_qt_container_v<Container>, JsonStruct::JsonValue>::type
qtContainerToJson(const Container& container) {
    JsonStruct::JsonValue::ArrayType arr;
    for (const auto& item : container) {
        // 这里需要根据Qt类型调用相应的序列化
        if constexpr (type_traits<typename Container::value_type>::requires_registration) {
            arr.push_back(TypeRegistry::instance().toJson(item));
        } else {
            // 对于基础类型，直接处理
            arr.push_back(JsonStruct::JsonValue(item));
        }
    }
    return JsonStruct::JsonValue(arr);
}

template<typename Container>
typename std::enable_if<is_qt_container_v<Container>, Container>::type
qtContainerFromJson(const JsonStruct::JsonValue& json, const Container& defaultValue) {
    if (!json.isArray()) {
        return defaultValue;
    }

    Container result;
    const auto& arr = json.toArray();

    for (const auto& item : arr) {
        using ValueType = typename Container::value_type;
        if constexpr (type_traits<ValueType>::requires_registration) {
            result.append(TypeRegistry::instance().fromJson(item, ValueType{}));
        } else {
            // 对于基础类型，直接转换
            result.append(ValueType(item));
        }
    }

    return result;
}
} // namespace JsonStruct
