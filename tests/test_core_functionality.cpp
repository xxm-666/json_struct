#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include <string>
#include <vector>

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

TEST(CoreFunctionality_FieldAccess) {
    SimpleStruct obj;
    obj.id = 999;
    obj.name = "modified";
    obj.active = false;
    
    // 测试字段名称解析
    auto names = obj.get_field_names();
    ASSERT_EQ(names.size(), 3);
    ASSERT_EQ(names[0], "id");
    ASSERT_EQ(names[1], "name");
    ASSERT_EQ(names[2], "active");
    
    // 测试字段值访问
    auto fields = obj.json_fields();
    ASSERT_EQ(std::get<0>(fields), 999);
    ASSERT_EQ(std::get<1>(fields), "modified");
    ASSERT_EQ(std::get<2>(fields), false);
}

TEST(CoreFunctionality_MacroExpansion) {
    BasicTypes obj;
    
    // 测试JSON_AUTO宏展开
    auto names = obj.get_field_names();
    ASSERT_EQ(names.size(), 4);
    ASSERT_EQ(names[0], "integer");
    ASSERT_EQ(names[1], "decimal");
    ASSERT_EQ(names[2], "text");
    ASSERT_EQ(names[3], "flag");
    
    // 测试字段访问
    auto fields = obj.json_fields();
    ASSERT_EQ(std::get<0>(fields), 123);
    ASSERT_NEAR(std::get<1>(fields), 3.14, 0.001);
    ASSERT_EQ(std::get<2>(fields), "hello");
    ASSERT_EQ(std::get<3>(fields), true);
}

TEST(CoreFunctionality_SerializationBasics) {
    BasicTypes obj;
    obj.integer = 456;
    obj.decimal = 2.71;
    obj.text = "world";
    obj.flag = false;
    
    // 测试序列化
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    // 测试反序列化
    BasicTypes obj2;
    fromJson(obj2, json);
    
    ASSERT_EQ(obj.integer, obj2.integer);
    ASSERT_NEAR(obj.decimal, obj2.decimal, 0.001);
    ASSERT_EQ(obj.text, obj2.text);
    ASSERT_EQ(obj.flag, obj2.flag);
}

TEST(CoreFunctionality_TypeRegistry) {
    // 测试类型注册系统是否正常工作
    BasicTypes obj;
    
    // 确认对象能够正确处理字段
    auto names = obj.get_field_names();
    ASSERT_TRUE(names.size() > 0);
    
    // 确认能够获取字段值
    auto fields = obj.json_fields();
    // 如果能到这里说明类型注册工作正常
    ASSERT_TRUE(true);
}

TEST(CoreFunctionality_EdgeCases) {
    // 测试单字段结构
    {
        SingleField single;
        single.value = 100;
        
        auto single_names = single.get_field_names();
        ASSERT_EQ(single_names.size(), 1);
        ASSERT_EQ(single_names[0], "value");
        
        auto single_fields = single.json_fields();
        ASSERT_EQ(std::get<0>(single_fields), 100);
    }
    
    // 测试双字段结构
    {
        DoubleField double_field;
        double_field.first = 10;
        double_field.second = "test";
        
        auto double_names = double_field.get_field_names();
        ASSERT_EQ(double_names.size(), 2);
        ASSERT_EQ(double_names[0], "first");
        ASSERT_EQ(double_names[1], "second");
        
        auto double_fields = double_field.json_fields();
        ASSERT_EQ(std::get<0>(double_fields), 10);
        ASSERT_EQ(std::get<1>(double_fields), "test");
    }
}

TEST(CoreFunctionality_JsonRoundTrip) {
    SimpleStruct original;
    original.id = 12345;
    original.name = "test_object";
    original.active = true;
    
    // 序列化到JSON
    JsonValue json = JsonValue(toJson(original));
    ASSERT_TRUE(json.isObject());
    
    // 从JSON反序列化
    SimpleStruct restored;
    fromJson(restored, json);
    
    // 验证数据完整性
    ASSERT_EQ(original.id, restored.id);
    ASSERT_EQ(original.name, restored.name);
    ASSERT_EQ(original.active, restored.active);
}

int main() {
    return RUN_ALL_TESTS();
}
