#include "jsonstruct_std.h"
#include "std_custom_types_example.h"
#include <iostream>
#include <cassert>

using namespace JsonStruct;

// 测试基础JSON值功能
void testJsonValue() {
    std::cout << "Testing JsonValue..." << std::endl;
    
    // 测试基础类型
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
    
    // 测试数组
    JsonValue arrayVal;
    arrayVal.append(JsonValue(1));
    arrayVal.append(JsonValue(2));
    arrayVal.append(JsonValue(3));
    assert(arrayVal.isArray());
    assert(arrayVal.size() == 3);
    assert(arrayVal[0].toInt() == 1);
    assert(arrayVal[1].toInt() == 2);
    assert(arrayVal[2].toInt() == 3);
    
    // 测试对象
    JsonValue objectVal;
    objectVal["name"] = JsonValue("test");
    objectVal["value"] = JsonValue(100);
    assert(objectVal.isObject());
    assert(objectVal.contains("name"));
    assert(objectVal["name"].toString() == "test");
    assert(objectVal["value"].toInt() == 100);
    
    std::cout << "JsonValue tests passed!" << std::endl;
}

// 测试JSON序列化/反序列化
void testJsonSerialization() {
    std::cout << "Testing JSON serialization..." << std::endl;
    
    // 创建测试对象
    JsonValue::ObjectType obj;
    obj["name"] = JsonValue("Alice");
    obj["age"] = JsonValue(25);
    
    JsonValue::ArrayType hobbies;
    hobbies.push_back(JsonValue("reading"));
    hobbies.push_back(JsonValue("coding"));
    obj["hobbies"] = JsonValue(hobbies);
    
    JsonValue root(obj);
    
    // 序列化
    std::string jsonStr = root.dump(2);
    std::cout << "Serialized JSON:\n" << jsonStr << std::endl;
    
    // 反序列化
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

// 测试类型注册系统
void testTypeRegistry() {
    std::cout << "Testing TypeRegistry..." << std::endl;
    
    // 测试Point3D
    Point3D point(1.5, 2.5, 3.5);
    JsonValue pointJson = TypeRegistry::instance().toJson(point);
    assert(pointJson.isArray());
    assert(pointJson.size() == 3);
    assert(pointJson[0].toDouble() == 1.5);
    assert(pointJson[1].toDouble() == 2.5);
    assert(pointJson[2].toDouble() == 3.5);
    
    Point3D restoredPoint = TypeRegistry::instance().fromJson<Point3D>(pointJson, Point3D());
    assert(restoredPoint == point);
    
    // 测试UserInfo
    UserInfo user("Bob", 30);
    user.hobbies = {"gaming", "sports"};
    
    JsonValue userJson = TypeRegistry::instance().toJson(user);
    assert(userJson.isObject());
    assert(userJson["name"].toString() == "Bob");
    assert(userJson["age"].toInt() == 30);
    assert(userJson["hobbies"].isArray());
    assert(userJson["hobbies"].size() == 2);
    
    UserInfo restoredUser = TypeRegistry::instance().fromJson<UserInfo>(userJson, UserInfo());
    assert(restoredUser == user);
    
    // 测试ComplexData
    ComplexData complex;
    complex.position = Point3D(10, 20, 30);
    complex.user = user;
    complex.path = {Point3D(0,0,0), Point3D(5,5,5), Point3D(10,10,10)};
    
    JsonValue complexJson = TypeRegistry::instance().toJson(complex);
    std::string complexStr = complexJson.dump(2);
    std::cout << "Complex object JSON:\n" << complexStr << std::endl;
    
    ComplexData restoredComplex = TypeRegistry::instance().fromJson<ComplexData>(complexJson, ComplexData());
    assert(restoredComplex == complex);
    
    std::cout << "TypeRegistry tests passed!" << std::endl;
}

// 测试标准容器类型
void testStdTypes() {
    std::cout << "Testing standard container types..." << std::endl;
    
    // 测试std::vector<int>
    std::vector<int> intVec = {1, 2, 3, 4, 5};
    JsonValue intVecJson = TypeRegistry::instance().toJson(intVec);
    assert(intVecJson.isArray());
    assert(intVecJson.size() == 5);
    
    std::vector<int> restoredIntVec = TypeRegistry::instance().fromJson<std::vector<int>>(intVecJson, std::vector<int>());
    assert(restoredIntVec == intVec);
    
    // 测试std::vector<std::string>
    std::vector<std::string> stringVec = {"hello", "world", "test"};
    JsonValue stringVecJson = TypeRegistry::instance().toJson(stringVec);
    assert(stringVecJson.isArray());
    assert(stringVecJson.size() == 3);
    
    std::vector<std::string> restoredStringVec = TypeRegistry::instance().fromJson<std::vector<std::string>>(stringVecJson, std::vector<std::string>());
    assert(restoredStringVec == stringVec);
    
    // 测试std::map<std::string, int>
    std::map<std::string, int> intMap;
    intMap["first"] = 1;
    intMap["second"] = 2;
    intMap["third"] = 3;
    
    JsonValue intMapJson = TypeRegistry::instance().toJson(intMap);
    assert(intMapJson.isObject());
    assert(intMapJson["first"].toInt() == 1);
    assert(intMapJson["second"].toInt() == 2);
    assert(intMapJson["third"].toInt() == 3);
    
    std::map<std::string, int> restoredIntMap = TypeRegistry::instance().fromJson<std::map<std::string, int>>(intMapJson, std::map<std::string, int>());
    assert(restoredIntMap == intMap);
    
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
