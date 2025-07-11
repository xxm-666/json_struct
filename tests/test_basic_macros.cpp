#include "jsonstruct.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace JsonStruct;

int main() {
    std::cout << "=== JsonStruct Basic Macro Test ===" << std::endl;
    
    // 测试1: 基础字段访问
    std::cout << "\nTest 1: Basic Field Access" << std::endl;
    struct BasicStruct {
        int id = 42;
        std::string name = "test";
        bool active = true;
        JSON_FIELDS(id, name, active)
    };
    
    BasicStruct obj;
    obj.id = 999;
    obj.name = "modified";
    obj.active = false;
    
    // 测试字段名称
    auto names = obj.get_field_names();
    assert(names.size() == 3);
    assert(names[0] == "id");
    assert(names[1] == "name");
    assert(names[2] == "active");
    std::cout << "✓ Field names correct: " << names[0] << ", " << names[1] << ", " << names[2] << std::endl;
    
    // 测试字段访问
    auto fields = obj.json_fields();
    assert(std::get<0>(fields) == 999);
    assert(std::get<1>(fields) == "modified");
    assert(std::get<2>(fields) == false);
    std::cout << "✓ Field values correct: " << std::get<0>(fields) << ", " 
              << std::get<1>(fields) << ", " << std::get<2>(fields) << std::endl;
    
    // 测试2: 简化的字段访问
    std::cout << "\nTest 2: Simple Field Access" << std::endl;
    struct SimpleStruct {
        int value = 123;
        double ratio = 3.14;
        JSON_FIELDS(value, ratio)
    };
    
    SimpleStruct simple_obj;
    auto simple_names = simple_obj.get_field_names();
    assert(simple_names.size() == 2);
    assert(simple_names[0] == "value");
    assert(simple_names[1] == "ratio");
    std::cout << "✓ Simple field names correct: " << simple_names[0] << ", " << simple_names[1] << std::endl;
    
    auto simple_fields = simple_obj.json_fields();
    assert(std::get<0>(simple_fields) == 123);
    assert(std::get<1>(simple_fields) == 3.14);
    std::cout << "✓ Simple field values correct: " << std::get<0>(simple_fields) << ", " 
              << std::get<1>(simple_fields) << std::endl;
    
    // 测试3: 字段修改
    std::cout << "\nTest 3: Field Modification" << std::endl;
    auto mutable_fields = obj.json_fields();
    std::get<0>(mutable_fields) = 777;
    std::get<1>(mutable_fields) = "changed";
    std::get<2>(mutable_fields) = true;
    
    assert(obj.id == 777);
    assert(obj.name == "changed");
    assert(obj.active == true);
    std::cout << "✓ Field modification successful: " << obj.id << ", " 
              << obj.name << ", " << obj.active << std::endl;
    
    // 测试4: 类型注册表访问
    std::cout << "\nTest 4: Type Registry Access" << std::endl;
    TypeRegistry& reg = TypeRegistry::instance();
    auto registered_types = reg.getRegisteredTypes();
    std::cout << "✓ Type registry accessible, " << registered_types.size() << " types registered" << std::endl;
    
    // 测试5: 边界情况
    std::cout << "\nTest 5: Edge Cases" << std::endl;
    
    // 单字段结构
    struct SingleField {
        int value = 100;
        JSON_FIELDS(value)
    };
    
    SingleField single;
    auto single_names = single.get_field_names();
    assert(single_names.size() == 1);
    assert(single_names[0] == "value");
    
    auto single_fields = single.json_fields();
    assert(std::get<0>(single_fields) == 100);
    std::cout << "✓ Single field struct works correctly" << std::endl;
    
    // 多字段结构 - 使用JSON_FIELDS而不是JSON_AUTO
    struct ManyFields {
        int a = 1;
        int b = 2;
        int c = 3;
        int d = 4;
        int e = 5;
        JSON_FIELDS(a, b, c, d, e)
    };
    
    ManyFields many;
    auto many_names = many.get_field_names();
    assert(many_names.size() == 5);
    assert(many_names[0] == "a");
    assert(many_names[4] == "e");
    
    auto many_fields = many.json_fields();
    assert(std::get<0>(many_fields) == 1);
    assert(std::get<4>(many_fields) == 5);
    std::cout << "✓ Multiple field struct works correctly" << std::endl;
    
    // 测试6: 字段名字符串
    std::cout << "\nTest 6: Field Name String" << std::endl;
    const char* field_names_str = obj.json_field_names();
    std::string names_str(field_names_str);
    assert(names_str == "id, name, active");
    std::cout << "✓ Field name string correct: \"" << names_str << "\"" << std::endl;
    
    // 最终总结
    std::cout << "\n=== ALL TESTS PASSED! ===" << std::endl;
    std::cout << "✅ JSON_FIELDS macro working correctly" << std::endl;
    std::cout << "✅ Field access and modification working" << std::endl;
    std::cout << "✅ Field name parsing working" << std::endl;
    std::cout << "✅ Type registry accessible" << std::endl;
    std::cout << "✅ Edge cases handled properly" << std::endl;
    std::cout << "\nJsonStruct macro system is functioning correctly!" << std::endl;
    std::cout << "\nNote: JSON_AUTO macro needs further development for serialization/deserialization." << std::endl;
    
    return 0;
}
