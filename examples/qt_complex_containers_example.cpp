#include "../src/qt_types/qt_registry.h"
#include <iostream>
#include <QList>
#include <QVector>
#include <QSet>
#include <QMap>
#include <QHash>
#include <QPair>
#include <QPointF>
#include <QString>

using namespace JsonStruct;

int main() {
    std::cout << "=== Qt Complex Container Test ===" << std::endl;
    
    // Test QSet<int>
    std::cout << "\n1. Testing QSet<int>:" << std::endl;
    QSet<int> intSet = {1, 2, 3, 4, 5};
    JsonValue jsonIntSet = TypeRegistry::instance().toJson(intSet);
    QSet<int> restoredIntSet = TypeRegistry::instance().fromJson<QSet<int>>(jsonIntSet, QSet<int>());
    
    std::cout << "Original: ";
    for (int i : intSet) std::cout << i << " ";
    std::cout << "\nRestored: ";
    for (int i : restoredIntSet) std::cout << i << " ";
    std::cout << std::endl;
    
    // Test QMap<QString, int>
    std::cout << "\n2. Testing QMap<QString, int>:" << std::endl;
    QMap<QString, int> stringIntMap;
    stringIntMap["one"] = 1;
    stringIntMap["two"] = 2;
    stringIntMap["three"] = 3;
    
    JsonValue jsonStringIntMap = TypeRegistry::instance().toJson(stringIntMap);
    QMap<QString, int> restoredStringIntMap = TypeRegistry::instance().fromJson<QMap<QString, int>>(jsonStringIntMap, QMap<QString, int>());
    
    std::cout << "Original: ";
    for (auto it = stringIntMap.begin(); it != stringIntMap.end(); ++it) {
        std::cout << "(" << it.key().toStdString() << ":" << it.value() << ") ";
    }
    std::cout << "\nRestored: ";
    for (auto it = restoredStringIntMap.begin(); it != restoredStringIntMap.end(); ++it) {
        std::cout << "(" << it.key().toStdString() << ":" << it.value() << ") ";
    }
    std::cout << std::endl;
    
    // Test QMap<int, QString> (non-string key)
    std::cout << "\n3. Testing QMap<int, QString>:" << std::endl;
    QMap<int, QString> intStringMap;
    intStringMap[1] = "one";
    intStringMap[2] = "two";
    intStringMap[3] = "three";
    
    JsonValue jsonIntStringMap = TypeRegistry::instance().toJson(intStringMap);
    QMap<int, QString> restoredIntStringMap = TypeRegistry::instance().fromJson<QMap<int, QString>>(jsonIntStringMap, QMap<int, QString>());
    
    std::cout << "Original: ";
    for (auto it = intStringMap.begin(); it != intStringMap.end(); ++it) {
        std::cout << "(" << it.key() << ":" << it.value().toStdString() << ") ";
    }
    std::cout << "\nRestored: ";
    for (auto it = restoredIntStringMap.begin(); it != restoredIntStringMap.end(); ++it) {
        std::cout << "(" << it.key() << ":" << it.value().toStdString() << ") ";
    }
    std::cout << std::endl;
    
    // Test QHash<QString, QPointF>
    std::cout << "\n4. Testing QHash<QString, QPointF>:" << std::endl;
    QHash<QString, QPointF> stringPointHash;
    stringPointHash["point1"] = QPointF(1.1, 2.2);
    stringPointHash["point2"] = QPointF(3.3, 4.4);
    
    JsonValue jsonStringPointHash = TypeRegistry::instance().toJson(stringPointHash);
    QHash<QString, QPointF> restoredStringPointHash = TypeRegistry::instance().fromJson<QHash<QString, QPointF>>(jsonStringPointHash, QHash<QString, QPointF>());
    
    std::cout << "Original: ";
    for (auto it = stringPointHash.begin(); it != stringPointHash.end(); ++it) {
        std::cout << "(" << it.key().toStdString() << ":(" << it.value().x() << "," << it.value().y() << ")) ";
    }
    std::cout << "\nRestored: ";
    for (auto it = restoredStringPointHash.begin(); it != restoredStringPointHash.end(); ++it) {
        std::cout << "(" << it.key().toStdString() << ":(" << it.value().x() << "," << it.value().y() << ")) ";
    }
    std::cout << std::endl;
    
    // Test QPair<QString, QPointF>
    std::cout << "\n5. Testing QPair<QString, QPointF>:" << std::endl;
    QPair<QString, QPointF> stringPointPair("center", QPointF(5.5, 6.6));
    JsonValue jsonStringPointPair = TypeRegistry::instance().toJson(stringPointPair);
    QPair<QString, QPointF> restoredStringPointPair = TypeRegistry::instance().fromJson<QPair<QString, QPointF>>(jsonStringPointPair, QPair<QString, QPointF>());
    
    std::cout << "Original: (" << stringPointPair.first.toStdString() << ", (" << stringPointPair.second.x() << "," << stringPointPair.second.y() << "))" << std::endl;
    std::cout << "Restored: (" << restoredStringPointPair.first.toStdString() << ", (" << restoredStringPointPair.second.x() << "," << restoredStringPointPair.second.y() << "))" << std::endl;
    
    // Test nested containers
    std::cout << "\n6. Testing nested containers:" << std::endl;
    
    // Register and test QList<QPair<QString, int>>
    REGISTER_QT_CONTAINER(QList<QPair<QString, int>>);
    
    QList<QPair<QString, int>> pairList;
    pairList.append(QPair<QString, int>("first", 1));
    pairList.append(QPair<QString, int>("second", 2));
    
    JsonValue jsonPairList = TypeRegistry::instance().toJson(pairList);
    QList<QPair<QString, int>> restoredPairList = TypeRegistry::instance().fromJson<QList<QPair<QString, int>>>(jsonPairList, QList<QPair<QString, int>>());
    
    std::cout << "QList<QPair<QString, int>>:" << std::endl;
    std::cout << "Original: ";
    for (const auto& pair : pairList) {
        std::cout << "(" << pair.first.toStdString() << ":" << pair.second << ") ";
    }
    std::cout << "\nRestored: ";
    for (const auto& pair : restoredPairList) {
        std::cout << "(" << pair.first.toStdString() << ":" << pair.second << ") ";
    }
    std::cout << std::endl;
    
    // Register and test QMap<QString, QList<int>>
    REGISTER_QT_CONTAINER(QMap<QString, QList<int>>);
    
    QMap<QString, QList<int>> stringListMap;
    stringListMap["numbers1"] = QList<int>{1, 2, 3};
    stringListMap["numbers2"] = QList<int>{4, 5, 6};
    
    JsonValue jsonStringListMap = TypeRegistry::instance().toJson(stringListMap);
    QMap<QString, QList<int>> restoredStringListMap = TypeRegistry::instance().fromJson<QMap<QString, QList<int>>>(jsonStringListMap, QMap<QString, QList<int>>());
    
    std::cout << "\nQMap<QString, QList<int>>:" << std::endl;
    std::cout << "Original: ";
    for (auto it = stringListMap.begin(); it != stringListMap.end(); ++it) {
        std::cout << it.key().toStdString() << ":[";
        for (int i : it.value()) std::cout << i << " ";
        std::cout << "] ";
    }
    std::cout << "\nRestored: ";
    for (auto it = restoredStringListMap.begin(); it != restoredStringListMap.end(); ++it) {
        std::cout << it.key().toStdString() << ":[";
        for (int i : it.value()) std::cout << i << " ";
        std::cout << "] ";
    }
    std::cout << std::endl;
    
    std::cout << "\n=== All tests completed! ===" << std::endl;
    return 0;
}
