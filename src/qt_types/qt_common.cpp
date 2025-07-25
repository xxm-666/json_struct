#include "qt_common.h"

namespace JsonStruct {
namespace QtBasic {

JsonValue qtBasicToJson(const QString& value) {
    return JsonValue(value.toStdString());
}

JsonValue qtBasicToJson(const QStringList& value) {
    JsonValue::ArrayType arr;
    for (const QString& str : value) {
        arr.push_back(JsonValue(str.toStdString()));
    }
    return JsonValue(arr);
}

JsonValue qtBasicToJson(const QPointF& value) {
    JsonValue::ArrayType arr;
    arr.push_back(JsonValue(value.x()));
    arr.push_back(JsonValue(value.y()));
    return JsonValue(arr);
}

JsonValue qtBasicToJson(const QRectF& value) {
    JsonValue::ArrayType arr;
    arr.push_back(JsonValue(value.x()));
    arr.push_back(JsonValue(value.y()));
    arr.push_back(JsonValue(value.width()));
    arr.push_back(JsonValue(value.height()));
    return JsonValue(arr);
}

JsonValue qtBasicToJson(const QRect& value) {
    JsonValue::ArrayType arr;
    arr.push_back(JsonValue(value.x()));
    arr.push_back(JsonValue(value.y()));
    arr.push_back(JsonValue(value.width()));
    arr.push_back(JsonValue(value.height()));
    return JsonValue(arr);
}

JsonValue qtBasicToJson(const QColor& value) {
    JsonValue::ArrayType arr;
    arr.push_back(JsonValue(value.red()));
    arr.push_back(JsonValue(value.green()));
    arr.push_back(JsonValue(value.blue()));
    arr.push_back(JsonValue(value.alpha()));
    return JsonValue(arr);
}

JsonValue qtBasicToJson(const QSize& value) {
    JsonValue::ArrayType arr;
    arr.push_back(JsonValue(value.width()));
    arr.push_back(JsonValue(value.height()));
    return JsonValue(arr);
}

JsonValue qtBasicToJson(const QSizeF& value) {
    JsonValue::ArrayType arr;
    arr.push_back(JsonValue(value.width()));
    arr.push_back(JsonValue(value.height()));
    return JsonValue(arr);
}

QString qtBasicFromJson(const JsonValue& json, const QString& defaultValue) {
    if (json.isString()) {
        return QString::fromStdString(json.toString());
    }
    return defaultValue;
}

QStringList qtBasicFromJson(const JsonValue& json, const QStringList& defaultValue) {
    if (const auto& arr = json.toArray()) {
        QStringList result;
        for (const auto& item : arr->get()) {
            if (item.isString()) {
                result.append(QString::fromStdString(item.toString()));
            }
        }
        return result;
    }
    return defaultValue;
}

QPointF qtBasicFromJson(const JsonValue& json, const QPointF& defaultValue) {
    if (const auto& array = json.toArray()) {
        const auto& arr = array->get();
        if (arr.size() >= 2 && arr[0].isNumber() && arr[1].isNumber()) {
            return QPointF(arr[0].toDouble(), arr[1].toDouble());
        }
    }
    return defaultValue;
}

QRectF qtBasicFromJson(const JsonValue& json, const QRectF& defaultValue) {
    if (const auto& array = json.toArray()) {
        const auto& arr = array->get();
        if (arr.size() >= 4) {
            return QRectF(arr[0].toDouble(), arr[1].toDouble(), 
                          arr[2].toDouble(), arr[3].toDouble());
        }
    }
    return defaultValue;
}

QRect qtBasicFromJson(const JsonValue& json, const QRect& defaultValue) {
    if (const auto& array = json.toArray()) {
        const auto& arr = array->get();
        if (arr.size() >= 4) {
            return QRect(arr[0].toInt(), arr[1].toInt(), 
                         arr[2].toInt(), arr[3].toInt());
        }
    }
    return defaultValue;
}

QColor qtBasicFromJson(const JsonValue& json, const QColor& defaultValue) {
    if (const auto& array = json.toArray()) {
        const auto& arr = array->get();
        if (arr.size() >= 4) {
            return QColor(arr[0].toInt(), arr[1].toInt(), 
                          arr[2].toInt(), arr[3].toInt());
        }
    }
    return defaultValue;
}

QSize qtBasicFromJson(const JsonValue& json, const QSize& defaultValue) {
    if (const auto& array = json.toArray()) {
        const auto& arr = array->get();
        if (arr.size() >= 2) {
            return QSize(arr[0].toInt(), arr[1].toInt());
        }
    }
    return defaultValue;
}

QSizeF qtBasicFromJson(const JsonValue& json, const QSizeF& defaultValue) {
    if (const auto& array = json.toArray()) {
        const auto& arr = array->get();
        if (arr.size() >= 2) {
            return QSizeF(arr[0].toDouble(), arr[1].toDouble());
        }
    }
    return defaultValue;
}

} // namespace QtBasic
} // namespace JsonStruct
