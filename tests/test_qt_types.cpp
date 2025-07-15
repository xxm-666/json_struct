#include "type_registry/registry_core.h"
#include "qt_types/qt_registry.h"
#include <QStringList>
#include <QPointF>
#include <QRectF>
#include <QRect>
#include <QColor>
#include <QList>
#include <iostream>

using namespace JsonStruct;

void testQStringList() {
    QStringList list = {"one", "two", "three"};
    JsonValue json = TypeRegistry::instance().toJson(list);
    std::cout << "QStringList to JSON: " << json.toJson(2) << std::endl;

    QStringList restored = TypeRegistry::instance().fromJson<QStringList>(json, QStringList());
    std::cout << "Restored QStringList: " << restored.join(", ").toStdString() << std::endl;
}

void testQPointF() {
    QPointF point(1.5, 2.5);
    JsonValue json = TypeRegistry::instance().toJson(point);
    std::cout << "QPointF to JSON: " << json.toJson(2) << std::endl;

    QPointF restored = TypeRegistry::instance().fromJson<QPointF>(json, QPointF());
    std::cout << "Restored QPointF: (" << restored.x() << ", " << restored.y() << ")" << std::endl;
}

void testQRectF() {
    QRectF rect(1.0, 2.0, 3.0, 4.0);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    std::cout << "QRectF to JSON: " << json.toJson(2) << std::endl;

    QRectF restored = TypeRegistry::instance().fromJson<QRectF>(json, QRectF());
    std::cout << "Restored QRectF: (" << restored.x() << ", " << restored.y() << ", " << restored.width() << ", " << restored.height() << ")" << std::endl;
}

void testQColor() {
    QColor color(255, 128, 64, 32);
    JsonValue json = TypeRegistry::instance().toJson(color);
    std::cout << "QColor to JSON: " << json.toJson(2) << std::endl;

    QColor restored = TypeRegistry::instance().fromJson<QColor>(json, QColor());
    std::cout << "Restored QColor: (" << restored.red() << ", " << restored.green() << ", " << restored.blue() << ", " << restored.alpha() << ")" << std::endl;
}

void testQRect() {
    QRect rect(10, 20, 30, 40);
    JsonValue json = TypeRegistry::instance().toJson(rect);
    std::cout << "QRect to JSON: " << json.toJson(2) << std::endl;

    QRect restored = TypeRegistry::instance().fromJson<QRect>(json, QRect());
    std::cout << "Restored QRect: (" << restored.x() << ", " << restored.y() << ", " << restored.width() << ", " << restored.height() << ")" << std::endl;
}

void testQListQPointF() {
    QList<QPointF> points = {QPointF(1.1, 2.2), QPointF(3.3, 4.4)};
    JsonValue json = TypeRegistry::instance().toJson(points);
    std::cout << "QList<QPointF> to JSON: " << json.toJson(2) << std::endl;

    QList<QPointF> restored = TypeRegistry::instance().fromJson<QList<QPointF>>(json, QList<QPointF>());
    std::cout << "Restored QList<QPointF>: ";
    for (const auto& point : restored) {
        std::cout << "(" << point.x() << ", " << point.y() << ") ";
    }
    std::cout << std::endl;
}

int main() {
    JsonStruct::QtTypesRegistration::registerAllQtTypes();

    std::cout << "Testing QStringList..." << std::endl;
    testQStringList();

    std::cout << "\nTesting QPointF..." << std::endl;
    testQPointF();

    std::cout << "\nTesting QRectF..." << std::endl;
    testQRectF();

    std::cout << "\nTesting QRect..." << std::endl;
    testQRect();

    std::cout << "\nTesting QColor..." << std::endl;
    testQColor();

    std::cout << "\nTesting QList<QPointF>..." << std::endl;
    testQListQPointF();

    return 0;
}
