#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"

// 只有在有Qt支持时才编译此文件
#ifdef QT_CORE_LIB
#include "../src/qt_types/qt_registry.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>

// QString stream operator for testing
inline std::ostream& operator<<(std::ostream& os, const QString& str) {
    return os << str.toStdString();
}

using namespace JsonStruct;

TEST(QtTypes_QStringList) {
    QStringList list = {"one", "two", "three"};
    JsonValue json = TypeRegistry::instance().toJson(list);
    QStringList restored = TypeRegistry::instance().fromJson<QStringList>(json, QStringList());
    
    ASSERT_EQ(restored.size(), list.size());
    for (int i = 0; i < list.size(); ++i) {
        ASSERT_EQ(restored[i], list[i]);
    }
}

TEST(QtTypes_QPointF) {
    QPointF point(1.5, 2.5);
    JsonValue json = TypeRegistry::instance().toJson(point);
    QPointF restored = TypeRegistry::instance().fromJson<QPointF>(json, QPointF());
    
    ASSERT_NEAR(restored.x(), point.x(), 0.001);
    ASSERT_NEAR(restored.y(), point.y(), 0.001);
}

TEST(QtTypes_QRectF) {
    QRectF rect(1.0, 2.0, 3.0, 4.0);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    QRectF restored = TypeRegistry::instance().fromJson<QRectF>(json, QRectF());
    
    ASSERT_NEAR(restored.x(), rect.x(), 0.001);
    ASSERT_NEAR(restored.y(), rect.y(), 0.001);
    ASSERT_NEAR(restored.width(), rect.width(), 0.001);
    ASSERT_NEAR(restored.height(), rect.height(), 0.001);
}

TEST(QtTypes_QColor) {
    QColor color(255, 128, 64, 32);
    JsonValue json = TypeRegistry::instance().toJson(color);
    QColor restored = TypeRegistry::instance().fromJson<QColor>(json, QColor());
    
    ASSERT_EQ(restored.red(), color.red());
    ASSERT_EQ(restored.green(), color.green());
    ASSERT_EQ(restored.blue(), color.blue());
    ASSERT_EQ(restored.alpha(), color.alpha());
}

TEST(QtTypes_QRect) {
    QRect rect(10, 20, 30, 40);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    QRect restored = TypeRegistry::instance().fromJson<QRect>(json, QRect());
    
    ASSERT_EQ(restored.x(), rect.x());
    ASSERT_EQ(restored.y(), rect.y());
    ASSERT_EQ(restored.width(), rect.width());
    ASSERT_EQ(restored.height(), rect.height());
}

TEST(QtTypes_QListQPointF) {
    QList<QPointF> points = {
        QPointF(1.0, 2.0),
        QPointF(3.0, 4.0),
        QPointF(5.0, 6.0)
    };
    
    JsonValue json = TypeRegistry::instance().toJson(points);
    QList<QPointF> restored = TypeRegistry::instance().fromJson<QList<QPointF>>(json, QList<QPointF>());
    
    ASSERT_EQ(restored.size(), points.size());
    for (int i = 0; i < points.size(); ++i) {
        ASSERT_NEAR(restored[i].x(), points[i].x(), 0.001);
        ASSERT_NEAR(restored[i].y(), points[i].y(), 0.001);
    }
}

TEST(QtTypes_ComplexStructureWithQt) {
    // 测试包含Qt类型的复杂结构
    struct QtStruct {
        JSON_AUTO(name, position, size, color)
        QString name = "Test Widget";
        QPointF position = QPointF(10.5, 20.5);
        QRectF size = QRectF(0, 0, 100, 50);
        QColor color = QColor(255, 0, 0);
    };
    
    QtStruct obj;
    obj.name = "Complex Test";
    obj.position = QPointF(15.5, 25.5);
    obj.size = QRectF(5, 10, 200, 100);
    obj.color = QColor(0, 255, 0, 128);
    
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    QtStruct restored;
    fromJson(restored, json);
    
    ASSERT_EQ(restored.name, obj.name);
    ASSERT_NEAR(restored.position.x(), obj.position.x(), 0.001);
    ASSERT_NEAR(restored.position.y(), obj.position.y(), 0.001);
    ASSERT_NEAR(restored.size.x(), obj.size.x(), 0.001);
    ASSERT_NEAR(restored.size.y(), obj.size.y(), 0.001);
    ASSERT_NEAR(restored.size.width(), obj.size.width(), 0.001);
    ASSERT_NEAR(restored.size.height(), obj.size.height(), 0.001);
    ASSERT_EQ(restored.color.red(), obj.color.red());
    ASSERT_EQ(restored.color.green(), obj.color.green());
    ASSERT_EQ(restored.color.blue(), obj.color.blue());
    ASSERT_EQ(restored.color.alpha(), obj.color.alpha());
}

#else
// 如果没有Qt支持，提供一个空测试
TEST(QtTypes_NotAvailable) {
    // Qt types not available in this build
    ASSERT_TRUE(true);
}
#endif

int main() {
#ifdef QT_CORE_LIB
    // Register Qt types before running tests
    QtTypesRegistration::registerAllQtTypes();
#endif
    return RUN_ALL_TESTS();
}
