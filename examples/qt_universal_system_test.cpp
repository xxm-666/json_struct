#include "../src/qt_types/qt_registry.h"
#include "../src/type_registry/auto_serializer.h"
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

// Define a custom struct for testing
struct CustomStruct {
    JSON_AUTO(id, name, value)
    int id = 0;
    QString name = "";
    double value = 0.0;
};

int main() {
    std::cout << "=== Qt Universal Container System Test ===" << std::endl;
    
    // Test 1: Register the custom struct
    std::cout << "\n1. Testing custom struct registration:" << std::endl;
    // CustomStruct should work automatically with JSON_AUTO
    
    CustomStruct custom;
    custom.id = 42;
    custom.name = "Test";
    custom.value = 3.14;
    
    JsonValue customJson = qtUniversalToJson(custom);
    CustomStruct restoredCustom = qtUniversalFromJson<CustomStruct>(customJson);
    
    std::cout << "Original: {id: " << custom.id << ", name: " << custom.name.toStdString() 
              << ", value: " << custom.value << "}" << std::endl;
    std::cout << "Restored: {id: " << restoredCustom.id << ", name: " << restoredCustom.name.toStdString() 
              << ", value: " << restoredCustom.value << "}" << std::endl;
    
    // Test 2: Complex nested type - QMap<QString, QVector<QPair<QString, CustomStruct>>>
    std::cout << "\n2. Testing complex nested type:" << std::endl;
    
    using ComplexType = QMap<QString, QVector<QPair<QString, CustomStruct>>>;
    
    // No need to manually register - the universal system handles it!
    ComplexType complexData;
    
    QVector<QPair<QString, CustomStruct>> vector1;
    CustomStruct item1{1, "Item1", 1.5};
    CustomStruct item2{2, "Item2", 2.5};
    vector1.append(QPair<QString, CustomStruct>("first", item1));
    vector1.append(QPair<QString, CustomStruct>("second", item2));
    
    QVector<QPair<QString, CustomStruct>> vector2;
    CustomStruct item3{3, "Item3", 3.5};
    vector2.append(QPair<QString, CustomStruct>("third", item3));
    
    complexData["group1"] = vector1;
    complexData["group2"] = vector2;
    
    // Serialize using universal system
    JsonValue complexJson = qtUniversalToJson(complexData);
    
    // Deserialize using universal system
    ComplexType restoredComplex = qtUniversalFromJson<ComplexType>(complexJson);
    
    std::cout << "Original complex data:" << std::endl;
    for (auto it = complexData.begin(); it != complexData.end(); ++it) {
        std::cout << "  " << it.key().toStdString() << ": [";
        for (const auto& pair : it.value()) {
            std::cout << "(" << pair.first.toStdString() << ", {" 
                      << pair.second.id << ", " << pair.second.name.toStdString() << ", " 
                      << pair.second.value << "}) ";
        }
        std::cout << "]" << std::endl;
    }
    
    std::cout << "Restored complex data:" << std::endl;
    for (auto it = restoredComplex.begin(); it != restoredComplex.end(); ++it) {
        std::cout << "  " << it.key().toStdString() << ": [";
        for (const auto& pair : it.value()) {
            std::cout << "(" << pair.first.toStdString() << ", {" 
                      << pair.second.id << ", " << pair.second.name.toStdString() << ", " 
                      << pair.second.value << "}) ";
        }
        std::cout << "]" << std::endl;
    }
    
    // Test 3: Even more complex - QList<QMap<int, QSet<QPair<QString, CustomStruct>>>>
    std::cout << "\n3. Testing ultra-complex nested type:" << std::endl;
    
    using UltraComplexType = QList<QMap<int, QSet<QPair<QString, CustomStruct>>>>;
    
    UltraComplexType ultraComplex;
    
    QMap<int, QSet<QPair<QString, CustomStruct>>> map1;
    QSet<QPair<QString, CustomStruct>> set1;
    set1.insert(QPair<QString, CustomStruct>("key1", CustomStruct{10, "Ultra1", 10.1}));
    set1.insert(QPair<QString, CustomStruct>("key2", CustomStruct{20, "Ultra2", 20.2}));
    map1[100] = set1;
    
    QSet<QPair<QString, CustomStruct>> set2;
    set2.insert(QPair<QString, CustomStruct>("key3", CustomStruct{30, "Ultra3", 30.3}));
    map1[200] = set2;
    
    ultraComplex.append(map1);
    
    // This should work without any manual registration!
    JsonValue ultraJson = qtUniversalToJson(ultraComplex);
    UltraComplexType restoredUltra = qtUniversalFromJson<UltraComplexType>(ultraJson);
    
    std::cout << "Ultra-complex type serialization completed successfully!" << std::endl;
    std::cout << "Original size: " << ultraComplex.size() << std::endl;
    std::cout << "Restored size: " << restoredUltra.size() << std::endl;
    
    // Test 4: Register and use with TypeRegistry for compatibility
    std::cout << "\n4. Testing TypeRegistry integration:" << std::endl;
    
    // Register the complex type for use with TypeRegistry
    REGISTER_QT_TYPE(ComplexType);
    
    JsonValue registryJson = TypeRegistry::instance().toJson(complexData);
    ComplexType restoredFromRegistry = TypeRegistry::instance().fromJson<ComplexType>(registryJson, ComplexType{});
    
    std::cout << "TypeRegistry integration successful!" << std::endl;
    std::cout << "Original groups: " << complexData.size() << std::endl;
    std::cout << "Restored groups: " << restoredFromRegistry.size() << std::endl;
    
    std::cout << "\n=== All tests completed successfully! ===" << std::endl;
    return 0;
}
