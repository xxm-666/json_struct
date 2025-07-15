#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include <string>
#include <vector>
#include <map>

using namespace JsonStruct;

// 基础类型测试
struct BasicTypes {
    JSON_AUTO(flag, integer, decimal, text)
    bool flag = false;
    int integer = 0;
    double decimal = 0.0;
    std::string text = "";
};

// 容器类型测试
struct ContainerTypes {
    JSON_AUTO(numbers, words)
    std::vector<int> numbers;
    std::vector<std::string> words;
    // 注意：map类型可能需要特殊处理，暂时先测试vector
};

// 嵌套结构体测试
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

// 边界情况测试
struct EdgeCases {
    JSON_AUTO(empty_string, empty_vector, zero_value)
    std::string empty_string = "";
    std::vector<int> empty_vector;
    int zero_value = 0;
};

TEST(JsonAuto_BasicTypes) {
    BasicTypes obj;
    obj.flag = true;
    obj.integer = 42;
    obj.decimal = 3.14159;
    obj.text = "Hello, World!";
    
    // 测试序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 测试反序列化
    BasicTypes restored;
    fromJson(restored, json);
    
    ASSERT_EQ(obj.flag, restored.flag);
    ASSERT_EQ(obj.integer, restored.integer);
    ASSERT_NEAR(obj.decimal, restored.decimal, 0.00001);
    ASSERT_EQ(obj.text, restored.text);
}

TEST(JsonAuto_ContainerTypes) {
    ContainerTypes obj;
    obj.numbers = {1, 2, 3, 4, 5};
    obj.words = {"hello", "world", "test"};
    
    // 测试序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 测试反序列化
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

TEST(JsonAuto_NestedStructures) {
    Person person;
    person.name = "John Doe";
    person.age = 30;
    person.address.street = "123 Main St";
    person.address.city = "Anytown";
    person.address.zipcode = 12345;
    
    // 测试序列化
    JsonValue json = JsonValue(toJson(person));
    ASSERT_TRUE(json.isObject());
    
    // 测试反序列化
    Person restored;
    fromJson(restored, json);
    
    ASSERT_EQ(person.name, restored.name);
    ASSERT_EQ(person.age, restored.age);
    ASSERT_EQ(person.address.street, restored.address.street);
    ASSERT_EQ(person.address.city, restored.address.city);
    ASSERT_EQ(person.address.zipcode, restored.address.zipcode);
}

TEST(JsonAuto_EdgeCases) {
    EdgeCases obj;
    // 保持默认值：空字符串、空向量、零值
    
    // 测试序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 测试反序列化
    EdgeCases restored;
    fromJson(restored, json);
    
    ASSERT_EQ(obj.empty_string, restored.empty_string);
    ASSERT_EQ(obj.empty_vector.size(), restored.empty_vector.size());
    ASSERT_EQ(obj.zero_value, restored.zero_value);
}

TEST(JsonAuto_FieldsAccess) {
    BasicTypes obj;
    obj.flag = true;
    obj.integer = 100;
    obj.decimal = 2.5;
    obj.text = "test";
    
    // 测试字段名称
    auto names = obj.get_field_names();
    ASSERT_EQ(names.size(), 4);
    ASSERT_EQ(names[0], "flag");
    ASSERT_EQ(names[1], "integer");
    ASSERT_EQ(names[2], "decimal");
    ASSERT_EQ(names[3], "text");
    
    // 测试字段值访问
    auto fields = obj.json_fields();
    ASSERT_EQ(std::get<0>(fields), true);
    ASSERT_EQ(std::get<1>(fields), 100);
    ASSERT_NEAR(std::get<2>(fields), 2.5, 0.001);
    ASSERT_EQ(std::get<3>(fields), "test");
}

TEST(JsonAuto_TypeSafety) {
    BasicTypes obj1;
    obj1.integer = 123;
    
    Person obj2;
    obj2.age = 456;
    
    // 确保不同类型的结构有不同的字段
    auto names1 = obj1.get_field_names();
    auto names2 = obj2.get_field_names();
    
    ASSERT_NE(names1.size(), names2.size());
    
    // 确保各自的JSON序列化正常
    JsonValue json1 = JsonValue(toJson(obj1));
    JsonValue json2 = JsonValue(toJson(obj2));
    
    ASSERT_TRUE(json1.isObject());
    ASSERT_TRUE(json2.isObject());
}

TEST(JsonAuto_PerformanceBasic) {
    BasicTypes obj;
    obj.flag = true;
    obj.integer = 999;
    obj.decimal = 123.456;
    obj.text = "performance test";
    
    // 执行多次序列化/反序列化循环
    for (int i = 0; i < 100; ++i) {
        JsonValue json = JsonValue(toJson(obj));
        BasicTypes restored;
        fromJson(restored, json);
        
        // 验证最后一次的正确性
        if (i == 99) {
            ASSERT_EQ(obj.flag, restored.flag);
            ASSERT_EQ(obj.integer, restored.integer);
            ASSERT_EQ(obj.text, restored.text);
        }
    }
    
    // 如果到达这里说明性能测试通过
    ASSERT_TRUE(true);
}

int main() {
    return RUN_ALL_TESTS();
}
