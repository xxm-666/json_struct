#include <test_framework/test_framework.h>
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
#include <QSet>
#include <QMap>
#include <QHash>
#include <QPair>
#include <QVector>

// QString stream operator for testing
inline std::ostream& operator<<(std::ostream& os, const QString& str) {
    return os << str.toStdString();
}

using namespace JsonStruct;

struct ComplexA
{
    QString name;
    int value;
    QList<int> sisters;
    JSON_AUTO(name, value, sisters)
};

struct WindowConfig {
    QString title = "My Window";
    QPointF position = {100.0, 200.0};
    QRectF geometry = {0, 0, 800, 600};
    QStringList recentFiles = {"file1.txt", "file2.txt"};
    QColor backgroundColor = QColor(255, 255, 255);

    JSON_FIELDS(title, position, geometry, recentFiles, backgroundColor)
};

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

TEST(QtTypes_GenericContainers) {
    // Test QList<int>
    QList<int> intList = {1, 2, 3, 4, 5};
    JsonValue jsonIntList = TypeRegistry::instance().toJson(intList);
    QList<int> restoredIntList = TypeRegistry::instance().fromJson<QList<int>>(jsonIntList, QList<int>());
    
    ASSERT_EQ(restoredIntList.size(), intList.size());
    for (int i = 0; i < intList.size(); ++i) {
        ASSERT_EQ(restoredIntList[i], intList[i]);
    }
    
    // Test QVector<double>
    QVector<double> doubleVector = {1.1, 2.2, 3.3};
    JsonValue jsonDoubleVector = TypeRegistry::instance().toJson(doubleVector);
    QVector<double> restoredDoubleVector = TypeRegistry::instance().fromJson<QVector<double>>(jsonDoubleVector, QVector<double>());
    
    ASSERT_EQ(restoredDoubleVector.size(), doubleVector.size());
    for (int i = 0; i < doubleVector.size(); ++i) {
        ASSERT_NEAR(restoredDoubleVector[i], doubleVector[i], 0.001);
    }
    
    // Test QList<QString>
    QList<QString> stringList = {QString("hello"), QString("world"), QString("test")};
    JsonValue jsonStringList = TypeRegistry::instance().toJson(stringList);
    QList<QString> restoredStringList = TypeRegistry::instance().fromJson<QList<QString>>(jsonStringList, QList<QString>());
    
    ASSERT_EQ(restoredStringList.size(), stringList.size());
    for (int i = 0; i < stringList.size(); ++i) {
        ASSERT_EQ(restoredStringList[i].toStdString(), stringList[i].toStdString());
    }
}

TEST(QtTypes_CustomContainerRegistration) {
    // Test custom container registration using the new universal system
    REGISTER_QT_TYPE(QList<QSize>);
    
    QList<QSize> sizeList = {QSize(10, 20), QSize(30, 40)};
    JsonValue jsonSizeList = TypeRegistry::instance().toJson(sizeList);
    QList<QSize> restoredSizeList = TypeRegistry::instance().fromJson<QList<QSize>>(jsonSizeList, QList<QSize>());
    
    ASSERT_EQ(restoredSizeList.size(), sizeList.size());
    for (int i = 0; i < sizeList.size(); ++i) {
        ASSERT_EQ(restoredSizeList[i].width(), sizeList[i].width());
        ASSERT_EQ(restoredSizeList[i].height(), sizeList[i].height());
    }
    
    // Test nested containers
    REGISTER_QT_TYPE(QList<QList<int>>);
    
    QList<QList<int>> nestedList = {{1, 2}, {3, 4, 5}, {6}};
    JsonValue jsonNestedList = TypeRegistry::instance().toJson(nestedList);
    QList<QList<int>> restoredNestedList = TypeRegistry::instance().fromJson<QList<QList<int>>>(jsonNestedList, QList<QList<int>>());
    
    ASSERT_EQ(restoredNestedList.size(), nestedList.size());
    for (int i = 0; i < nestedList.size(); ++i) {
        ASSERT_EQ(restoredNestedList[i].size(), nestedList[i].size());
        for (int j = 0; j < nestedList[i].size(); ++j) {
            ASSERT_EQ(restoredNestedList[i][j], nestedList[i][j]);
        }
    }
}

TEST(QtTypes_ComplexContainers) {
    // Test QSet<QString>
    QSet<QString> stringSet = {"apple", "banana", "cherry"};
    JsonValue jsonStringSet = TypeRegistry::instance().toJson(stringSet);
    QSet<QString> restoredStringSet = TypeRegistry::instance().fromJson<QSet<QString>>(jsonStringSet, QSet<QString>());
    
    ASSERT_EQ(restoredStringSet.size(), stringSet.size());
    for (const QString& str : stringSet) {
        ASSERT_TRUE(restoredStringSet.contains(str));
    }
    
    // Test QMap<QString, int>
    QMap<QString, int> stringIntMap;
    stringIntMap["one"] = 1;
    stringIntMap["two"] = 2;
    stringIntMap["three"] = 3;
    
    JsonValue jsonStringIntMap = TypeRegistry::instance().toJson(stringIntMap);
    QMap<QString, int> restoredStringIntMap = TypeRegistry::instance().fromJson<QMap<QString, int>>(jsonStringIntMap, QMap<QString, int>());
    
    ASSERT_EQ(restoredStringIntMap.size(), stringIntMap.size());
    for (auto it = stringIntMap.begin(); it != stringIntMap.end(); ++it) {
        ASSERT_EQ(restoredStringIntMap[it.key()], it.value());
    }
    
    // Test QMap<int, QString> (non-string key)
    QMap<int, QString> intStringMap;
    intStringMap[1] = "one";
    intStringMap[2] = "two";
    intStringMap[3] = "three";
    
    JsonValue jsonIntStringMap = TypeRegistry::instance().toJson(intStringMap);
    QMap<int, QString> restoredIntStringMap = TypeRegistry::instance().fromJson<QMap<int, QString>>(jsonIntStringMap, QMap<int, QString>());
    
    ASSERT_EQ(restoredIntStringMap.size(), intStringMap.size());
    for (auto it = intStringMap.begin(); it != intStringMap.end(); ++it) {
        ASSERT_EQ(restoredIntStringMap[it.key()].toStdString(), it.value().toStdString());
    }
    
    // Test QHash<QString, QPointF>
    QHash<QString, QPointF> stringPointHash;
    stringPointHash["point1"] = QPointF(1.1, 2.2);
    stringPointHash["point2"] = QPointF(3.3, 4.4);
    
    JsonValue jsonStringPointHash = TypeRegistry::instance().toJson(stringPointHash);
    QHash<QString, QPointF> restoredStringPointHash = TypeRegistry::instance().fromJson<QHash<QString, QPointF>>(jsonStringPointHash, QHash<QString, QPointF>());
    
    ASSERT_EQ(restoredStringPointHash.size(), stringPointHash.size());
    for (auto it = stringPointHash.begin(); it != stringPointHash.end(); ++it) {
        QPointF restored = restoredStringPointHash[it.key()];
        ASSERT_NEAR(restored.x(), it.value().x(), 0.001);
        ASSERT_NEAR(restored.y(), it.value().y(), 0.001);
    }
    
    // Test QPair<QString, QPointF>
    QPair<QString, QPointF> stringPointPair("center", QPointF(5.5, 6.6));
    JsonValue jsonStringPointPair = TypeRegistry::instance().toJson(stringPointPair);
    QPair<QString, QPointF> restoredStringPointPair = TypeRegistry::instance().fromJson<QPair<QString, QPointF>>(jsonStringPointPair, QPair<QString, QPointF>());
    
    ASSERT_EQ(restoredStringPointPair.first.toStdString(), stringPointPair.first.toStdString());
    ASSERT_NEAR(restoredStringPointPair.second.x(), stringPointPair.second.x(), 0.001);
    ASSERT_NEAR(restoredStringPointPair.second.y(), stringPointPair.second.y(), 0.001);
}

TEST(QtTypes_NestedComplexContainers) {
    // Test QList<QPair<QString, int>>
    // REGISTER_QT_TYPE(QList<QPair<QString, int>>);
    registerCustomQtType<QList<QPair<QString, int>>>();
    
    QList<QPair<QString, int>> pairList;
    pairList.append(QPair<QString, int>("first", 1));
    pairList.append(QPair<QString, int>("second", 2));
    pairList.append(QPair<QString, int>("third", 3));
    
    JsonValue jsonPairList = TypeRegistry::instance().toJson(pairList);
    QList<QPair<QString, int>> restoredPairList = TypeRegistry::instance().fromJson<QList<QPair<QString, int>>>(jsonPairList, QList<QPair<QString, int>>());
    
    ASSERT_EQ(restoredPairList.size(), pairList.size());
    for (int i = 0; i < pairList.size(); ++i) {
        ASSERT_EQ(restoredPairList[i].first.toStdString(), pairList[i].first.toStdString());
        ASSERT_EQ(restoredPairList[i].second, pairList[i].second);
    }
    
    // Test QMap<QString, QList<int>>
    registerCustomQtType<QMap<QString, QList<int>>>();
    
    QMap<QString, QList<int>> stringListMap;
    stringListMap["numbers1"] = QList<int>{1, 2, 3};
    stringListMap["numbers2"] = QList<int>{4, 5, 6};
    stringListMap["numbers3"] = QList<int>{7, 8, 9};
    
    JsonValue jsonStringListMap = TypeRegistry::instance().toJson(stringListMap);
    QMap<QString, QList<int>> restoredStringListMap = TypeRegistry::instance().fromJson<QMap<QString, QList<int>>>(jsonStringListMap, QMap<QString, QList<int>>());
    
    ASSERT_EQ(restoredStringListMap.size(), stringListMap.size());
    for (auto it = stringListMap.begin(); it != stringListMap.end(); ++it) {
        QList<int> restored = restoredStringListMap[it.key()];
        ASSERT_EQ(restored.size(), it.value().size());
        for (int i = 0; i < it.value().size(); ++i) {
            ASSERT_EQ(restored[i], it.value()[i]);
        }
    }
    
    // Test QSet<QPair<int, int>>
    // REGISTER_QT_TYPE(QSet<QPair<int, int>>);
    registerCustomQtType<QSet<QPair<int, int>>>();
    
    QSet<QPair<int, int>> pairSet;
    pairSet.insert(QPair<int, int>(1, 2));
    pairSet.insert(QPair<int, int>(3, 4));
    pairSet.insert(QPair<int, int>(5, 6));
    
    JsonValue jsonPairSet = TypeRegistry::instance().toJson(pairSet);
    QSet<QPair<int, int>> restoredPairSet = TypeRegistry::instance().fromJson<QSet<QPair<int, int>>>(jsonPairSet, QSet<QPair<int, int>>());
    
    ASSERT_EQ(restoredPairSet.size(), pairSet.size());
    for (const auto& pair : pairSet) {
        ASSERT_TRUE(restoredPairSet.contains(pair));
    }
    
    // Test deeply nested: QMap<QString, QMap<int, QList<QPointF>>>
    // REGISTER_QT_TYPE(QMap<int, QList<QPointF>>);
    // REGISTER_QT_TYPE(QMap<QString, QMap<int, QList<QPointF>>>);
    registerCustomQtType<QMap<int, QList<QPointF>>>();
    registerCustomQtType<QMap<QString, QMap<int, QList<QPointF>>>>();

    QMap<QString, QMap<int, QList<QPointF>>> deepNestedMap;
    QMap<int, QList<QPointF>> innerMap1;
    innerMap1[1] = QList<QPointF>{QPointF(1.1, 1.2), QPointF(1.3, 1.4)};
    innerMap1[2] = QList<QPointF>{QPointF(2.1, 2.2)};
    deepNestedMap["group1"] = innerMap1;
    
    QMap<int, QList<QPointF>> innerMap2;
    innerMap2[3] = QList<QPointF>{QPointF(3.1, 3.2), QPointF(3.3, 3.4), QPointF(3.5, 3.6)};
    deepNestedMap["group2"] = innerMap2;
    
    JsonValue jsonDeepNested = TypeRegistry::instance().toJson(deepNestedMap);
    QMap<QString, QMap<int, QList<QPointF>>> restoredDeepNested = TypeRegistry::instance().fromJson<QMap<QString, QMap<int, QList<QPointF>>>>(jsonDeepNested, QMap<QString, QMap<int, QList<QPointF>>>());
    
    ASSERT_EQ(restoredDeepNested.size(), deepNestedMap.size());
    for (auto outerIt = deepNestedMap.begin(); outerIt != deepNestedMap.end(); ++outerIt) {
        ASSERT_TRUE(restoredDeepNested.contains(outerIt.key()));
        QMap<int, QList<QPointF>> restoredInner = restoredDeepNested[outerIt.key()];
        ASSERT_EQ(restoredInner.size(), outerIt.value().size());
        
        for (auto innerIt = outerIt.value().begin(); innerIt != outerIt.value().end(); ++innerIt) {
            ASSERT_TRUE(restoredInner.contains(innerIt.key()));
            QList<QPointF> restoredList = restoredInner[innerIt.key()];
            ASSERT_EQ(restoredList.size(), innerIt.value().size());
            
            for (int i = 0; i < innerIt.value().size(); ++i) {
                ASSERT_NEAR(restoredList[i].x(), innerIt.value()[i].x(), 0.001);
                ASSERT_NEAR(restoredList[i].y(), innerIt.value()[i].y(), 0.001);
            }
        }
    }
}
TEST(QtTypes_ComplexA) {
    // Initialize Qt registry first
    QtTypesRegistration::registerAllQtTypes();
    
    // First test if QString works
    QString testString = "Test String";
    JsonValue jsonString = TypeRegistry::instance().toJson(testString);
    QString restoredString = TypeRegistry::instance().fromJson<QString>(jsonString, QString());
    std::cout << "QString test - Original: " << testString.toStdString() 
              << ", Restored: " << restoredString.toStdString() << std::endl;
    ASSERT_EQ(restoredString, testString);
    
    // Test if QList<int> is properly registered
    std::cout << "Testing if QList<int> is registered: " 
              << TypeRegistry::instance().isRegistered<QList<int>>() << std::endl;
    
    // First test QList<int> separately
    QList<int> testList = {1, 2, 3, 4, 5};
    JsonValue jsonList = TypeRegistry::instance().toJson(testList);
    QList<int> restoredList = TypeRegistry::instance().fromJson<QList<int>>(jsonList, QList<int>());
    
    std::cout << "QList<int> test - Original size: " << testList.size() 
              << ", Restored size: " << restoredList.size() << std::endl;
    std::cout << "QList<int> JSON: " << jsonList.toString() << std::endl;
    
    ASSERT_EQ(restoredList.size(), testList.size());
    for (int i = 0; i < testList.size(); ++i) {
        ASSERT_EQ(restoredList[i], testList[i]);
    }
    
    // Now test ComplexA
    // First, register ComplexA type using auto-serialization
    // TypeRegistry::instance().registerType<ComplexA>(
    //     [](const ComplexA& obj) -> JsonValue {
    //         return obj.toJson();
    //     },
    //     [](const JsonValue& json, const ComplexA& defaultValue) -> ComplexA {
    //         ComplexA obj = defaultValue;
    //         obj.fromJson(json);
    //         return obj;
    //     }
    // );
    
    ComplexA a;
    a.name = "Test ComplexA";
    a.value = 42;
    a.sisters = {1, 2, 3, 4, 5};
    
    // Check if ComplexA is registered (it should be auto-registered by JSON_AUTO)
    std::cout << "Testing if ComplexA is registered: "
              << TypeRegistry::instance().isRegistered<ComplexA>() << std::endl;
    
    // JsonValue json = TypeRegistry::instance().toJson(a);
    JsonValue json = JsonValue(toJson(a));
    std::cout << "ComplexA JSON: " << json.toString() << std::endl;
    
    // ComplexA restored;
    auto restored = fromJson<ComplexA>(json);
    // ComplexA restored = TypeRegistry::instance().fromJson<ComplexA>(json, ComplexA());
    
    std::cout << "Original name: " << a.name.toStdString() << std::endl;
    std::cout << "Restored name: " << restored.name.toStdString() << std::endl;
    std::cout << "Original value: " << a.value << std::endl;
    std::cout << "Restored value: " << restored.value << std::endl;
    std::cout << "Original sisters size: " << a.sisters.size() << std::endl;
    std::cout << "Restored sisters size: " << restored.sisters.size() << std::endl;
    
    ASSERT_EQ(restored.name, a.name);
    ASSERT_EQ(restored.value, a.value);
    ASSERT_EQ(restored.sisters.size(), a.sisters.size());
    for (int i = 0; i < a.sisters.size(); ++i) {
        ASSERT_EQ(restored.sisters[i], a.sisters[i]);
    }
}
TEST(QtTypes_QListInt) {
    QList<int> testList = {1, 2, 3, 4, 5};
    
    std::cout << "Testing QList<int> serialization..." << std::endl;
    
    JsonValue json = TypeRegistry::instance().toJson(testList);
    std::cout << "QList<int> JSON: " << json.toString() << std::endl;
    
    QList<int> restored = TypeRegistry::instance().fromJson<QList<int>>(json, QList<int>());
    
    std::cout << "Original size: " << testList.size() << std::endl;
    std::cout << "Restored size: " << restored.size() << std::endl;
    
    ASSERT_EQ(restored.size(), testList.size());
    for (int i = 0; i < testList.size(); ++i) {
        std::cout << "Original[" << i << "]: " << testList[i] << ", Restored[" << i << "]: " << restored[i] << std::endl;
        ASSERT_EQ(restored[i], testList[i]);
    }
}
TEST(QtTypes_WindowConfig) {
    WindowConfig config;
    config.title = "Main Window";
    config.position = QPointF(50.0, 100.7);
    config.geometry = QRectF(10, 20, 800, 600);
    config.recentFiles = QStringList{"file1.txt", "file2.txt", "file3.txt"};
    config.backgroundColor = QColor(200, 200, 200);

    std::cout << "Testing WindowConfig serialization..." << std::endl;

    JsonValue json = JsonValue(toJson(config));
    std::cout << "WindowConfig JSON: " << json.toJson(true) << std::endl;

    // WindowConfig restored = TypeRegistry::instance().fromJson<WindowConfig>(json, WindowConfig());
    auto restored = fromJson<WindowConfig>(json);
    std::cout << "Restored title: " << restored.title.toStdString() << std::endl;
    std::cout << "Restored position: (" << restored.position.x() << ", " << restored.position.y() << ")" << std::endl;
    std::cout << "Restored geometry: ("
              << restored.geometry.x() << ", "
              << restored.geometry.y() << ", "
              << restored.geometry.width() << ", "
              << restored.geometry.height() << ")" << std::endl;

    ASSERT_EQ(restored.title, config.title);
    ASSERT_NEAR(restored.position.x(), config.position.x(), 0.001);
    ASSERT_NEAR(restored.position.y(), config.position.y(), 0.001);
    ASSERT_NEAR(restored.geometry.x(), config.geometry.x(), 0.001);
    ASSERT_NEAR(restored.geometry.y(), config.geometry.y(), 0.001);
    ASSERT_NEAR(restored.geometry.width(), config.geometry.width(), 0.001);
    ASSERT_NEAR(restored.geometry.height(), config.geometry.height(), 0.001);

    ASSERT_EQ(restored.recentFiles.size(), config.recentFiles.size());
    for (int i = 0; i < config.recentFiles.size(); ++i) {
        ASSERT_EQ(restored.recentFiles[i], config.recentFiles[i]);
    }

    ASSERT_EQ(restored.backgroundColor.red(), config.backgroundColor.red());
    ASSERT_EQ(restored.backgroundColor.green(), config.backgroundColor.green());
    ASSERT_EQ(restored.backgroundColor.blue(), config.backgroundColor.blue());
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

TEST(QtTypes_UniversalSystem) {
    // Test the new universal system with simple types first
    std::cout << "Testing universal Qt type system..." << std::endl;
    
    // Test if basic registration check works
    std::cout << "QList<int> registered: " 
              << TypeRegistry::instance().isRegistered<QList<int>>() << std::endl;
    std::cout << "QString registered: " 
              << TypeRegistry::instance().isRegistered<QString>() << std::endl;
    
    std::cout << "Universal system test completed!" << std::endl;    
}

TEST(QtTypes_Deserialize) {
    std::string jsonStr = "{\"one\":1,\"two\":2}";
    auto json = JsonValue::parse(jsonStr);
    ASSERT_TRUE(json.isObject());
    ASSERT_EQ(json.toJson(), jsonStr);

    auto map = TypeRegistry::instance().fromJson<QMap<QString, int>>(json);

    ASSERT_EQ(map.size(), 2);
    ASSERT_EQ(map["one"], 1);
    ASSERT_EQ(map["two"], 2);
}
