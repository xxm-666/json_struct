#include "../src/qt_types/qt_registry.h"
#include <iostream>
#include <QList>
#include <QVector>
#include <QPointF>
#include <QString>

using namespace JsonStruct;

int main() {
    // Test generic Qt container system
    
    // Test QList<int>
    QList<int> intList = {1, 2, 3, 4, 5};
    JsonValue jsonIntList = TypeRegistry::instance().toJson(intList);
    QList<int> restoredIntList = TypeRegistry::instance().fromJson<QList<int>>(jsonIntList, QList<int>());
    
    std::cout << "QList<int> test: ";
    for (int i : restoredIntList) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
    
    // Test QList<QString>
    QList<QString> stringList = {QString("hello"), QString("world"), QString("test")};
    JsonValue jsonStringList = TypeRegistry::instance().toJson(stringList);
    QList<QString> restoredStringList = TypeRegistry::instance().fromJson<QList<QString>>(jsonStringList, QList<QString>());
    
    std::cout << "QList<QString> test: ";
    for (const QString& s : restoredStringList) {
        std::cout << s.toStdString() << " ";
    }
    std::cout << std::endl;
    
    // Test QList<QPointF>
    QList<QPointF> pointList = {QPointF(1.1, 2.2), QPointF(3.3, 4.4), QPointF(5.5, 6.6)};
    JsonValue jsonPointList = TypeRegistry::instance().toJson(pointList);
    QList<QPointF> restoredPointList = TypeRegistry::instance().fromJson<QList<QPointF>>(jsonPointList, QList<QPointF>());
    
    std::cout << "QList<QPointF> test: ";
    for (const QPointF& p : restoredPointList) {
        std::cout << "(" << p.x() << "," << p.y() << ") ";
    }
    std::cout << std::endl;
    
    // Test QVector<double>
    QVector<double> doubleVector = {1.1, 2.2, 3.3};
    JsonValue jsonDoubleVector = TypeRegistry::instance().toJson(doubleVector);
    QVector<double> restoredDoubleVector = TypeRegistry::instance().fromJson<QVector<double>>(jsonDoubleVector, QVector<double>());
    
    std::cout << "QVector<double> test: ";
    for (double d : restoredDoubleVector) {
        std::cout << d << " ";
    }
    std::cout << std::endl;
    
    // Test custom container registration
    REGISTER_QT_CONTAINER(QList<QSize>);
    
    QList<QSize> sizeList = {QSize(10, 20), QSize(30, 40)};
    JsonValue jsonSizeList = TypeRegistry::instance().toJson(sizeList);
    QList<QSize> restoredSizeList = TypeRegistry::instance().fromJson<QList<QSize>>(jsonSizeList, QList<QSize>());
    
    std::cout << "QList<QSize> test: ";
    for (const QSize& s : restoredSizeList) {
        std::cout << "(" << s.width() << "x" << s.height() << ") ";
    }
    std::cout << std::endl;
    
    return 0;
}
