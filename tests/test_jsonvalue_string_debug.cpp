// 深度调试JsonValue的string处理
#include "../src/json_engine/json_value.h"
#include "../src/std_types/container_registry.h"
#include "test_framework.h"
#include <iostream>

using namespace JsonStruct;

TEST(JsonValueDebug_DirectStringConstruction) {
    std::cout << "=== Direct JsonValue string construction ===" << std::endl;
    
    std::string test_str = "hello";
    std::cout << "Original string: '" << test_str << "'" << std::endl;
    
    JsonValue json_val(test_str);
    std::cout << "JsonValue created" << std::endl;
    std::cout << "isString(): " << json_val.isString() << std::endl;
    std::cout << "type(): " << static_cast<int>(json_val.type()) << std::endl;
    
    auto getString_result = json_val.getString();
    if (getString_result) {
        std::cout << "getString() succeeded: '" << *getString_result << "'" << std::endl;
    } else {
        std::cout << "getString() failed" << std::endl;
    }
    
    std::string toString_result = json_val.toString("DEFAULT");
    std::cout << "toString(): '" << toString_result << "'" << std::endl;
    std::cout << "Expected: '" << test_str << "'" << std::endl;
    std::cout << "Match: " << (toString_result == test_str) << std::endl;
    
    ASSERT_TRUE(json_val.isString());
    ASSERT_EQ(toString_result, test_str);
}

TEST(JsonValueDebug_ArrayWithStrings) {
    std::cout << "=== Array with strings ===" << std::endl;
    
    std::vector<std::string> original = {"hello", "world"};
    
    JsonValue::ArrayType arr;
    for (const auto& item : original) {
        std::cout << "Adding string: '" << item << "'" << std::endl;
        JsonValue json_item(item);
        std::cout << "  JsonValue isString: " << json_item.isString() << std::endl;
        std::cout << "  JsonValue toString: '" << json_item.toString("FAIL") << "'" << std::endl;
        arr.push_back(json_item);
    }
    
    JsonValue json_array(arr);
    std::cout << "Array created, isArray: " << json_array.isArray() << std::endl;
    
    const auto& restored_arr = json_array.toArray();
    std::cout << "Array size: " << restored_arr.size() << std::endl;
    
    for (size_t i = 0; i < restored_arr.size(); ++i) {
        std::cout << "  Array[" << i << "]:" << std::endl;
        std::cout << "    isString: " << restored_arr[i].isString() << std::endl;
        std::cout << "    toString: '" << restored_arr[i].toString("FAIL") << "'" << std::endl;
    }
    
    ASSERT_TRUE(json_array.isArray());
    ASSERT_EQ(restored_arr.size(), 2);
    ASSERT_EQ(restored_arr[0].toString("FAIL"), "hello");
    ASSERT_EQ(restored_arr[1].toString("FAIL"), "world");
}

int main() {
    std::cout << "Starting JsonValue String Debug Tests..." << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✓ All JsonValue debug tests PASSED!" << std::endl;
    } else {
        std::cout << "✗ Some JsonValue debug tests FAILED!" << std::endl;
    }
    
    return result;
}
