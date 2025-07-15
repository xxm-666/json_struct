#pragma once
#include "../type_registry/registry_core.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace JsonStruct {

// Qt type registration
class QtTypesRegistration {
public:
    static void registerAllQtTypes() {
        registerQStringList();
        registerQPointF();
        registerQRectF();
        registerQRect();
        registerQColor();
        registerQListQPointF();
    }

private:
    // QStringList registration
    static void registerQStringList() {
        TypeRegistry::instance().registerType<QStringList>(
            // toJson
            [](const QStringList& list) -> JsonStruct::JsonValue {
                JsonStruct::JsonValue::ArrayType arr;
                for (const auto& str : list) {
                    arr.push_back(JsonStruct::JsonValue(str.toStdString()));
                }
                return JsonStruct::JsonValue(arr);
            },
            // fromJson
            [](const JsonStruct::JsonValue& json, const QStringList& defaultValue) -> QStringList {
                if (json.isArray()) {
                    QStringList result;
                    for (const auto& item : json.toArray()) {
                        result << QString::fromStdString(item.toString());
                    }
                    return result; // Directly return result, supports empty array
                }
                return defaultValue; // Only return default when not an array
            }
        );
    }
    
    // QPointF registration
    static void registerQPointF() {
        TypeRegistry::instance().registerType<QPointF>(
            // toJson
            [](const QPointF& point) -> JsonStruct::JsonValue {
                JsonStruct::JsonValue::ArrayType arr;
                arr.push_back(JsonStruct::JsonValue(point.x()));
                arr.push_back(JsonStruct::JsonValue(point.y()));
                return JsonStruct::JsonValue(arr);
            },
            // fromJson
            [](const JsonStruct::JsonValue& json, const QPointF& defaultValue) -> QPointF {
                if (json.isArray()) {
                    // auto obj = json.toObject();
                    auto arr = json.toArray();
                    return QPointF(arr[0].toDouble(), arr[1].toDouble());
                }
                return defaultValue;
            }
        );
    }
    
    // QRectF registration
    static void registerQRectF() {
        TypeRegistry::instance().registerType<QRectF>(
            // toJson
            [](const QRectF& rect) -> JsonStruct::JsonValue {
                JsonStruct::JsonValue::ArrayType arr;
                arr.push_back(JsonStruct::JsonValue(rect.x()));
                arr.push_back(JsonStruct::JsonValue(rect.y()));
                arr.push_back(JsonStruct::JsonValue(rect.width()));
                arr.push_back(JsonStruct::JsonValue(rect.height()));
                return JsonStruct::JsonValue(arr);
            },
            // fromJson
            [](const JsonStruct::JsonValue& json, const QRectF& defaultValue) -> QRectF {
                if (json.isArray()) {
                    auto arr = json.toArray();
                    if (arr.size() == 4) {
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
            [](const QRect& rect) -> JsonStruct::JsonValue {
                JsonStruct::JsonValue::ArrayType arr;
                arr.push_back(JsonStruct::JsonValue(rect.x()));
                arr.push_back(JsonStruct::JsonValue(rect.y()));
                arr.push_back(JsonStruct::JsonValue(rect.width()));
                arr.push_back(JsonStruct::JsonValue(rect.height()));
                return JsonStruct::JsonValue(arr);
            },
            // fromJson
            [](const JsonStruct::JsonValue& json, const QRect& defaultValue) -> QRect {
                if (json.isArray()) {
                    auto arr = json.toArray();
                    if (arr.size() == 4) {
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
            [](const QColor& color) -> JsonStruct::JsonValue {
                JsonStruct::JsonValue::ArrayType arr;
                arr.push_back(JsonStruct::JsonValue(color.red()));
                arr.push_back(JsonStruct::JsonValue(color.green()));
                arr.push_back(JsonStruct::JsonValue(color.blue()));
                arr.push_back(JsonStruct::JsonValue(color.alpha()));
                return JsonStruct::JsonValue(arr);
            },
            // fromJson
            [](const JsonStruct::JsonValue& json, const QColor& defaultValue) -> QColor {
                if (json.isArray()) {
                    auto arr = json.toArray();
                    if (arr.size() == 4) {
                        return QColor(arr[0].toInt(), arr[1].toInt(),
                                      arr[2].toInt(), arr[3].toInt());
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
            [](const QList<QPointF>& list) -> JsonStruct::JsonValue {
                JsonStruct::JsonValue::ArrayType arr;
                for (const auto& point : list) {
                    arr.push_back(TypeRegistry::instance().toJson(point));
                }
                return JsonStruct::JsonValue(arr);
            },
            // fromJson
            [](const JsonStruct::JsonValue& json, const QList<QPointF>& defaultValue) -> QList<QPointF> {
                if (json.isArray()) {
                    QList<QPointF> result;
                    for (const auto& item : json.toArray()) {
                        result.push_back(TypeRegistry::instance().fromJson<QPointF>(item, QPointF()));
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

} // namespace JsonStruct
