#include "../src/jsonstruct.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <algorithm>

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
    std::cout << "=== Test Performance Pattern ===" << std::endl;
    
    const int iterations = 3; // Small number for debugging
    std::vector<PerformanceStruct> objects;
    objects.reserve(iterations);
    
    // 准备测试数据 - exactly like in comprehensive test
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    for (int i = 0; i < iterations; ++i) {
        PerformanceStruct obj;
        obj.id = i;
        obj.name = "Object_" + std::to_string(i);
        obj.value = i * 0.1;
        obj.data.resize(10);
        std::generate(obj.data.begin(), obj.data.end(), [&]() { return dis(gen); });
        obj.properties = {
            {"key1", "value1_" + std::to_string(i)},
            {"key2", "value2_" + std::to_string(i)},
            {"key3", "value3_" + std::to_string(i)}
        };
        objects.push_back(std::move(obj));
    }
    
    std::cout << "Created " << objects.size() << " test objects" << std::endl;
    for (size_t i = 0; i < objects.size(); ++i) {
        std::cout << "Object " << i << ": id=" << objects[i].id 
                  << ", name=" << objects[i].name 
                  << ", data.size=" << objects[i].data.size() << std::endl;
    }
    
    // 序列化性能测试 - exactly like in comprehensive test
    std::cout << "\nSerializing..." << std::endl;
    std::vector<JsonValue> serialized;
    serialized.reserve(iterations);
    
    for (const auto& obj : objects) {
        serialized.push_back(obj.toJson());
    }
    
    std::cout << "Serialized " << serialized.size() << " objects" << std::endl;
    
    // 反序列化性能测试 - exactly like in comprehensive test
    std::cout << "\nDeserializing..." << std::endl;
    std::vector<PerformanceStruct> deserialized;
    deserialized.reserve(iterations);
    
    for (const auto& json : serialized) {
        PerformanceStruct obj;
        obj.fromJson(json.toObject());
        deserialized.push_back(std::move(obj));
    }
    
    std::cout << "Deserialized " << deserialized.size() << " objects" << std::endl;
    for (size_t i = 0; i < deserialized.size(); ++i) {
        std::cout << "Deserialized " << i << ": id=" << deserialized[i].id 
                  << ", name=" << deserialized[i].name 
                  << ", data.size=" << deserialized[i].data.size() << std::endl;
    }
    
    // 验证数据一致性 - exactly like in comprehensive test
    std::cout << "\nValidating..." << std::endl;
    for (size_t i = 0; i < objects.size(); ++i) {
        bool id_match = (objects[i].id == deserialized[i].id);
        bool name_match = (objects[i].name == deserialized[i].name);
        bool data_size_match = (objects[i].data.size() == deserialized[i].data.size());
        
        std::cout << "Object " << i << " validation:" << std::endl;
        std::cout << "  ID: " << objects[i].id << " vs " << deserialized[i].id << " = " << (id_match ? "PASS" : "FAIL") << std::endl;
        std::cout << "  Name: '" << objects[i].name << "' vs '" << deserialized[i].name << "' = " << (name_match ? "PASS" : "FAIL") << std::endl;
        std::cout << "  Data size: " << objects[i].data.size() << " vs " << deserialized[i].data.size() << " = " << (data_size_match ? "PASS" : "FAIL") << std::endl;
        
        if (!id_match || !name_match || !data_size_match) {
            std::cout << "MISMATCH DETECTED!" << std::endl;
            return 1;
        }
    }
    
    std::cout << "\nAll validations PASSED!" << std::endl;
    return 0;
}
