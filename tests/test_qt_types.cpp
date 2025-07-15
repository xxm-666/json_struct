#include "type_registry/registry_core.h"
#include "qt_types/qt_registry.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>
#include <iostream>
#include <cassert>

using namespace JsonStruct;

void testQStringList() {
    QStringList list = {"one", "two", "three"};
    JsonValue json = TypeRegistry::instance().toJson(list);
    QStringList restored = TypeRegistry::instance().fromJson<QStringList>(json, QStringList());
    assert(restored == list);
}

void testQPointF() {
    QPointF point(1.5, 2.5);
    JsonValue json = TypeRegistry::instance().toJson(point);
    QPointF restored = TypeRegistry::instance().fromJson<QPointF>(json, QPointF());
    assert(qFuzzyCompare(restored.x(), point.x()));
    assert(qFuzzyCompare(restored.y(), point.y()));
}

void testQRectF() {
    QRectF rect(1.0, 2.0, 3.0, 4.0);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    QRectF restored = TypeRegistry::instance().fromJson<QRectF>(json, QRectF());
    assert(qFuzzyCompare(restored.x(), rect.x()));
    assert(qFuzzyCompare(restored.y(), rect.y()));
    assert(qFuzzyCompare(restored.width(), rect.width()));
    assert(qFuzzyCompare(restored.height(), rect.height()));
}

void testQColor() {
    QColor color(255, 128, 64, 32);
    JsonValue json = TypeRegistry::instance().toJson(color);
    QColor restored = TypeRegistry::instance().fromJson<QColor>(json, QColor());
    assert(restored.red() == color.red());
    assert(restored.green() == color.green());
    assert(restored.blue() == color.blue());
    assert(restored.alpha() == color.alpha());
}

void testQRect() {
    QRect rect(10, 20, 30, 40);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    QRect restored = TypeRegistry::instance().fromJson<QRect>(json, QRect());
    assert(restored.x() == rect.x());
    assert(restored.y() == rect.y());
    assert(restored.width() == rect.width());
    assert(restored.height() == rect.height());
}

void testQListQPointF() {
    QList<QPointF> points = {QPointF(1.1, 2.2), QPointF(3.3, 4.4)};
    JsonValue json = TypeRegistry::instance().toJson(points);
    QList<QPointF> restored = TypeRegistry::instance().fromJson<QList<QPointF>>(json, QList<QPointF>());
    assert(restored.size() == points.size());
    for (int i = 0; i < points.size(); ++i) {
        assert(qFuzzyCompare(restored[i].x(), points[i].x()));
        assert(qFuzzyCompare(restored[i].y(), points[i].y()));
    }
}

void testToQJsonValue() {
    JsonStruct::JsonValue jsonValue;
    jsonValue["key"] = JsonStruct::JsonValue("value");
    QJsonValue qJsonValue = JsonStruct::toQJsonValue(jsonValue);
    assert(qJsonValue.isObject());
    assert(qJsonValue.toObject()["key"].toString() == "value");
}

void testFromQJsonValue() {
    QJsonValue qJsonValue = QJsonValue(QString("value"));
    JsonStruct::JsonValue jsonValue = JsonStruct::fromQJsonValue(qJsonValue);
    assert(jsonValue.isString());
    assert(jsonValue.toString() == "value");
}

void testComplexObject() {
    QJsonObject innerObj;
    innerObj["innerKey1"] = 123;
    innerObj["innerKey2"] = "innerValue";
    QJsonArray arr;
    arr.append(QJsonValue(1.23));
    arr.append(QJsonValue(true));
    arr.append(QJsonValue("arrayString"));
    QJsonObject obj;
    obj["stringKey"] = "stringValue";
    obj["intKey"] = 42;
    obj["boolKey"] = false;
    obj["arrayKey"] = arr;
    obj["objectKey"] = innerObj;
    QJsonValue complexValue(obj);
    JsonStruct::JsonValue jsonValue = JsonStruct::fromQJsonValue(complexValue);
    QJsonValue restoredQJson = JsonStruct::toQJsonValue(jsonValue);
    QJsonObject restoredObj = restoredQJson.toObject();
    assert(restoredObj["stringKey"].toString() == "stringValue");
    assert(restoredObj["intKey"].toInt() == 42);
    assert(restoredObj["boolKey"].toBool() == false);
    assert(restoredObj["arrayKey"].toArray()[2].toString() == "arrayString");
    assert(restoredObj["objectKey"].toObject()["innerKey2"].toString() == "innerValue");
}

void testEdgeCases() {
    QJsonObject emptyObj;
    JsonStruct::JsonValue jsonEmpty = JsonStruct::fromQJsonValue(QJsonValue(emptyObj));
    assert(jsonEmpty.isObject());
    QJsonArray emptyArr;
    JsonStruct::JsonValue jsonArr = JsonStruct::fromQJsonValue(QJsonValue(emptyArr));
    assert(jsonArr.isArray());
    QJsonValue nullVal;
    JsonStruct::JsonValue jsonNull = JsonStruct::fromQJsonValue(nullVal);
    assert(jsonNull.isNull());
}

int main() {
    JsonStruct::QtTypesRegistration::registerAllQtTypes();

    testQStringList();
    testQPointF();
    testQRectF();
    testQRect();
    testQColor();
    testQListQPointF();
    testToQJsonValue();
    testFromQJsonValue();
    testComplexObject();
    testEdgeCases();

    std::cout << "All Qt type tests passed!" << std::endl;
    return 0;
}
