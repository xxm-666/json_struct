#include "qt_ultimate_registry.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

namespace JsonStruct {
namespace QtUniversal {

// QString implementations
JsonValue qtBasicToJson(const QString& value) {
    return JsonValue(value.toStdString());
}

QString qtBasicFromJson(const JsonValue& json, const QString& defaultValue) {
    return json.isString() ? QString::fromStdString(json.toString()) : defaultValue;
}

// QStringList implementations
JsonValue qtBasicToJson(const QStringList& value) {
    JsonValue::ArrayType arr;
    for (const QString& str : value) {
        arr.push_back(JsonValue(str.toStdString()));
    }
    return JsonValue(arr);
}

QStringList qtBasicFromJson(const JsonValue& json, const QStringList& defaultValue) {
    if (!json.isArray()) {
        return defaultValue;
    }
    
    QStringList result;
    const JsonValue::ArrayType* arr = json.getArray();
    if (arr) {
        for (const JsonValue& item : *arr) {
            if (item.isString()) {
                result.append(QString::fromStdString(item.toString()));
            }
        }
    }
    return result;
}

// QPointF implementations
JsonValue qtBasicToJson(const QPointF& value) {
    JsonValue::ObjectType obj;
    obj["x"] = JsonValue(value.x());
    obj["y"] = JsonValue(value.y());
    return JsonValue(obj);
}

QPointF qtBasicFromJson(const JsonValue& json, const QPointF& defaultValue) {
    if (!json.isObject()) {
        return defaultValue;
    }
    
    const JsonValue::ObjectType* obj = json.getObject();
    if (!obj) {
        return defaultValue;
    }
    
    double x = defaultValue.x();
    double y = defaultValue.y();
    
    JsonValue::ObjectType::const_iterator xIt = obj->find("x");
    if (xIt != obj->end() && xIt->second.isNumber()) {
        x = xIt->second.toDouble();
    }
    
    JsonValue::ObjectType::const_iterator yIt = obj->find("y");
    if (yIt != obj->end() && yIt->second.isNumber()) {
        y = yIt->second.toDouble();
    }
    
    return QPointF(x, y);
}

// QRectF implementations
JsonValue qtBasicToJson(const QRectF& value) {
    JsonValue::ObjectType obj;
    obj["x"] = JsonValue(value.x());
    obj["y"] = JsonValue(value.y());
    obj["width"] = JsonValue(value.width());
    obj["height"] = JsonValue(value.height());
    return JsonValue(obj);
}

QRectF qtBasicFromJson(const JsonValue& json, const QRectF& defaultValue) {
    if (!json.isObject()) {
        return defaultValue;
    }
    
    const JsonValue::ObjectType* obj = json.getObject();
    if (!obj) {
        return defaultValue;
    }
    
    double x = defaultValue.x();
    double y = defaultValue.y();
    double width = defaultValue.width();
    double height = defaultValue.height();
    
    JsonValue::ObjectType::const_iterator xIt = obj->find("x");
    if (xIt != obj->end() && xIt->second.isNumber()) {
        x = xIt->second.toDouble();
    }
    
    JsonValue::ObjectType::const_iterator yIt = obj->find("y");
    if (yIt != obj->end() && yIt->second.isNumber()) {
        y = yIt->second.toDouble();
    }
    
    JsonValue::ObjectType::const_iterator wIt = obj->find("width");
    if (wIt != obj->end() && wIt->second.isNumber()) {
        width = wIt->second.toDouble();
    }
    
    JsonValue::ObjectType::const_iterator hIt = obj->find("height");
    if (hIt != obj->end() && hIt->second.isNumber()) {
        height = hIt->second.toDouble();
    }
    
    return QRectF(x, y, width, height);
}

// QRect implementations
JsonValue qtBasicToJson(const QRect& value) {
    JsonValue::ObjectType obj;
    obj["x"] = JsonValue(static_cast<int64_t>(value.x()));
    obj["y"] = JsonValue(static_cast<int64_t>(value.y()));
    obj["width"] = JsonValue(static_cast<int64_t>(value.width()));
    obj["height"] = JsonValue(static_cast<int64_t>(value.height()));
    return JsonValue(obj);
}

QRect qtBasicFromJson(const JsonValue& json, const QRect& defaultValue) {
    if (!json.isObject()) {
        return defaultValue;
    }
    
    const JsonValue::ObjectType* obj = json.getObject();
    if (!obj) {
        return defaultValue;
    }
    
    int x = defaultValue.x();
    int y = defaultValue.y();
    int width = defaultValue.width();
    int height = defaultValue.height();
    
    JsonValue::ObjectType::const_iterator xIt = obj->find("x");
    if (xIt != obj->end() && xIt->second.isNumber()) {
        x = xIt->second.toInt();
    }
    
    JsonValue::ObjectType::const_iterator yIt = obj->find("y");
    if (yIt != obj->end() && yIt->second.isNumber()) {
        y = yIt->second.toInt();
    }
    
    JsonValue::ObjectType::const_iterator wIt = obj->find("width");
    if (wIt != obj->end() && wIt->second.isNumber()) {
        width = wIt->second.toInt();
    }
    
    JsonValue::ObjectType::const_iterator hIt = obj->find("height");
    if (hIt != obj->end() && hIt->second.isNumber()) {
        height = hIt->second.toInt();
    }
    
    return QRect(x, y, width, height);
}

// QColor implementations
JsonValue qtBasicToJson(const QColor& value) {
    JsonValue::ObjectType obj;
    obj["r"] = JsonValue(static_cast<int64_t>(value.red()));
    obj["g"] = JsonValue(static_cast<int64_t>(value.green()));
    obj["b"] = JsonValue(static_cast<int64_t>(value.blue()));
    obj["a"] = JsonValue(static_cast<int64_t>(value.alpha()));
    return JsonValue(obj);
}

QColor qtBasicFromJson(const JsonValue& json, const QColor& defaultValue) {
    if (!json.isObject()) {
        return defaultValue;
    }
    
    const JsonValue::ObjectType* obj = json.getObject();
    if (!obj) {
        return defaultValue;
    }
    
    int r = defaultValue.red();
    int g = defaultValue.green();
    int b = defaultValue.blue();
    int a = defaultValue.alpha();
    
    JsonValue::ObjectType::const_iterator rIt = obj->find("r");
    if (rIt != obj->end() && rIt->second.isNumber()) {
        r = rIt->second.toInt();
    }
    
    JsonValue::ObjectType::const_iterator gIt = obj->find("g");
    if (gIt != obj->end() && gIt->second.isNumber()) {
        g = gIt->second.toInt();
    }
    
    JsonValue::ObjectType::const_iterator bIt = obj->find("b");
    if (bIt != obj->end() && bIt->second.isNumber()) {
        b = bIt->second.toInt();
    }
    
    JsonValue::ObjectType::const_iterator aIt = obj->find("a");
    if (aIt != obj->end() && aIt->second.isNumber()) {
        a = aIt->second.toInt();
    }
    
    return QColor(r, g, b, a);
}

// QSize implementations
JsonValue qtBasicToJson(const QSize& value) {
    JsonValue::ObjectType obj;
    obj["width"] = JsonValue(static_cast<int64_t>(value.width()));
    obj["height"] = JsonValue(static_cast<int64_t>(value.height()));
    return JsonValue(obj);
}

QSize qtBasicFromJson(const JsonValue& json, const QSize& defaultValue) {
    if (!json.isObject()) {
        return defaultValue;
    }
    
    const JsonValue::ObjectType* obj = json.getObject();
    if (!obj) {
        return defaultValue;
    }
    
    int width = defaultValue.width();
    int height = defaultValue.height();
    
    JsonValue::ObjectType::const_iterator wIt = obj->find("width");
    if (wIt != obj->end() && wIt->second.isNumber()) {
        width = wIt->second.toInt();
    }
    
    JsonValue::ObjectType::const_iterator hIt = obj->find("height");
    if (hIt != obj->end() && hIt->second.isNumber()) {
        height = hIt->second.toInt();
    }
    
    return QSize(width, height);
}

// QSizeF implementations
JsonValue qtBasicToJson(const QSizeF& value) {
    JsonValue::ObjectType obj;
    obj["width"] = JsonValue(value.width());
    obj["height"] = JsonValue(value.height());
    return JsonValue(obj);
}

QSizeF qtBasicFromJson(const JsonValue& json, const QSizeF& defaultValue) {
    if (!json.isObject()) {
        return defaultValue;
    }
    
    const JsonValue::ObjectType* obj = json.getObject();
    if (!obj) {
        return defaultValue;
    }
    
    double width = defaultValue.width();
    double height = defaultValue.height();
    
    JsonValue::ObjectType::const_iterator wIt = obj->find("width");
    if (wIt != obj->end() && wIt->second.isNumber()) {
        width = wIt->second.toDouble();
    }
    
    JsonValue::ObjectType::const_iterator hIt = obj->find("height");
    if (hIt != obj->end() && hIt->second.isNumber()) {
        height = hIt->second.toDouble();
    }
    
    return QSizeF(width, height);
}

} // namespace QtUniversal
} // namespace JsonStruct
