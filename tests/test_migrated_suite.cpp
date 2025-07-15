#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include "../src/std_types/std_registry.h"

// 包含所有迁移的测试
#include <iostream>

// 从 test_core_functionality_migrated.cpp 复制的测试结构
using namespace JsonStruct;

// 简单结构体测试
struct SimpleStruct {
    JSON_AUTO(id, name, active)
    int id = 42;
    std::string name = "test";
    bool active = true;
};

// 基础类型测试
struct BasicTypes {
    JSON_AUTO(integer, decimal, text, flag)
    int integer = 123;
    double decimal = 3.14;
    std::string text = "hello";
    bool flag = true;
};

// 单字段结构
struct SingleField {
    JSON_AUTO(value)
    int value = 100;
};

// 双字段结构
struct DoubleField {
    JSON_AUTO(first, second)
    int first = 10;
    std::string second = "test";
};

// 从 test_json_auto_migrated.cpp 复制的测试结构
struct ContainerTypes {
    JSON_AUTO(numbers, words)
    std::vector<int> numbers;
    std::vector<std::string> words;
};

struct Address {
    JSON_AUTO(street, city, zipcode)
    std::string street;
    std::string city;
    int zipcode = 0;
};

struct Person {
    JSON_AUTO(name, age, address)
    std::string name;
    int age = 0;
    Address address;
};

struct EdgeCases {
    JSON_AUTO(empty_string, empty_vector, zero_value)
    std::string empty_string = "";
    std::vector<int> empty_vector;
    int zero_value = 0;
};

// === 核心功能测试 ===
TEST(MigratedCore_FieldAccess) {
    SimpleStruct obj;
    obj.id = 999;
    obj.name = "modified";
    obj.active = false;
    
    auto names = obj.get_field_names();
    ASSERT_EQ(names.size(), 3);
    ASSERT_EQ(names[0], "id");
    ASSERT_EQ(names[1], "name");
    ASSERT_EQ(names[2], "active");
    
    auto fields = obj.json_fields();
    ASSERT_EQ(std::get<0>(fields), 999);
    ASSERT_EQ(std::get<1>(fields), "modified");
    ASSERT_EQ(std::get<2>(fields), false);
}

TEST(MigratedCore_MacroExpansion) {
    BasicTypes obj;
    
    auto names = obj.get_field_names();
    ASSERT_EQ(names.size(), 4);
    ASSERT_EQ(names[0], "integer");
    ASSERT_EQ(names[1], "decimal");
    ASSERT_EQ(names[2], "text");
    ASSERT_EQ(names[3], "flag");
    
    auto fields = obj.json_fields();
    ASSERT_EQ(std::get<0>(fields), 123);
    ASSERT_NEAR(std::get<1>(fields), 3.14, 0.001);
    ASSERT_EQ(std::get<2>(fields), "hello");
    ASSERT_EQ(std::get<3>(fields), true);
}

TEST(MigratedCore_Serialization) {
    BasicTypes obj;
    obj.integer = 456;
    obj.decimal = 2.71;
    obj.text = "world";
    obj.flag = false;
    
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    BasicTypes obj2;
    fromJson(obj2, json);
    
    ASSERT_EQ(obj.integer, obj2.integer);
    ASSERT_NEAR(obj.decimal, obj2.decimal, 0.001);
    ASSERT_EQ(obj.text, obj2.text);
    ASSERT_EQ(obj.flag, obj2.flag);
}

// === JSON自动序列化测试 ===
TEST(MigratedJsonAuto_BasicTypes) {
    BasicTypes obj;
    obj.flag = true;
    obj.integer = 42;
    obj.decimal = 3.14159;
    obj.text = "Hello, World!";
    
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    BasicTypes restored;
    fromJson(restored, json);
    
    ASSERT_EQ(obj.flag, restored.flag);
    ASSERT_EQ(obj.integer, restored.integer);
    ASSERT_NEAR(obj.decimal, restored.decimal, 0.00001);
    ASSERT_EQ(obj.text, restored.text);
}

TEST(MigratedJsonAuto_ContainerTypes) {
    ContainerTypes obj;
    obj.numbers = {1, 2, 3, 4, 5};
    obj.words = {"hello", "world", "test"};
    
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    ContainerTypes restored;
    fromJson(restored, json);
    
    ASSERT_EQ(obj.numbers.size(), restored.numbers.size());
    for (size_t i = 0; i < obj.numbers.size(); ++i) {
        ASSERT_EQ(obj.numbers[i], restored.numbers[i]);
    }
    
    ASSERT_EQ(obj.words.size(), restored.words.size());
    for (size_t i = 0; i < obj.words.size(); ++i) {
        ASSERT_EQ(obj.words[i], restored.words[i]);
    }
}

TEST(MigratedJsonAuto_NestedStructures) {
    Person person;
    person.name = "John Doe";
    person.age = 30;
    person.address.street = "123 Main St";
    person.address.city = "Anytown";
    person.address.zipcode = 12345;
    
    JsonValue json = JsonValue(toJson(person));
    ASSERT_TRUE(json.isObject());
    
    Person restored;
    fromJson(restored, json);
    
    ASSERT_EQ(person.name, restored.name);
    ASSERT_EQ(person.age, restored.age);
    ASSERT_EQ(person.address.street, restored.address.street);
    ASSERT_EQ(person.address.city, restored.address.city);
    ASSERT_EQ(person.address.zipcode, restored.address.zipcode);
}

int main() {
    std::cout << "=== JsonStruct Library - Migrated Tests Suite ===" << std::endl;
    std::cout << "Running migrated tests from original test suite..." << std::endl;
    std::cout << std::endl;
    
    // 确保容器类型已注册
    JsonStruct::StdTypesRegistration::registerAllStdTypes();
    
    TestFramework::TestSuite& suite = TestFramework::TestSuite::instance();
    int result = suite.runAll();
    
    std::cout << std::endl;
    std::cout << "=== Migrated Test Results Summary ===" << std::endl;
    
    if (result == 0) {
        std::cout << "✅ All migrated tests PASSED!" << std::endl;
        std::cout << "✅ Core functionality migration successful" << std::endl;
        std::cout << "✅ JSON auto-serialization migration successful" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some migrated tests FAILED!" << std::endl;
        return 1;
    }
}
