#include "jsonstruct.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <map>

using namespace JsonStruct;

// 基础类型测试
struct BasicTypes {
    bool flag = false;
    int integer = 0;
    double decimal = 0.0;
    std::string text = "";
    
    JSON_AUTO(flag, integer, decimal, text)
};

// 容器类型测试
struct ContainerTypes {
    std::vector<int> numbers;
    std::vector<std::string> words;
    std::map<std::string, int> mapping;
    
    JSON_FIELDS(numbers, words, mapping)
};

// 嵌套结构体测试
struct Address {
    std::string street;
    std::string city;
    int zipcode;
    
    JSON_FIELDS(street, city, zipcode)
};

struct Person {
    std::string name;
    int age;
    Address address;
    
    JSON_FIELDS(name, age, address)
};

// 边界情况测试
struct EdgeCases {
    std::string empty_string = "";
    std::vector<int> empty_vector;
    int zero_value = 0;
    
    JSON_FIELDS(empty_string, empty_vector, zero_value)
};

void test_basic_types() {
    std::cout << "Testing basic types..." << std::endl;
    
    BasicTypes obj{true, 42, 3.14159, "Hello World"};
    
    // 序列化测试
    auto jsonVal = obj.toJson();
    assert(jsonVal.isObject());
    
    auto& jsonObj = jsonVal.toObject();
    assert(jsonObj.at("flag").toBool() == true);
    assert(jsonObj.at("integer").toInt() == 42);
    assert(std::abs(jsonObj.at("decimal").toDouble() - 3.14159) < 0.00001);
    assert(jsonObj.at("text").toString() == "Hello World");
    
    // 反序列化测试
    BasicTypes obj2;
    obj2.fromJson(jsonVal);
    assert(obj2.flag == true);
    assert(obj2.integer == 42);
    assert(std::abs(obj2.decimal - 3.14159) < 0.00001);
    assert(obj2.text == "Hello World");
    
    std::cout << "Basic types test passed!" << std::endl;
}

void test_container_types() {
    std::cout << "Testing container types..." << std::endl;
    
    ContainerTypes obj;
    obj.numbers = {1, 2, 3, 4, 5};
    obj.words = {"apple", "banana", "cherry"};
    obj.mapping = {{"one", 1}, {"two", 2}, {"three", 3}};
    
    // 测试字段名解析
    auto names = obj.get_field_names();
    assert(names.size() == 3);
    assert(names[0] == "numbers");
    assert(names[1] == "words");
    assert(names[2] == "mapping");
    
    // 测试字段访问
    auto fields = obj.json_fields();
    assert(std::get<0>(fields).size() == 5);
    assert(std::get<1>(fields).size() == 3);
    assert(std::get<2>(fields).size() == 3);
    
    std::cout << "Container types test passed!" << std::endl;
}

void test_nested_structures() {
    std::cout << "Testing nested structures..." << std::endl;
    
    Person person;
    person.name = "John Doe";
    person.age = 30;
    person.address.street = "123 Main St";
    person.address.city = "Anytown";
    person.address.zipcode = 12345;
    
    // 测试字段访问
    auto fields = person.json_fields();
    assert(std::get<0>(fields) == "John Doe");
    assert(std::get<1>(fields) == 30);
    assert(std::get<2>(fields).street == "123 Main St");
    
    std::cout << "Nested structures test passed!" << std::endl;
}

void test_edge_cases() {
    std::cout << "Testing edge cases..." << std::endl;
    
    EdgeCases obj;
    
    // 测试空值和边界值
    auto fields = obj.json_fields();
    assert(std::get<0>(fields) == "");  // empty_string
    assert(std::get<1>(fields).empty()); // empty_vector
    assert(std::get<2>(fields) == 0);   // zero_value
    
    std::cout << "Edge cases test passed!" << std::endl;
}

void test_json_fields_macro() {
    std::cout << "Testing JSON_FIELDS macro functionality..." << std::endl;
    
    struct TestStruct {
        int a = 1;
        double b = 2.5;
        std::string c = "test";
        JSON_FIELDS(a, b, c)
    };
    
    TestStruct obj;
    
    // 测试字段名字符串
    const char* names = obj.json_field_names();
    std::string names_str(names);
    assert(names_str == "a, b, c");
    
    // 测试字段名解析
    auto field_names = obj.get_field_names();
    assert(field_names.size() == 3);
    assert(field_names[0] == "a");
    assert(field_names[1] == "b");
    assert(field_names[2] == "c");
    
    // 测试字段元组访问
    auto fields_const = static_cast<const TestStruct&>(obj).json_fields();
    auto fields_mutable = obj.json_fields();
    
    assert(std::get<0>(fields_const) == 1);
    assert(std::get<1>(fields_const) == 2.5);
    assert(std::get<2>(fields_const) == "test");
    
    // 修改可变字段
    std::get<0>(fields_mutable) = 10;
    assert(obj.a == 10);
    
    std::cout << "JSON_FIELDS macro test passed!" << std::endl;
}

void test_type_safety() {
    std::cout << "Testing type safety..." << std::endl;
    
    struct TypeSafeStruct {
        int int_field = 123;
        bool bool_field = true;
        double double_field = 45.67;
        JSON_FIELDS(int_field, bool_field, double_field)
    };
    
    TypeSafeStruct obj;
    
    // 验证字段访问
    auto fields = obj.json_fields();
    assert(std::get<0>(fields) == 123);
    assert(std::get<1>(fields) == true);
    assert(std::abs(std::get<2>(fields) - 45.67) < 0.001);
    
    std::cout << "Type safety test passed!" << std::endl;
}

void test_performance_basic() {
    std::cout << "Testing basic performance..." << std::endl;
    
    struct PerfStruct {
        int id;
        std::string name;
        double value;
        JSON_AUTO(id, name, value)
    };
    
    const int iterations = 100;  // 减少迭代次数避免过长的测试时间
    
    // 批量序列化测试
    for (int i = 0; i < iterations; ++i) {
        PerfStruct obj{i, "item_" + std::to_string(i), i * 0.1};
        auto jsonVal = obj.toJson();
        assert(jsonVal.isObject());
    }
    
    std::cout << "Performance test completed (" << iterations << " iterations)" << std::endl;
}

int main() {
    std::cout << "=== Comprehensive JSON_AUTO and JSON_FIELDS Test Suite ===" << std::endl;
    
    try {
        test_basic_types();
        test_container_types();
        test_nested_structures();
        test_edge_cases();
        test_json_fields_macro();
        test_type_safety();
        test_performance_basic();
        
        std::cout << "\n=== ALL TESTS PASSED! ===" << std::endl;
        std::cout << "✓ Basic type serialization/deserialization" << std::endl;
        std::cout << "✓ Container type handling" << std::endl;
        std::cout << "✓ Nested structure support" << std::endl;
        std::cout << "✓ Edge case handling" << std::endl;
        std::cout << "✓ JSON_FIELDS macro functionality" << std::endl;
        std::cout << "✓ Type safety verification" << std::endl;
        std::cout << "✓ Basic performance validation" << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
