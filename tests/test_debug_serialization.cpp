#include "../src/jsonstruct.h"
#include <iostream>
#include <string>

using namespace JsonStruct;

struct SimpleTest {
    int id = 100;
    std::string name = "original";
    std::vector<int> data = {1, 2, 3};
    
    JSON_AUTO(id, name, data)
};

int main() {
    std::cout << "=== Debug Auto-Serialization ===" << std::endl;
    
    SimpleTest obj;
    std::cout << "Original: id=" << obj.id << ", name=" << obj.name << ", data.size=" << obj.data.size() << std::endl;
    
    // Serialize
    std::cout << "\n1. Testing serialization..." << std::endl;
    auto jsonVal = obj.toJson();
    std::cout << "Serialization completed." << std::endl;
    
    // Check serialized content
    if (jsonVal.isObject()) {
        auto jsonObj = jsonVal.toObject();
        std::cout << "JSON object created with " << jsonObj.size() << " fields:" << std::endl;
        for (const auto& pair : jsonObj) {
            std::cout << "  " << pair.first << " = ";
            if (pair.second.isNumber()) {
                std::cout << pair.second.toInt();
            } else if (pair.second.isString()) {
                std::cout << "\"" << pair.second.toString() << "\"";
            } else {
                std::cout << "(other type)";
            }
            std::cout << std::endl;
        }
    }
    
    // Test deserialization
    std::cout << "\n2. Testing deserialization..." << std::endl;
    SimpleTest obj2;
    obj2.id = 999;
    obj2.name = "modified";
    obj2.data = {99, 88};
    std::cout << "Before deserialization: id=" << obj2.id << ", name=" << obj2.name << ", data.size=" << obj2.data.size() << std::endl;
    
    obj2.fromJson(jsonVal.toObject());
    std::cout << "After deserialization: id=" << obj2.id << ", name=" << obj2.name << ", data.size=" << obj2.data.size() << std::endl;
    
    // Test field access
    std::cout << "\n3. Testing field access..." << std::endl;
    auto fields = obj.json_fields();
    std::cout << "Field 0 (id): " << std::get<0>(fields) << std::endl;
    std::cout << "Field 1 (name): " << std::get<1>(fields) << std::endl;
    
    // Test field modification through tuple
    std::cout << "\n4. Testing field modification..." << std::endl;
    std::get<0>(fields) = 777;
    std::get<1>(fields) = "changed_through_tuple";
    std::cout << "After tuple modification: id=" << obj.id << ", name=" << obj.name << std::endl;
    
    return 0;
}
