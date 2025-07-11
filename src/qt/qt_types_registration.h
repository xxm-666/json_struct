#pragma once
#include "json_type_registry.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>
#include <QJsonArray>
#include <QString>

namespace JsonStruct {

// Qt类型注册
class QtTypesRegistration {
public:
    static void registerAllQtTypes() {
        registerQStringList();
        registerQPointF();
        registerQRectF();
        registerQRect();
        registerQColor();
        registerQListQPointF();
        // 可以继续添加更多Qt类型...
    }

private:
    // QStringList 注册
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
                    return result; // 直接返回结果，支持空数组
                }
                return defaultValue; // 只有当不是数组时才返回默认�?
            }
        );
    }
    
    // QPointF 注册
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
    
    // QRectF 注册
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
    
    // QRect 注册
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
    
    // QColor 注册
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
    
    // QList<QPointF> 注册
    static void registerQListQPointF() {
        TypeRegistry::instance().registerType<QList<QPointF>>(
            // toJson
            [](const QList<QPointF>& list) -> QJsonValue {
                QJsonArray arr;
                for (const auto& point : list) {
                    // 使用已注册的QPointF序列化器
                    arr.append(TypeRegistry::instance().toJson(point));
                }
                return arr;
            },
            // fromJson
            [](const QJsonValue& json, const QList<QPointF>& defaultValue) -> QList<QPointF> {
                if (json.isArray()) {
                    QList<QPointF> result;
                    for (const auto& item : json.toArray()) {
                        // 使用已注册的QPointF反序列化�?
                        result << TypeRegistry::instance().fromJson<QPointF>(item, QPointF());
                    }
                    return result; // 直接返回结果，支持空数组
                }
                return defaultValue; // 只有当不是数组时才返回默认�?
            }
        );
    }
};

// 自动注册Qt类型的静态初始化�?
namespace {
    struct QtTypesAutoRegistrar {
        QtTypesAutoRegistrar() {
            QtTypesRegistration::registerAllQtTypes();
        }
    };
    static QtTypesAutoRegistrar qt_types_auto_registrar;
}

} // namespace JsonStruct
