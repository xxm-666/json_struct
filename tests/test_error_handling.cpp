#include <test_framework/test_framework.h>
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include <string>

using namespace JsonStruct;

// 测试错误处理和边界情况

struct SimpleStruct {
    JSON_AUTO(number, text)
    int number = 0;
    std::string text = "";
};

TEST(ErrorHandling_InvalidJSON) {
    std::string invalidJson = "{ invalid json }";
    
    // 测试解析无效JSON - 使用简单的try-catch处理
    bool caught_exception = false;
    try {
        JsonValue json = JsonValue::parse(invalidJson);
    } catch (...) {
        caught_exception = true;
    }
    
    // 期望捕获到异常
    ASSERT_TRUE(caught_exception);
}

TEST(ErrorHandling_EmptyJSON) {
    std::string jsonStr = "{}";
    
    JsonValue json = JsonValue::parse(jsonStr);
    SimpleStruct obj;
    fromJson(obj, json);
    
    // 所有字段应该使用默认值
    ASSERT_EQ(obj.number, 0);
    ASSERT_TRUE(obj.text.empty());
}

TEST(BoundaryConditions_LargeNumbers) {
    SimpleStruct obj;
    obj.number = 2147483647; // INT_MAX
    obj.text = "max_int";
    
    auto json_map = toJson(obj);
    JsonValue json(json_map);
    
    SimpleStruct obj2;
    fromJson(obj2, json);
    
    ASSERT_EQ(obj.number, obj2.number);
    ASSERT_EQ(obj.text, obj2.text);
}

int main() {
    return RUN_ALL_TESTS();
}
