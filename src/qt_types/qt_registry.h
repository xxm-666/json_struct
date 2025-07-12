#pragma once
#include "../type_registry/registry_core.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>
#include <QJsonArray>
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
            [](const QStringList& list) -> QJsonValue {
                QJsonArray arr;
                for (const auto& str : list) {
                    arr.append(str);
                }
                return arr;
            },
            // fromJson
            [](const QJsonValue& json, const QStringList& defaultValue) -> QStringList {
                if (json.isArray()) {
                    QStringList result;
                    for (const auto& item : json.toArray()) {
                        result << item.toString();
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
            [](const QPointF& point) -> QJsonValue {
                QJsonObject obj;
                obj["x"] = point.x();
                obj["y"] = point.y();
                return obj;
            },
            // fromJson
            [](const QJsonValue& json, const QPointF& defaultValue) -> QPointF {
                if (json.isObject()) {
                    auto obj = json.toObject();
                    return QPointF(obj["x"].toDouble(), obj["y"].toDouble());
                }
                return defaultValue;
            }
        );
    }
    
    // QRectF registration
    static void registerQRectF() {
        TypeRegistry::instance().registerType<QRectF>(
            // toJson
            [](const QRectF& rect) -> QJsonValue {
                QJsonArray arr;
                arr.append(rect.x());
                arr.append(rect.y());
                arr.append(rect.width());
                arr.append(rect.height());
                return arr;
            },
            // fromJson
            [](const QJsonValue& json, const QRectF& defaultValue) -> QRectF {
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
            [](const QRect& rect) -> QJsonValue {
                QJsonArray arr;
                arr.append(rect.x());
                arr.append(rect.y());
                arr.append(rect.width());
                arr.append(rect.height());
                return arr;
            },
            // fromJson
            [](const QJsonValue& json, const QRect& defaultValue) -> QRect {
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
            [](const QColor& color) -> QJsonValue {
                QJsonArray arr;
                arr.append(color.red());
                arr.append(color.green());
                arr.append(color.blue());
                arr.append(color.alpha());
                return arr;
            },
            // fromJson
            [](const QJsonValue& json, const QColor& defaultValue) -> QColor {
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
            [](const QList<QPointF>& list) -> QJsonValue {
                QJsonArray arr;
                for (const auto& point : list) {
                    // Use the registered QPointF serializer
                    arr.append(TypeRegistry::instance().toJson(point));
                }
                return arr;
            },
            // fromJson
            [](const QJsonValue& json, const QList<QPointF>& defaultValue) -> QList<QPointF> {
                if (json.isArray()) {
                    QList<QPointF> result;
                    for (const auto& item : json.toArray()) {
                        // Use the registered QPointF deserializer
                        result << TypeRegistry::instance().fromJson<QPointF>(item, QPointF());
                    }
                    return result; // Directly return result, supports empty array
                }
                return defaultValue; // Only return default when not an array
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

} // namespace JsonStruct
