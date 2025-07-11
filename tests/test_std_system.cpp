#include "jsonstruct.h"
// #include "std_types/custom_types_example.h"
#include <iostream>
#include <cassert>

#ifdef _WIN32
#include <crtdbg.h>
#ifdef _DEBUG
#define assert(expression) _ASSERTE(expression)
#endif
#endif

using namespace JsonStruct;

// Test basic JSON value functionality
void testJsonValue() {
    std::cout << "Testing JsonValue..." << std::endl;
    // Test basic types
    JsonValue nullVal;
    assert(nullVal.isNull());
    JsonValue boolVal(true);
    assert(boolVal.isBool());
    assert(boolVal.toBool() == true);
    JsonValue intVal(42);
    assert(intVal.isNumber());
    assert(intVal.toInt() == 42);
    JsonValue doubleVal(3.14);
    assert(doubleVal.isNumber());
    assert(doubleVal.toDouble() == 3.14);
    JsonValue stringVal("hello");
    assert(stringVal.isString());
    assert(stringVal.toString() == "hello");
    // Test array
    JsonValue arrayVal;
    arrayVal.append(JsonValue(1));
    arrayVal.append(JsonValue(2));
    arrayVal.append(JsonValue(3));
    assert(arrayVal.isArray());
    assert(arrayVal.size() == 3);
    assert(arrayVal[0].toInt() == 1);
    assert(arrayVal[1].toInt() == 2);
    assert(arrayVal[2].toInt() == 3);
    // Test object
    JsonValue objectVal;
    objectVal["name"] = JsonValue("test");
    objectVal["value"] = JsonValue(100);
    assert(objectVal.isObject());
    assert(objectVal.contains("name"));
    assert(objectVal["name"].toString() == "test");
    assert(objectVal["value"].toInt() == 100);
    std::cout << "JsonValue tests passed!" << std::endl;
}

// Test JSON serialization and deserialization
void testJsonSerialization() {
    std::cout << "Testing JSON serialization..." << std::endl;
    // Create test object
    JsonValue::ObjectType obj;
    obj["name"] = JsonValue("Alice");
    obj["age"] = JsonValue(25);
    JsonValue::ArrayType hobbies;
    hobbies.push_back(JsonValue("reading"));
    hobbies.push_back(JsonValue("coding"));
    obj["hobbies"] = JsonValue(hobbies);
    JsonValue root(obj);
    // Serialize
    std::string jsonStr = root.dump(2);
    std::cout << "Serialized JSON:\n" << jsonStr << std::endl;
    // Deserialize
    JsonValue parsed = JsonValue::parse(jsonStr);
    assert(parsed.isObject());
    assert(parsed["name"].toString() == "Alice");
    assert(parsed["age"].toInt() == 25);
    assert(parsed["hobbies"].isArray());
    assert(parsed["hobbies"].size() == 2);
    assert(parsed["hobbies"][0].toString() == "reading");
    assert(parsed["hobbies"][1].toString() == "coding");
    std::cout << "JSON serialization tests passed!" << std::endl;
}

// Test type registry system
void testTypeRegistry() {
    std::cout << "Testing TypeRegistry..." << std::endl;
    try {
        // Test Point3D
        std::cout << "Creating Point3D..." << std::endl;
        Point3D point(1.5, 2.5, 3.5);
        std::cout << "Converting to JSON..." << std::endl;
        JsonValue pointJson = TypeRegistry::instance().toJson(point);
        std::cout << "Checking if result is array..." << std::endl;
        assert(pointJson.isArray());
        assert(pointJson.size() == 3);
        assert(pointJson[0].toDouble() == 1.5);
        assert(pointJson[1].toDouble() == 2.5);
        assert(pointJson[2].toDouble() == 3.5);
        std::cout << "Converting back from JSON..." << std::endl;
        Point3D restoredPoint = TypeRegistry::instance().fromJson<Point3D>(pointJson, Point3D());
        assert(restoredPoint == point);
        std::cout << "Point3D tests passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception in TypeRegistry test: " << e.what() << std::endl;
        throw;
    }
    std::cout << "TypeRegistry tests passed!" << std::endl;
}

// Test standard container types
void testStdTypes() {
    std::cout << "Testing standard container types..." << std::endl;
    try {
        // Test std::vector<int>
        std::cout << "Testing std::vector<int>..." << std::endl;
        std::vector<int> intVec = {1, 2, 3, 4, 5};
        JsonValue intVecJson = TypeRegistry::instance().toJson(intVec);
        std::cout << "Converted to JSON, checking if array..." << std::endl;
        assert(intVecJson.isArray());
        std::cout << "Array size: " << intVecJson.toArray().size() << std::endl;
        assert(intVecJson.toArray().size() == 5);
        std::cout << "Converting back from JSON..." << std::endl;
        std::vector<int> restoredIntVec = TypeRegistry::instance().fromJson<std::vector<int>>(intVecJson, std::vector<int>());
        assert(restoredIntVec == intVec);
        std::cout << "std::vector<int> test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Exception in std::vector<int> test: " << e.what() << std::endl;
        throw;
    }
    std::cout << "Standard container types tests passed!" << std::endl;
}

int main() {
    try {
        std::cout << "=== Testing Standard C++ JsonStruct System ===" << std::endl;
        
        testJsonValue();
        testJsonSerialization();
        testTypeRegistry();
        testStdTypes();
        
        
        
        std::cout << "\n=== All tests passed! ===" << std::endl;
        std::cout << "\nThe standard C++ JSON framework is working correctly." << std::endl;
        std::cout << "- No Qt dependencies" << std::endl;
        std::cout << "- Type registry system functional" << std::endl;
        std::cout << "- Custom type registration working" << std::endl;
        std::cout << "- Standard container types supported" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
