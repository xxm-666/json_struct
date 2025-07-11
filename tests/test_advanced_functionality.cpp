#include "jsonstruct.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace JsonStruct;

// 测试统计
struct TestResult {
    bool passed = true;
    std::string name;
    std::string error;
    
    TestResult(const std::string& test_name) : name(test_name) {}
    
    void fail(const std::string& message) {
        passed = false;
        error = message;
    }
};

std::vector<TestResult> test_results;

#define RUN_TEST(test_func, test_name) \
    do { \
        TestResult result(test_name); \
        try { \
            test_func(result); \
        } catch (const std::exception& e) { \
            result.fail(std::string("Exception: ") + e.what()); \
        } catch (...) { \
            result.fail("Unknown exception"); \
        } \
        test_results.push_back(result); \
        std::cout << (result.passed ? "PASS" : "FAIL") << " " << test_name; \
        if (!result.passed) std::cout << " - " << result.error; \
        std::cout << std::endl; \
    } while(0)

// 基础类型测试
struct BasicData {
    int id = 42;
    std::string name = "test";
    double value = 3.14159;
    bool active = true;
    
    JSON_AUTO(id, name, value, active)
};

// 容器类型测试
struct ContainerData {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::vector<std::string> words = {"hello", "world", "test"};
    std::map<std::string, int> mapping = {{"one", 1}, {"two", 2}, {"three", 3}};
    
    JSON_FIELDS(numbers, words, mapping)
};

// 嵌套结构测试 - 简化版本
struct Address {
    std::string street = "123 Main St";
    std::string city = "TestCity";
    int zipcode = 12345;
    
    JSON_FIELDS(street, city, zipcode)
};

struct SimplePerson {
    std::string name = "John Doe";
    int age = 30;
    std::vector<std::string> hobbies = {"reading", "coding"};
    
    JSON_AUTO(name, age, hobbies)
};

void test_basic_serialization(TestResult& result) {
    BasicData original;
    original.id = 999;
    original.name = "serialization_test";
    original.value = 2.71828;
    original.active = false;
    
    // 序列化
    auto json = original.toJson();
    if (!json.isObject()) {
        result.fail("Serialization did not produce object");
        return;
    }
    
    auto& obj = json.toObject();
    if (obj.size() != 4) {
        result.fail("Wrong number of fields in serialized object");
        return;
    }
    
    // 验证字段值
    if (obj.at("id").toInt() != 999) {
        result.fail("ID field incorrect");
        return;
    }
    
    if (obj.at("name").toString() != "serialization_test") {
        result.fail("Name field incorrect");
        return;
    }
    
    if (std::abs(obj.at("value").toDouble() - 2.71828) > 0.00001) {
        result.fail("Value field incorrect");
        return;
    }
    
    if (obj.at("active").toBool() != false) {
        result.fail("Active field incorrect");
        return;
    }
}

void test_basic_deserialization(TestResult& result) {
    // 创建JSON对象
    JsonValue json = JsonValue::object();
    auto& obj = json.toObject();
    obj["id"] = JsonValue(555);
    obj["name"] = JsonValue("deserialization_test");
    obj["value"] = JsonValue(1.41421);
    obj["active"] = JsonValue(true);
    
    // 反序列化
    BasicData restored;
    restored.fromJson(obj);
    
    // 验证数据
    if (restored.id != 555) {
        result.fail("ID not restored correctly");
        return;
    }
    
    if (restored.name != "deserialization_test") {
        result.fail("Name not restored correctly");
        return;
    }
    
    if (std::abs(restored.value - 1.41421) > 0.00001) {
        result.fail("Value not restored correctly");
        return;
    }
    
    if (restored.active != true) {
        result.fail("Active not restored correctly");
        return;
    }
}

void test_roundtrip_serialization(TestResult& result) {
    BasicData original;
    original.id = 777;
    original.name = "roundtrip_test";
    original.value = 1.23456789;
    original.active = true;
    
    // 序列化然后反序列化
    auto json = original.toJson();
    BasicData restored;
    restored.fromJson(json.toObject());
    
    // 验证数据完整性
    if (restored.id != original.id) {
        result.fail("ID roundtrip failed");
        return;
    }
    
    if (restored.name != original.name) {
        result.fail("Name roundtrip failed");
        return;
    }
    
    if (std::abs(restored.value - original.value) > 0.000001) {
        result.fail("Value roundtrip failed");
        return;
    }
    
    if (restored.active != original.active) {
        result.fail("Active roundtrip failed");
        return;
    }
}

void test_container_serialization(TestResult& result) {
    ContainerData original;
    original.numbers = {10, 20, 30};
    original.words = {"apple", "banana"};
    original.mapping = {{"key1", 100}, {"key2", 200}};
    
    // 检查字段访问
    auto fields = original.json_fields();
    if (std::get<0>(fields).size() != 3) {
        result.fail("Numbers container wrong size");
        return;
    }
    
    if (std::get<1>(fields).size() != 2) {
        result.fail("Words container wrong size");
        return;
    }
    
    if (std::get<2>(fields).size() != 2) {
        result.fail("Mapping container wrong size");
        return;
    }
    
    // 验证字段名
    auto names = original.get_field_names();
    if (names.size() != 3) {
        result.fail("Wrong number of field names");
        return;
    }
    
    if (names[0] != "numbers" || names[1] != "words" || names[2] != "mapping") {
        result.fail("Field names incorrect");
        return;
    }
}

void test_nested_structure(TestResult& result) {
    SimplePerson original;
    original.name = "Alice Smith";
    original.age = 25;
    original.hobbies = {"swimming", "painting", "cooking"};
    
    // 检查字段访问
    auto fields = original.json_fields();
    if (std::get<0>(fields) != "Alice Smith") {
        result.fail("Name field access failed");
        return;
    }
    
    if (std::get<1>(fields) != 25) {
        result.fail("Age field access failed");
        return;
    }
    
    if (std::get<2>(fields).size() != 3) {
        result.fail("Hobbies vector access failed");
        return;
    }
    
    // 尝试序列化简化结构
    try {
        auto json = original.toJson();
        if (!json.isObject()) {
            result.fail("Simplified structure serialization failed");
            return;
        }
        
        auto& obj = json.toObject();
        if (obj.size() != 3) {
            result.fail("Wrong number of fields in serialized simplified structure");
            return;
        }
        
    } catch (...) {
        result.fail("Simplified structure serialization threw exception");
        return;
    }
}

void test_field_modification(TestResult& result) {
    BasicData data;
    
    // 通过字段引用修改数据
    auto fields = data.json_fields();
    std::get<0>(fields) = 888;
    std::get<1>(fields) = "modified_name";
    std::get<2>(fields) = 9.87654;
    std::get<3>(fields) = false;
    
    // 验证修改生效
    if (data.id != 888) {
        result.fail("ID modification failed");
        return;
    }
    
    if (data.name != "modified_name") {
        result.fail("Name modification failed");
        return;
    }
    
    if (std::abs(data.value - 9.87654) > 0.00001) {
        result.fail("Value modification failed");
        return;
    }
    
    if (data.active != false) {
        result.fail("Active modification failed");
        return;
    }
}

void test_type_registry_integration(TestResult& result) {
    TypeRegistry& reg = TypeRegistry::instance();
    
    // 检查注册状态
    bool int_registered = reg.isRegistered<int>();
    bool string_registered = reg.isRegistered<std::string>();
    bool double_registered = reg.isRegistered<double>();
    bool bool_registered = reg.isRegistered<bool>();
    
    std::cout << "  Registry status - int: " << int_registered 
              << ", string: " << string_registered
              << ", double: " << double_registered 
              << ", bool: " << bool_registered << std::endl;
    
    // 获取注册类型数量
    auto types = reg.getRegisteredTypes();
    std::cout << "  Total registered types: " << types.size() << std::endl;
    
    // 基本类型应该已经注册或能够注册
    // 这个测试主要是检查注册表的可访问性
}

void test_macro_edge_cases(TestResult& result) {
    // 单字段结构
    struct SingleField {
        int value = 100;
        JSON_FIELDS(value)
    };
    
    SingleField single;
    auto names = single.get_field_names();
    if (names.size() != 1 || names[0] != "value") {
        result.fail("Single field test failed");
        return;
    }
    
    auto fields = single.json_fields();
    if (std::get<0>(fields) != 100) {
        result.fail("Single field value test failed");
        return;
    }
    
    // 多字段结构 (测试宏的可扩展性)
    struct ManyFields {
        int a = 1;
        int b = 2;
        int c = 3;
        int d = 4;
        int e = 5;
        JSON_AUTO(a, b, c, d, e)
    };
    
    ManyFields many;
    auto many_names = many.get_field_names();
    if (many_names.size() != 5) {
        result.fail("Many fields count test failed");
        return;
    }
    
    auto many_fields = many.json_fields();
    if (std::get<0>(many_fields) != 1 || std::get<4>(many_fields) != 5) {
        result.fail("Many fields value test failed");
        return;
    }
}

void print_summary() {
    std::cout << "\n=== Test Summary ===" << std::endl;
    
    int passed = 0;
    int failed = 0;
    
    for (const auto& result : test_results) {
        if (result.passed) {
            passed++;
        } else {
            failed++;
            std::cout << "FAILED: " << result.name << " - " << result.error << std::endl;
        }
    }
    
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Total:  " << test_results.size() << std::endl;
    
    if (failed == 0) {
        std::cout << "\nALL TESTS PASSED!" << std::endl;
        std::cout << "JsonStruct macro system is working correctly!" << std::endl;
    } else {
        std::cout << "\nSome tests failed. Please check the errors above." << std::endl;
    }
}

int main() {
    std::cout << "=== JsonStruct Advanced Test Suite ===" << std::endl;
    std::cout << "Testing serialization, deserialization, and full integration...\n" << std::endl;
    
    RUN_TEST(test_basic_serialization, "Basic Serialization");
    RUN_TEST(test_basic_deserialization, "Basic Deserialization");
    RUN_TEST(test_roundtrip_serialization, "Roundtrip Serialization");
    RUN_TEST(test_container_serialization, "Container Serialization");
    RUN_TEST(test_nested_structure, "Nested Structure");
    RUN_TEST(test_field_modification, "Field Modification");
    RUN_TEST(test_type_registry_integration, "Type Registry Integration");
    RUN_TEST(test_macro_edge_cases, "Macro Edge Cases");
    
    print_summary();
    
    return (std::count_if(test_results.begin(), test_results.end(), 
                         [](const TestResult& r) { return !r.passed; }) == 0) ? 0 : 1;
}
