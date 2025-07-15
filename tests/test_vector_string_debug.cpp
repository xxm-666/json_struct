#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include "../src/std_types/std_registry.h"
#include <vector>
#include <string>
#include <iostream>

using namespace JsonStruct;

TEST(VectorStringDebug_DirectTest) {
    // 确保容器类型已注册
    StdTypesRegistration::registerAllStdTypes();
    
    std::cout << "=== Debugging vector<string> ===" << std::endl;
    
    // 创建测试数据
    std::vector<std::string> original = {"hello", "world", "test"};
    std::cout << "Original vector size: " << original.size() << std::endl;
    for (size_t i = 0; i < original.size(); ++i) {
        std::cout << "  [" << i << "]: '" << original[i] << "'" << std::endl;
    }
    
    // 序列化
    JsonValue json = TypeRegistry::instance().toJson(original);
    std::cout << "JSON created, isArray: " << json.isArray() << std::endl;
    
    if (json.isArray()) {
        const auto& arr = json.toArray();
        std::cout << "JSON array size: " << arr.size() << std::endl;
        for (size_t i = 0; i < arr.size(); ++i) {
            std::cout << "  JSON[" << i << "]: isString=" << arr[i].isString() 
                     << ", value='" << arr[i].toString("EMPTY") << "'" << std::endl;
        }
    }
    
    // 反序列化
    std::vector<std::string> restored = TypeRegistry::instance().fromJson<std::vector<std::string>>(json, std::vector<std::string>());
    std::cout << "Restored vector size: " << restored.size() << std::endl;
    for (size_t i = 0; i < restored.size(); ++i) {
        std::cout << "  Restored[" << i << "]: '" << restored[i] << "'" << std::endl;
    }
    
    // 验证
    ASSERT_EQ(original.size(), restored.size());
    for (size_t i = 0; i < original.size(); ++i) {
        std::cout << "Comparing [" << i << "]: '" << original[i] << "' vs '" << restored[i] << "'" << std::endl;
        ASSERT_EQ(original[i], restored[i]);
    }
}

TEST(StringRegistration_Check) {
    // 检查string类型是否正确注册
    std::cout << "=== Checking string registration ===" << std::endl;
    
    std::string original = "test_string";
    JsonValue json = TypeRegistry::instance().toJson(original);
    
    std::cout << "String JSON: isString=" << json.isString() << ", value='" << json.toString() << "'" << std::endl;
    
    std::string restored = TypeRegistry::instance().fromJson<std::string>(json, "default");
    std::cout << "Restored string: '" << restored << "'" << std::endl;
    
    ASSERT_EQ(original, restored);
}

int main() {
    std::cout << "=== Vector String Debug Test ===" << std::endl;
    
    TestFramework::TestSuite& suite = TestFramework::TestSuite::instance();
    int result = suite.runAll();
    
    if (result == 0) {
        std::cout << "✅ All debug tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "❌ Some debug tests FAILED!" << std::endl;
        return 1;
    }
}
