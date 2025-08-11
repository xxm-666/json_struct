#include "../test_framework/test_framework.h"
#include "../src/type_registry/registry_core.h"
#include "../src/type_registry/auto_serializer.h"
#include "../src/json_engine/json_value.h"
#include <string>
#include <vector>
#include <limits>
#include <stdexcept>

using namespace JsonStruct;

// 测试极端异常场景的数据结构
struct ExtremeExceptionTest {
    JSON_AUTO(id, name, largeNumber, smallNumber, data)
    
    int id;
    std::string name;
    long long largeNumber;
    double smallNumber;
    std::vector<int> data;
};

TEST(ExtremeException_InvalidJSON) {
    // 测试各种无效的JSON字符串
    std::vector<std::string> invalidJSONs = {
        "{" ,  // 不完整的对象
        "}" ,  // 只有结束括号
        "[" ,  // 不完整的数组
        "]" ,  // 只有结束方括号
        "{\"key\": }" ,  // 缺少值
        "{\"key\": null, }" ,  // 尾随逗号
        "{\"key\": 'single_quote'}" ,  // 使用单引号
        "{\"key\": 1.2.3}" ,  // 无效的数字格式
        "{\"key\": 1e999}" ,  // 过大的指数
        "{\"key\": NaN}" ,  // NaN值（除非特别支持）
        "{\"key\": Infinity}" ,  // Infinity值（除非特别支持）
        R"({"key": \uXXXX})" ,  // 无效的Unicode转义
        R"({"key": "unclosed string)" ,  // 未闭合的字符串
    };
    
    int parseFailures = 0;
    
    for (const auto& jsonStr : invalidJSONs) {
        try {
            JsonValue parsed = JsonValue::parse(jsonStr);
            // 如果解析成功，但应该是无效的JSON，这可能是一个问题
            std::cout << "Unexpected successful parsing of invalid JSON: " << jsonStr << std::endl;
        } catch (const std::exception& e) {
            // 这是预期的结果
            parseFailures++;
        } catch (...) {
            // 其他类型的异常
            parseFailures++;
        }
    }
    
    // 验证大部分无效JSON都被正确拒绝
    ASSERT_TRUE(parseFailures >= invalidJSONs.size() * 0.8); // 至少80%应该被拒绝

    std::cout << "Invalid JSON parsing test - " << parseFailures
              << " out of " << invalidJSONs.size() << " invalid JSON strings were correctly rejected" << std::endl;
}

TEST(ExtremeException_NumericOverflow) {
    // 测试数值溢出场景
    
    // 测试整数溢出
    std::string intOverflowJSON = "{\"value\": 9223372036854775808}"; // 超过int64_t最大值
    try {
        JsonValue parsed = JsonValue::parse(intOverflowJSON);
        // 检查是否正确处理溢出
        if (parsed["value"].isNumber()) {
            // 可能被解析为double或特殊值
            std::cout << "Large integer parsed as number" << std::endl;
        }
    } catch (...) {
        // 或者抛出异常
        std::cout << "Large integer correctly caused parsing exception" << std::endl;
    }
    
    // 测试double溢出
    std::string doubleOverflowJSON = "{\"value\": 1e400}"; // 超过double范围
    try {
        JsonValue parsed = JsonValue::parse(doubleOverflowJSON);
        // 检查是否正确处理溢出
        if (parsed["value"].isNumber()) {
            double val = parsed["value"].toDouble();
            if (std::isinf(val)) {
                std::cout << "Double overflow correctly parsed as infinity" << std::endl;
            }
        }
    } catch (...) {
        // 或者抛出异常
        std::cout << "Double overflow correctly caused parsing exception" << std::endl;
    }
    
    // 测试负double溢出
    std::string negativeDoubleOverflowJSON = "{\"value\": -1e400}";
    try {
        JsonValue parsed = JsonValue::parse(negativeDoubleOverflowJSON);
        // 检查是否正确处理溢出
        if (parsed["value"].isNumber()) {
            double val = parsed["value"].toDouble();
            if (std::isinf(val)) {
                std::cout << "Negative double overflow correctly parsed as negative infinity" << std::endl;
            }
        }
    } catch (...) {
        // 或者抛出异常
        std::cout << "Negative double overflow correctly caused parsing exception" << std::endl;
    }
}

TEST(ExtremeException_MemoryAllocationFailure) {
    // 测试内存分配失败场景
    // 创建一个非常大的数组来测试内存限制
    
    try {
        // 创建一个包含大量元素的vector
        std::vector<int> hugeVector;
        
        // 尝试reserve一个非常大的空间
        try {
            hugeVector.reserve(1000000000ULL); // 10亿个元素
            std::cout << "Successfully reserved space for 1 billion integers" << std::endl;
        } catch (const std::bad_alloc& e) {
            std::cout << "Memory allocation failed as expected for 1 billion integers" << std::endl;
        }
        
        // 创建一个深度嵌套的结构来测试栈溢出
        std::string deepNestedJSON = "{";
        for (int i = 0; i < 10000; ++i) {
            deepNestedJSON += "\"level" + std::to_string(i) + "\": {";
        }
        
        // 添加中心值
        deepNestedJSON += "\"value\": 42";
        
        // 添加结束括号
        for (int i = 0; i < 10000; ++i) {
            deepNestedJSON += "}";
            if (i < 9999) deepNestedJSON += ",";
        }
        deepNestedJSON += "}";
        
        try {
            JsonValue parsed = JsonValue::parse(deepNestedJSON);
            std::cout << "Successfully parsed deeply nested JSON with " << parsed.dump().size() << " characters" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Deeply nested JSON parsing failed as expected: " << e.what() << std::endl;
        }
        
    } catch (const std::bad_alloc& e) {
        std::cout << "Memory allocation failed as expected" << std::endl;
    } catch (...) {
        std::cout << "Other exception occurred during memory test" << std::endl;
    }
}

TEST(ExtremeException_TypeConversionErrors) {
    // 测试类型转换错误
    
    // 创建一个包含各种类型的JSON
    std::string mixedTypeJSON = R"({
        "intValue": 42,
        "stringValue": "hello",
        "boolValue": true,
        "nullValue": null,
        "arrayValue": [1, 2, 3],
        "objectValue": {"key": "value"}
    })";
    
    JsonValue parsed = JsonValue::parse(mixedTypeJSON);
    
    // 测试将对象转换为不兼容的类型
    try {
        int invalidInt = parsed["objectValue"].toInt();
        std::cout << "Unexpected successful conversion of object to int" << std::endl;
    } catch (...) {
        std::cout << "Correctly failed to convert object to int" << std::endl;
    }
    
    // 测试将数组转换为不兼容的类型
    try {
        std::string invalidString = parsed["arrayValue"].toString();
        std::cout << "Unexpected successful conversion of array to string" << std::endl;
    } catch (...) {
        std::cout << "Correctly failed to convert array to string" << std::endl;
    }
    
    // 测试将字符串转换为数字（无效格式）
    std::string invalidNumberJSON = "{\"value\": \"not_a_number\"}";
    JsonValue invalidNumberParsed = JsonValue::parse(invalidNumberJSON);
    
    try {
        int invalidInt = invalidNumberParsed["value"].toInt();
        std::cout << "Converted invalid string to int: " << invalidInt << std::endl;
    } catch (...) {
        std::cout << "Correctly failed to convert invalid string to int" << std::endl;
    }
    
    // 测试空值转换
    try {
        int nullToInt = parsed["nullValue"].toInt();
        std::cout << "Converted null to int: " << nullToInt << std::endl;
    } catch (...) {
        std::cout << "Correctly failed to convert null to int" << std::endl;
    }
}

int main() {
    std::cout << "=== Extreme Exception Scenarios Tests ===" << std::endl;
    
    int result = RUN_ALL_TESTS();
    
    if (result == 0) {
        std::cout << "✅ All extreme exception scenarios tests PASSED!" << std::endl;
    } else {
        std::cout << "❌ Some extreme exception scenarios tests FAILED!" << std::endl;
    }
    
    return result;
}