#include "../src/jsonstruct.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace JsonStruct;

struct PerformanceStruct {
    int id;
    std::string name;
    double value;
    std::vector<int> data;
    std::map<std::string, std::string> properties;
    
    JSON_AUTO(id, name, value, data, properties)
};

int main() {
    std::cout << "=== Debug PerformanceStruct ===" << std::endl;
    
    // Create test object
    PerformanceStruct obj;
    obj.id = 123;
    obj.name = "test_object";
    obj.value = 45.67;
    obj.data = {10, 20, 30};
    obj.properties = {{"key1", "value1"}, {"key2", "value2"}};
    
    std::cout << "Original object:" << std::endl;
    std::cout << "  id: " << obj.id << std::endl;
    std::cout << "  name: " << obj.name << std::endl;
    std::cout << "  value: " << obj.value << std::endl;
    std::cout << "  data size: " << obj.data.size() << std::endl;
    std::cout << "  properties size: " << obj.properties.size() << std::endl;
    
    // Test serialization
    std::cout << "\n1. Testing serialization..." << std::endl;
    try {
        auto jsonVal = obj.toJson();
        std::cout << "Serialization successful" << std::endl;
        
        if (jsonVal.isObject()) {
            auto jsonObj = jsonVal.toObject();
            std::cout << "JSON has " << jsonObj.size() << " fields:" << std::endl;
            for (const auto& pair : jsonObj) {
                std::cout << "  " << pair.first << ": ";
                if (pair.second.isNumber()) {
                    std::cout << "number";
                } else if (pair.second.isString()) {
                    std::cout << "string";
                } else if (pair.second.isArray()) {
                    std::cout << "array";
                } else if (pair.second.isObject()) {
                    std::cout << "object";
                } else {
                    std::cout << "other";
                }
                std::cout << std::endl;
            }
            
            // Test deserialization
            std::cout << "\n2. Testing deserialization..." << std::endl;
            PerformanceStruct obj2;
            obj2.id = 999;
            obj2.name = "changed";
            obj2.value = 888.888;
            obj2.data = {99};
            obj2.properties = {{"old", "data"}};
            
            std::cout << "Before deserialization:" << std::endl;
            std::cout << "  id: " << obj2.id << std::endl;
            std::cout << "  name: " << obj2.name << std::endl;
            std::cout << "  value: " << obj2.value << std::endl;
            std::cout << "  data size: " << obj2.data.size() << std::endl;
            std::cout << "  properties size: " << obj2.properties.size() << std::endl;
            
            obj2.fromJson(jsonVal);
            
            std::cout << "After deserialization:" << std::endl;
            std::cout << "  id: " << obj2.id << std::endl;
            std::cout << "  name: " << obj2.name << std::endl;
            std::cout << "  value: " << obj2.value << std::endl;
            std::cout << "  data size: " << obj2.data.size() << std::endl;
            std::cout << "  properties size: " << obj2.properties.size() << std::endl;
            
            // Compare results
            std::cout << "\n3. Comparison:" << std::endl;
            std::cout << "  id match: " << (obj.id == obj2.id ? "YES" : "NO") << std::endl;
            std::cout << "  name match: " << (obj.name == obj2.name ? "YES" : "NO") << std::endl;
            std::cout << "  value match: " << (obj.value == obj2.value ? "YES" : "NO") << std::endl;
            std::cout << "  data size match: " << (obj.data.size() == obj2.data.size() ? "YES" : "NO") << std::endl;
            std::cout << "  properties size match: " << (obj.properties.size() == obj2.properties.size() ? "YES" : "NO") << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
