#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include <string>

using namespace JsonStruct;

// 定义测试用结构体
struct BasicTypes {
    JSON_AUTO(flag, integer, decimal, text)
    bool flag = false;
    int integer = 0;
    double decimal = 0.0;
    std::string text;
};

struct Person {
    JSON_AUTO(name, age)
    std::string name;
    int age = 0;
};

TEST(BasicTypeSerialization) {
    BasicTypes obj;
    obj.flag = true;
    obj.integer = 42;
    obj.decimal = 3.14159;
    obj.text = "Hello World";
    
    auto jsonVal = obj.toJson();
    ASSERT_TRUE(jsonVal.isObject());

    if(const auto& jsonObj = jsonVal.toObject()) {
        ASSERT_EQ(true, jsonObj->get().at("flag").toBool());
        ASSERT_EQ(42, jsonObj->get().at("integer").toInt());
        ASSERT_NEAR(3.14159, jsonObj->get().at("decimal").toDouble(), 0.00001);
        ASSERT_EQ("Hello World", jsonObj->get().at("text").toString());
    } else {
        ASSERT_TRUE(false);
    }
}

TEST(BasicTypeDeserialization) {
    BasicTypes original;
    original.flag = true;
    original.integer = 42;
    original.decimal = 3.14159;
    original.text = "Hello World";
    
    auto jsonVal = original.toJson();
    
    BasicTypes restored;
    restored.fromJson(jsonVal);
    
    ASSERT_EQ(original.flag, restored.flag);
    ASSERT_EQ(original.integer, restored.integer);
    ASSERT_NEAR(original.decimal, restored.decimal, 0.00001);
    ASSERT_EQ(original.text, restored.text);
}

TEST(PersonSerialization) {
    Person person;
    person.name = "Alice";
    person.age = 30;
    
    auto jsonVal = person.toJson();
    ASSERT_TRUE(jsonVal.isObject());
    
    if(const auto& jsonObj = jsonVal.toObject()) {
        ASSERT_EQ("Alice", jsonObj->get().at("name").toString());
        ASSERT_EQ(30, jsonObj->get().at("age").toInt());
    } else {
        ASSERT_TRUE(false);
    }
}

TEST(PersonRoundTrip) {
    Person original;
    original.name = "Bob";
    original.age = 25;
    
    auto jsonVal = original.toJson();
    Person restored;
    restored.fromJson(jsonVal);
    
    ASSERT_EQ(original.name, restored.name);
    ASSERT_EQ(original.age, restored.age);
}

int main() {
    return RUN_ALL_TESTS();
}
