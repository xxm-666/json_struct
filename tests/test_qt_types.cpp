#include "../test_framework/test_framework.h"
#include "../src/qt_types/qt_registry.h"
#include "../src/type_registry/registry_core.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>

using namespace JsonStruct;

TEST(QStringListSerialization) {
    QtTypesRegistration::registerAllQtTypes();
    
    QStringList list = {"one", "two", "three"};
    JsonValue json = TypeRegistry::instance().toJson(list);
    QStringList restored = TypeRegistry::instance().fromJson<QStringList>(json, QStringList());
    
    ASSERT_EQ(list, restored);
}

TEST(QPointFSerialization) {
    QtTypesRegistration::registerAllQtTypes();
    
    QPointF point(1.5, 2.5);
    JsonValue json = TypeRegistry::instance().toJson(point);
    QPointF restored = TypeRegistry::instance().fromJson<QPointF>(json, QPointF());
    
    ASSERT_NEAR(point.x(), restored.x(), 0.0001);
    ASSERT_NEAR(point.y(), restored.y(), 0.0001);
}

TEST(QRectFSerialization) {
    QtTypesRegistration::registerAllQtTypes();
    
    QRectF rect(1.0, 2.0, 3.0, 4.0);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    QRectF restored = TypeRegistry::instance().fromJson<QRectF>(json, QRectF());
    
    ASSERT_NEAR(rect.x(), restored.x(), 0.0001);
    ASSERT_NEAR(rect.y(), restored.y(), 0.0001);
    ASSERT_NEAR(rect.width(), restored.width(), 0.0001);
    ASSERT_NEAR(rect.height(), restored.height(), 0.0001);
}

TEST(QRectSerialization) {
    QtTypesRegistration::registerAllQtTypes();
    
    QRect rect(10, 20, 30, 40);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    QRect restored = TypeRegistry::instance().fromJson<QRect>(json, QRect());
    
    ASSERT_EQ(rect.x(), restored.x());
    ASSERT_EQ(rect.y(), restored.y());
    ASSERT_EQ(rect.width(), restored.width());
    ASSERT_EQ(rect.height(), restored.height());
}

TEST(QColorSerialization) {
    QtTypesRegistration::registerAllQtTypes();
    
    QColor color(255, 128, 64, 32);
    JsonValue json = TypeRegistry::instance().toJson(color);
    QColor restored = TypeRegistry::instance().fromJson<QColor>(json, QColor());
    
    ASSERT_EQ(color.red(), restored.red());
    ASSERT_EQ(color.green(), restored.green());
    ASSERT_EQ(color.blue(), restored.blue());
    ASSERT_EQ(color.alpha(), restored.alpha());
}

TEST(QListQPointFSerialization) {
    QtTypesRegistration::registerAllQtTypes();
    
    QList<QPointF> points = {QPointF(1.1, 2.2), QPointF(3.3, 4.4)};
    JsonValue json = TypeRegistry::instance().toJson(points);
    QList<QPointF> restored = TypeRegistry::instance().fromJson<QList<QPointF>>(json, QList<QPointF>());
    
    ASSERT_EQ(points.size(), restored.size());
    for (int i = 0; i < points.size(); ++i) {
        ASSERT_NEAR(points[i].x(), restored[i].x(), 0.0001);
        ASSERT_NEAR(points[i].y(), restored[i].y(), 0.0001);
    }
}

TEST(JsonValueConversion) {
    JsonStruct::JsonValue jsonValue;
    jsonValue["key"] = JsonStruct::JsonValue("value");
    QJsonValue qJsonValue = JsonStruct::toQJsonValue(jsonValue);
    
    ASSERT_TRUE(qJsonValue.isObject());
    ASSERT_EQ("value", qJsonValue.toObject()["key"].toString().toStdString());
}

TEST(QJsonValueConversion) {
    QJsonValue qJsonValue = QJsonValue(QString("value"));
    JsonStruct::JsonValue jsonValue = JsonStruct::fromQJsonValue(qJsonValue);
    
    ASSERT_TRUE(jsonValue.isString());
    ASSERT_EQ("value", jsonValue.toString());
}

int main() {
    return RUN_ALL_TESTS();
}
