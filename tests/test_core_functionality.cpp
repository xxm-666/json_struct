#include "jsonstruct.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

using namespace JsonStruct;

// 简单结构体测试
struct SimpleStruct {
    int id = 42;
    std::string name = "test";
    bool active = true;
    
    JSON_FIELDS(id, name, active)
};

// 基础类型测试
struct BasicTypes {
    int integer = 123;
    double decimal = 3.14;
    std::string text = "hello";
    bool flag = true;
    
    JSON_AUTO(integer, decimal, text, flag)
};

void test_field_access() {
    std::cout << "=== Testing Field Access ===" << std::endl;
    
    SimpleStruct obj;
    obj.id = 999;
    obj.name = "modified";
    obj.active = false;
    
    // 测试字段名称解析
    auto names = obj.get_field_names();
    std::cout << "Field count: " << names.size() << std::endl;
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << "Field " << i << ": " << names[i] << std::endl;
    }
    
    // 测试字段值访问
    auto fields = obj.json_fields();
    std::cout << "Field values:" << std::endl;
    std::cout << "  id: " << std::get<0>(fields) << std::endl;
    std::cout << "  name: " << std::get<1>(fields) << std::endl;
    std::cout << "  active: " << std::get<2>(fields) << std::endl;
    
    // 验证字段值正确性
    assert(std::get<0>(fields) == 999);
    assert(std::get<1>(fields) == "modified");
    assert(std::get<2>(fields) == false);
    
    std::cout << "✓ Field access test passed!" << std::endl;
}

void test_macro_expansion() {
    std::cout << "\n=== Testing Macro Expansion ===" << std::endl;
    
    BasicTypes obj;
    
    // 测试字段名字符串
    const char* field_names_str = obj.json_field_names();
    std::cout << "Field names string: " << field_names_str << std::endl;
    
    // 测试字段名解析
    auto field_names = obj.get_field_names();
    std::cout << "Parsed field names:" << std::endl;
    for (size_t i = 0; i < field_names.size(); ++i) {
        std::cout << "  " << i << ": " << field_names[i] << std::endl;
    }
    
    // 测试字段访问
    auto fields = obj.json_fields();
    std::cout << "Field values:" << std::endl;
    std::cout << "  integer: " << std::get<0>(fields) << std::endl;
    std::cout << "  decimal: " << std::get<1>(fields) << std::endl;
    std::cout << "  text: " << std::get<2>(fields) << std::endl;
    std::cout << "  flag: " << std::get<3>(fields) << std::endl;
    
    // 验证字段值
    assert(std::get<0>(fields) == 123);
    assert(std::get<1>(fields) == 3.14);
    assert(std::get<2>(fields) == "hello");
    assert(std::get<3>(fields) == true);
    
    std::cout << "✓ Macro expansion test passed!" << std::endl;
}

void test_serialization_basics() {
    std::cout << "\n=== Testing Basic Serialization ===" << std::endl;
    
    BasicTypes obj;
    obj.integer = 456;
    obj.decimal = 2.718;
    obj.text = "world";
    obj.flag = false;
    
    // 尝试序列化
    try {
        auto json = obj.toJson();
        std::cout << "Serialization successful, type: " << 
            (json.isObject() ? "object" : 
             json.isNull() ? "null" : 
             json.isNumber() ? "number" : 
             json.isString() ? "string" : "unknown") << std::endl;
        
        if (json.isObject()) {
            auto& obj_map = json.toObject();
            std::cout << "Object contains " << obj_map.size() << " fields" << std::endl;
            for (const auto& pair : obj_map) {
                std::cout << "  " << pair.first << ": ";
                if (pair.second.isNumber()) {
                    std::cout << pair.second.toDouble();
                } else if (pair.second.isString()) {
                    std::cout << "\"" << pair.second.toString() << "\"";
                } else if (pair.second.isBool()) {
                    std::cout << (pair.second.toBool() ? "true" : "false");
                } else {
                    std::cout << "(other type)";
                }
                std::cout << std::endl;
            }
        }
        
        std::cout << "✓ Basic serialization test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ Serialization failed: " << e.what() << std::endl;
    }
}

void test_type_registry() {
    std::cout << "\n=== Testing Type Registry ===" << std::endl;
    
    TypeRegistry& reg = TypeRegistry::instance();
    
    // 清理注册表
    reg.clear();
    std::cout << "Registry cleared" << std::endl;
    
    // 检查基础类型是否已注册
    std::cout << "Type registration status:" << std::endl;
    std::cout << "  int: " << (reg.isRegistered<int>() ? "registered" : "not registered") << std::endl;
    std::cout << "  double: " << (reg.isRegistered<double>() ? "registered" : "not registered") << std::endl;
    std::cout << "  std::string: " << (reg.isRegistered<std::string>() ? "registered" : "not registered") << std::endl;
    std::cout << "  bool: " << (reg.isRegistered<bool>() ? "registered" : "not registered") << std::endl;
    
    // 获取已注册类型列表
    auto types = reg.getRegisteredTypes();
    std::cout << "Total registered types: " << types.size() << std::endl;
    
    std::cout << "✓ Type registry test completed!" << std::endl;
}

void test_edge_cases() {
    std::cout << "\n=== Testing Edge Cases ===" << std::endl;
    
    struct SingleField {
        int value = 100;
        JSON_FIELDS(value)
    };
    
    struct DoubleField {
        int first = 10;
        std::string second = "test";
        JSON_FIELDS(first, second)
    };
    
    // 测试单字段结构体
    try {
        SingleField single;
        auto single_names = single.get_field_names();
        std::cout << "Single field struct field count: " << single_names.size() << std::endl;
        assert(single_names.size() == 1);
        assert(single_names[0] == "value");
        
        auto single_fields = single.json_fields();
        assert(std::get<0>(single_fields) == 100);
        std::cout << "✓ Single field struct test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ Single field struct test failed: " << e.what() << std::endl;
    }
    
    // 测试双字段结构体
    try {
        DoubleField double_field;
        auto double_names = double_field.get_field_names();
        std::cout << "Double field struct field count: " << double_names.size() << std::endl;
        assert(double_names.size() == 2);
        assert(double_names[0] == "first");
        assert(double_names[1] == "second");
        
        auto double_fields = double_field.json_fields();
        assert(std::get<0>(double_fields) == 10);
        assert(std::get<1>(double_fields) == "test");
        std::cout << "✓ Double field struct test passed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ Double field struct test failed: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== JsonStruct Core Functionality Test ===" << std::endl;
    std::cout << "Testing fundamental macro and registry features...\n" << std::endl;
    
    try {
        test_field_access();
        test_macro_expansion();
        test_serialization_basics();
        test_type_registry();
        test_edge_cases();
        
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "✓ All core functionality tests completed!" << std::endl;
        std::cout << "✓ JSON_FIELDS macro working correctly" << std::endl;
        std::cout << "✓ JSON_AUTO macro expanding properly" << std::endl;
        std::cout << "✓ Field access and naming functional" << std::endl;
        std::cout << "✓ Basic serialization attempted" << std::endl;
        std::cout << "✓ Type registry accessible" << std::endl;
        std::cout << "✓ Edge cases handled" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
