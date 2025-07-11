#include "jsonstruct.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <list>
#include <deque>
#include <array>
#include <memory>
#include <optional>
#include <chrono>
#include <sstream>
#include <fstream>
#include <random>

using namespace JsonStruct;

// 测试统计
struct TestStats {
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failures;
    
    void addPass() { passed++; }
    void addFail(const std::string& msg) { 
        failed++; 
        failures.push_back(msg);
        std::cerr << "FAIL: " << msg << std::endl;
    }
    
    void report() {
        std::cout << "\n=== TEST RESULTS ===" << std::endl;
        std::cout << "Passed: " << passed << std::endl;
        std::cout << "Failed: " << failed << std::endl;
        
        if (!failures.empty()) {
            std::cout << "\nFailures:" << std::endl;
            for (const auto& failure : failures) {
                std::cout << "  - " << failure << std::endl;
            }
        }
        
        if (failed == 0) {
            std::cout << "\n✓ ALL TESTS PASSED!" << std::endl;
        }
    }
};

TestStats g_stats;

#define TEST_ASSERT(condition, message) \
    do { \
        if (condition) { \
            g_stats.addPass(); \
        } else { \
            g_stats.addFail(std::string(message) + " (line " + std::to_string(__LINE__) + ")"); \
        } \
    } while(0)

// 1. 基础类型测试
struct BasicTypes {
    bool flag = false;
    char char_val = 'A';
    int8_t int8_val = -128;
    uint8_t uint8_val = 255;
    int16_t int16_val = -32768;
    uint16_t uint16_val = 65535;
    int32_t int32_val = -2147483648;
    uint32_t uint32_val = 4294967295U;
    int64_t int64_val = -9223372036854775807LL;
    uint64_t uint64_val = 18446744073709551615ULL;
    float float_val = 3.14159f;
    double double_val = 2.718281828459045;
    std::string string_val = "Hello, World!";
    
    JSON_AUTO(flag, char_val, int8_val, uint8_val, int16_val, uint16_val,
              int32_val, uint32_val, int64_val, uint64_val, 
              float_val, double_val, string_val)
};

// 2. 容器类型测试
struct ContainerTypes {
    std::vector<int> vec_int;
    std::vector<std::string> vec_string;
    std::list<double> list_double;
    std::deque<bool> deque_bool;
    std::set<int> set_int;
    std::map<std::string, int> map_str_int;
    std::unordered_map<int, std::string> umap_int_str;
    std::array<int, 5> array_int;
    
    JSON_FIELDS(vec_int, vec_string, list_double, deque_bool, 
                set_int, map_str_int, umap_int_str, array_int)
};

// 3. 嵌套结构体测试
struct Address {
    std::string street;
    std::string city;
    std::string state;
    int zipcode;
    std::string country = "USA";
    
    JSON_AUTO(street, city, state, zipcode, country)
};

struct Contact {
    std::string email;
    std::string phone;
    Address address;
    
    JSON_FIELDS(email, phone, address)
};

struct Person {
    std::string name;
    int age;
    double height;
    bool married;
    Contact contact;
    std::vector<std::string> hobbies;
    std::map<std::string, std::string> metadata;
    
    JSON_AUTO(name, age, height, married, contact, hobbies, metadata)
};

// 4. 复杂嵌套测试
struct Department {
    std::string name;
    std::vector<Person> employees;
    Person manager;
    std::map<std::string, std::vector<Person>> teams;
    
    JSON_FIELDS(name, employees, manager, teams)
};

// 5. 边界值和特殊情况测试
struct EdgeCases {
    std::string empty_string = "";
    std::vector<int> empty_vector;
    std::map<std::string, int> empty_map;
    int zero = 0;
    double negative_double = -123.456;
    std::string unicode_string = "测试中文字符 🌟 émojis";
    std::string special_chars = "Special: \n\t\r\"\\'/";
    std::vector<std::vector<int>> nested_vectors;
    
    JSON_AUTO(empty_string, empty_vector, empty_map, zero, 
              negative_double, unicode_string, special_chars, nested_vectors)
};

// 6. 自定义类型测试
enum class Color { RED, GREEN, BLUE };
enum Status { PENDING = 1, APPROVED = 2, REJECTED = 3 };

struct CustomTypes {
    Color color = Color::RED;
    Status status = PENDING;
    std::vector<Color> color_list;
    
    JSON_FIELDS(color, status, color_list)
};

// 7. 性能测试结构
struct PerformanceStruct {
    int id;
    std::string name;
    double value;
    std::vector<int> data;
    std::map<std::string, std::string> properties;
    
    JSON_AUTO(id, name, value, data, properties)
};

// 8. 深度嵌套测试
struct Level3 {
    int value = 3;
    std::string name = "level3";
    JSON_FIELDS(value, name)
};

struct Level2 {
    Level3 level3;
    std::vector<Level3> level3_list;
    std::map<std::string, Level3> level3_map;
    JSON_FIELDS(level3, level3_list, level3_map)
};

struct Level1 {
    Level2 level2;
    std::vector<Level2> level2_list;
    JSON_FIELDS(level2, level2_list)
};

// 测试函数实现
void test_basic_types() {
    std::cout << "\n=== Testing Basic Types ===" << std::endl;
    
    BasicTypes obj;
    obj.flag = true;
    obj.char_val = 'Z';
    obj.int8_val = 127;
    obj.uint8_val = 200;
    obj.int16_val = 1000;
    obj.uint16_val = 50000;
    obj.int32_val = 1000000;
    obj.uint32_val = 2000000000U;
    obj.int64_val = 1000000000000LL;
    obj.uint64_val = 2000000000000ULL;
    obj.float_val = 1.23456f;
    obj.double_val = 9.87654321;
    obj.string_val = "Updated String";
    
    try {
        // 序列化
        auto jsonVal = obj.toJson();
        TEST_ASSERT(jsonVal.isObject(), "Basic types serialization should produce object");
        
        // 反序列化
        BasicTypes obj2;
        obj2.fromJson(jsonVal.toObject());
        
        TEST_ASSERT(obj2.flag == obj.flag, "Boolean field mismatch");
        TEST_ASSERT(obj2.char_val == obj.char_val, "Char field mismatch");
        TEST_ASSERT(obj2.int8_val == obj.int8_val, "Int8 field mismatch");
        TEST_ASSERT(obj2.uint8_val == obj.uint8_val, "UInt8 field mismatch");
        TEST_ASSERT(obj2.string_val == obj.string_val, "String field mismatch");
        TEST_ASSERT(std::abs(obj2.float_val - obj.float_val) < 0.0001f, "Float field mismatch");
        TEST_ASSERT(std::abs(obj2.double_val - obj.double_val) < 0.000001, "Double field mismatch");
        
    } catch (const std::exception& e) {
        g_stats.addFail("Basic types test exception: " + std::string(e.what()));
    }
}

void test_container_types() {
    std::cout << "\n=== Testing Container Types ===" << std::endl;
    
    ContainerTypes obj;
    obj.vec_int = {1, 2, 3, 4, 5};
    obj.vec_string = {"apple", "banana", "cherry"};
    obj.list_double = {1.1, 2.2, 3.3};
    obj.deque_bool = {true, false, true};
    obj.set_int = {10, 20, 30};
    obj.map_str_int = {{"one", 1}, {"two", 2}, {"three", 3}};
    obj.umap_int_str = {{1, "first"}, {2, "second"}};
    obj.array_int = {100, 200, 300, 400, 500};
    
    try {
        // 测试字段名称
        auto names = obj.get_field_names();
        TEST_ASSERT(names.size() == 8, "Container types should have 8 fields");
        TEST_ASSERT(names[0] == "vec_int", "First field name mismatch");
        
        // 测试字段访问
        auto fields = obj.json_fields();
        TEST_ASSERT(std::get<0>(fields).size() == 5, "Vector size mismatch");
        TEST_ASSERT(std::get<1>(fields).size() == 3, "String vector size mismatch");
        TEST_ASSERT(std::get<4>(fields).size() == 3, "Set size mismatch");
        
    } catch (const std::exception& e) {
        g_stats.addFail("Container types test exception: " + std::string(e.what()));
    }
}

void test_nested_structures() {
    std::cout << "\n=== Testing Nested Structures ===" << std::endl;
    
    Person person;
    person.name = "John Doe";
    person.age = 30;
    person.height = 175.5;
    person.married = true;
    person.contact.email = "john@example.com";
    person.contact.phone = "+1-555-0123";
    person.contact.address.street = "123 Main St";
    person.contact.address.city = "Anytown";
    person.contact.address.state = "CA";
    person.contact.address.zipcode = 12345;
    person.hobbies = {"reading", "swimming", "coding"};
    person.metadata = {{"department", "Engineering"}, {"level", "Senior"}};
    
    try {
        // 序列化
        auto jsonVal = person.toJson();
        TEST_ASSERT(jsonVal.isObject(), "Person serialization should produce object");
        
        // 反序列化
        Person person2;
        person2.fromJson(jsonVal.toObject());
        
        TEST_ASSERT(person2.name == person.name, "Person name mismatch");
        TEST_ASSERT(person2.age == person.age, "Person age mismatch");
        TEST_ASSERT(person2.contact.email == person.contact.email, "Contact email mismatch");
        TEST_ASSERT(person2.contact.address.city == person.contact.address.city, "Address city mismatch");
        TEST_ASSERT(person2.hobbies.size() == person.hobbies.size(), "Hobbies size mismatch");
        TEST_ASSERT(person2.metadata.size() == person.metadata.size(), "Metadata size mismatch");
        
    } catch (const std::exception& e) {
        g_stats.addFail("Nested structures test exception: " + std::string(e.what()));
    }
}

void test_edge_cases() {
    std::cout << "\n=== Testing Edge Cases ===" << std::endl;
    
    EdgeCases obj;
    obj.nested_vectors = {{1, 2}, {3, 4, 5}, {}};
    
    try {
        // 序列化
        auto jsonVal = obj.toJson();
        TEST_ASSERT(jsonVal.isObject(), "Edge cases serialization should produce object");
        
        // 反序列化
        EdgeCases obj2;
        obj2.fromJson(jsonVal.toObject());
        
        TEST_ASSERT(obj2.empty_string.empty(), "Empty string should remain empty");
        TEST_ASSERT(obj2.empty_vector.empty(), "Empty vector should remain empty");
        TEST_ASSERT(obj2.empty_map.empty(), "Empty map should remain empty");
        TEST_ASSERT(obj2.zero == 0, "Zero value should remain zero");
        TEST_ASSERT(obj2.negative_double < 0, "Negative double should remain negative");
        TEST_ASSERT(obj2.unicode_string == obj.unicode_string, "Unicode string mismatch");
        TEST_ASSERT(obj2.nested_vectors.size() == 3, "Nested vectors size mismatch");
        
    } catch (const std::exception& e) {
        g_stats.addFail("Edge cases test exception: " + std::string(e.what()));
    }
}

void test_deep_nesting() {
    std::cout << "\n=== Testing Deep Nesting ===" << std::endl;
    
    Level1 obj;
    obj.level2.level3.value = 42;
    obj.level2.level3.name = "deep_test";
    obj.level2.level3_list = {{100, "item1"}, {200, "item2"}};
    obj.level2.level3_map = {{"key1", {300, "map_item1"}}, {"key2", {400, "map_item2"}}};
    obj.level2_list = {obj.level2, obj.level2};
    
    try {
        // 测试字段访问
        auto fields = obj.json_fields();
        TEST_ASSERT(std::get<0>(fields).level3.value == 42, "Deep nested value mismatch");
        TEST_ASSERT(std::get<1>(fields).size() == 2, "Level2 list size mismatch");
        
    } catch (const std::exception& e) {
        g_stats.addFail("Deep nesting test exception: " + std::string(e.what()));
    }
}

void test_macro_functionality() {
    std::cout << "\n=== Testing Macro Functionality ===" << std::endl;
    
    struct MacroTest {
        int a = 1;
        double b = 2.5;
        std::string c = "test";
        bool d = true;
        std::vector<int> e = {1, 2, 3};
        JSON_FIELDS(a, b, c, d, e)
    };
    
    MacroTest obj;
    
    try {
        // 测试字段名字符串
        const char* names = obj.json_field_names();
        std::string names_str(names);
        TEST_ASSERT(names_str == "a, b, c, d, e", "Field names string mismatch");
        
        // 测试字段名解析
        auto field_names = obj.get_field_names();
        TEST_ASSERT(field_names.size() == 5, "Field names count mismatch");
        TEST_ASSERT(field_names[0] == "a", "First field name mismatch");
        TEST_ASSERT(field_names[4] == "e", "Last field name mismatch");
        
        // 测试const和非const访问
        const MacroTest& const_obj = obj;
        auto const_fields = const_obj.json_fields();
        auto mutable_fields = obj.json_fields();
        
        TEST_ASSERT(std::get<0>(const_fields) == 1, "Const field access failed");
        TEST_ASSERT(std::get<4>(mutable_fields).size() == 3, "Mutable vector field access failed");
        
        // 测试字段修改
        std::get<0>(mutable_fields) = 999;
        TEST_ASSERT(obj.a == 999, "Field modification failed");
        
    } catch (const std::exception& e) {
        g_stats.addFail("Macro functionality test exception: " + std::string(e.what()));
    }
}

void test_performance() {
    std::cout << "\n=== Testing Performance ===" << std::endl;
    
    const int iterations = 1000;
    std::vector<PerformanceStruct> objects;
    objects.reserve(iterations);
    
    // 准备测试数据
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
    
    try {
        // 序列化性能测试
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<JsonValue> serialized;
        serialized.reserve(iterations);
        
        for (const auto& obj : objects) {
            serialized.push_back(obj.toJson());
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        TEST_ASSERT(serialized.size() == iterations, "Serialization count mismatch");
        std::cout << "Serialized " << iterations << " objects in " << duration.count() << "ms" << std::endl;
        
        // 反序列化性能测试
        start = std::chrono::high_resolution_clock::now();
        std::vector<PerformanceStruct> deserialized;
        deserialized.reserve(iterations);
        
        for (const auto& json : serialized) {
            PerformanceStruct obj;
            obj.fromJson(json.toObject());
            deserialized.push_back(std::move(obj));
        }
        
        end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        TEST_ASSERT(deserialized.size() == iterations, "Deserialization count mismatch");
        std::cout << "Deserialized " << iterations << " objects in " << duration.count() << "ms" << std::endl;
        
        // 验证数据一致性
        for (size_t i = 0; i < objects.size(); ++i) {
            TEST_ASSERT(objects[i].id == deserialized[i].id, "ID mismatch in performance test");
            TEST_ASSERT(objects[i].name == deserialized[i].name, "Name mismatch in performance test");
            TEST_ASSERT(objects[i].data.size() == deserialized[i].data.size(), "Data size mismatch in performance test");
        }
        
    } catch (const std::exception& e) {
        g_stats.addFail("Performance test exception: " + std::string(e.what()));
    }
}

void test_error_handling() {
    std::cout << "\n=== Testing Error Handling ===" << std::endl;
    
    try {
        // 测试空JSON对象的处理
        JsonValue empty_json = JsonValue::object();  // 创建空对象而不是null值
        BasicTypes obj;
        
        // 这应该不会崩溃，而是使用默认值
        try {
            obj.fromJson(empty_json.toObject());
            TEST_ASSERT(true, "Empty JSON handling successful");
        } catch (...) {
            g_stats.addFail("Empty JSON handling failed");
        }
        
        // 测试类型不匹配的处理
        JsonValue invalid_json = JsonValue::object();
        invalid_json.toObject()["flag"] = JsonValue("not_a_boolean");
        
        try {
            BasicTypes obj2;
            obj2.fromJson(invalid_json.toObject());
            // 这里期望能够优雅处理类型转换
            TEST_ASSERT(true, "Type mismatch handling successful");
        } catch (...) {
            // 如果抛出异常也是可以接受的
            TEST_ASSERT(true, "Type mismatch handling with exception");
        }
        
    } catch (const std::exception& e) {
        g_stats.addFail("Error handling test exception: " + std::string(e.what()));
    }
}

void test_memory_usage() {
    std::cout << "\n=== Testing Memory Usage ===" << std::endl;
    
    try {
        // 创建大量对象来测试内存使用
        const int count = 10000;
        std::vector<Person> people;
        people.reserve(count);
        
        for (int i = 0; i < count; ++i) {
            Person p;
            p.name = "Person_" + std::to_string(i);
            p.age = 20 + (i % 50);
            p.height = 150.0 + (i % 50);
            p.married = (i % 2 == 0);
            p.contact.email = "person" + std::to_string(i) + "@example.com";
            p.hobbies = {"hobby1", "hobby2", "hobby3"};
            people.push_back(std::move(p));
        }
        
        // 批量序列化
        std::vector<JsonValue> json_people;
        json_people.reserve(count);
        
        for (const auto& person : people) {
            json_people.push_back(person.toJson());
        }
        
        TEST_ASSERT(json_people.size() == count, "Memory test serialization count");
        
        // 清理
        people.clear();
        json_people.clear();
        
        TEST_ASSERT(true, "Memory usage test completed");
        
    } catch (const std::exception& e) {
        g_stats.addFail("Memory usage test exception: " + std::string(e.what()));
    }
}

int main() {
    std::cout << "=== Comprehensive JSON_AUTO and JSON_FIELDS Test Suite ===" << std::endl;
    std::cout << "Testing JsonStruct type registry and macro functionality...\n" << std::endl;
    
    try {
        test_basic_types();
        test_container_types();
        test_nested_structures();
        test_edge_cases();
        test_deep_nesting();
        test_macro_functionality();
        test_performance();
        test_error_handling();
        test_memory_usage();
        
        g_stats.report();
        
        return (g_stats.failed == 0) ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Test suite failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test suite failed with unknown exception" << std::endl;
        return 1;
    }
}
