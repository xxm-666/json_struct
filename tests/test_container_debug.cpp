#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include "../src/std_types/std_registry.h"
#include <vector>
#include <string>

using namespace JsonStruct;

// 简单容器测试
struct SimpleContainer {
    JSON_AUTO(numbers)
    std::vector<int> numbers;
};

TEST(Container_BasicVectorInt) {
    // 确保容器类型已注册
    StdTypesRegistration::registerAllStdTypes();
    
    // 直接测试vector<int>
    std::vector<int> vec = {1, 2, 3};
    JsonValue json = TypeRegistry::instance().toJson(vec);
    
    std::vector<int> restored = TypeRegistry::instance().fromJson<std::vector<int>>(json, std::vector<int>());
    
    ASSERT_EQ(vec.size(), restored.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], restored[i]);
    }
}

TEST(Container_BasicVectorString) {
    // 直接测试vector<string>
    std::vector<std::string> vec = {"hello", "world"};
    JsonValue json = TypeRegistry::instance().toJson(vec);
    
    std::vector<std::string> restored = TypeRegistry::instance().fromJson<std::vector<std::string>>(json, std::vector<std::string>());
    
    ASSERT_EQ(vec.size(), restored.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        ASSERT_EQ(vec[i], restored[i]);
    }
}

TEST(Container_StructWithVector) {
    SimpleContainer obj;
    obj.numbers = {10, 20, 30};
    
    JsonValue json = JsonValue(toJson(obj));
    ASSERT_TRUE(json.isObject());
    
    SimpleContainer restored;
    fromJson(restored, json);
    
    ASSERT_EQ(obj.numbers.size(), restored.numbers.size());
    for (size_t i = 0; i < obj.numbers.size(); ++i) {
        ASSERT_EQ(obj.numbers[i], restored.numbers[i]);
    }
}

int main() {
    std::cout << "=== Container Types Debug Test ===" << std::endl;
    
    TestFramework::TestSuite& suite = TestFramework::TestSuite::instance();
    int result = suite.runAll();
    
    if (result == 0) {
        std::cout << "✅ All container tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some container tests FAILED!" << std::endl;
        return 1;
    }
}
