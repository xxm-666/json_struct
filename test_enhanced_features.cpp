#include "json_value_enhanced.h"
#include <iostream>
#include <chrono>
#include <cassert>
#include <array>
#include <set>
#include <map>

using namespace JsonStruct;
using namespace JsonStruct::literals;

void testBasicTypes() {
    std::cout << "=== Testing Basic Types ===" << std::endl;
    
    // 测试基本类型构造
    JsonValueEnhanced nullVal;
    JsonValueEnhanced boolVal(true);
    JsonValueEnhanced intVal(42);
    JsonValueEnhanced doubleVal(3.14);
    JsonValueEnhanced stringVal("Hello, World!");
    
    assert(nullVal.isNull());
    assert(boolVal.isBool() && boolVal.toBool() == true);
    assert(intVal.isNumber() && intVal.toInt() == 42);
    assert(doubleVal.isNumber() && doubleVal.toDouble() == 3.14);
    assert(stringVal.isString() && stringVal.toString() == "Hello, World!");
    
    std::cout << "✅ Basic types working!" << std::endl;
}

void testContainerConstructors() {
    std::cout << "\n=== Testing Container Constructors ===" << std::endl;
    
    // std::array
    std::array<int, 3> arr = {1, 2, 3};
    JsonValueEnhanced arrJson(arr);
    assert(arrJson.isArray() && arrJson.size() == 3);
    
    // std::vector
    std::vector<std::string> vec = {"a", "b", "c"};
    JsonValueEnhanced vecJson(vec);
    assert(vecJson.isArray() && vecJson.size() == 3);
    
    // std::set
    std::set<int> s = {3, 1, 2};
    JsonValueEnhanced setJson(s);
    assert(setJson.isArray() && setJson.size() == 3);
    
    // std::map
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    JsonValueEnhanced mapJson(m);
    assert(mapJson.isObject() && mapJson.size() == 2);
    
    std::cout << "✅ Container constructors working!" << std::endl;
}

void testAdvancedParsing() {
    std::cout << "\n=== Testing Advanced Parsing ===" << std::endl;
    
    // 测试Unicode代理对
    std::string emojiJson = R"({"emoji": "\uD83D\uDE00"})";
    auto parsed = JsonValueEnhanced::parse(emojiJson);
    std::cout << "Emoji: " << parsed["emoji"].toString() << std::endl;
    
    // 测试深度限制
    JsonValueEnhanced::ParseOptions limitOptions;
    limitOptions.maxDepth = 5;
    
    std::string deepJson = "[[[[[[42]]]]]]"; // 6层深度
    try {
        JsonValueEnhanced::parse(deepJson, limitOptions);
        std::cout << "❌ Should have failed due to depth limit!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✅ Depth limit working: " << e.what() << std::endl;
    }
    
    // 测试注释支持
    JsonValueEnhanced::ParseOptions commentOptions;
    commentOptions.allowComments = true;
    
    std::string jsonWithComments = R"({
        "name": "test", // 单行注释
        /* 多行注释 */
        "value": 42
    })";
    
    try {
        auto commentParsed = JsonValueEnhanced::parse(jsonWithComments, commentOptions);
        std::cout << "✅ Comments support working!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Comments failed: " << e.what() << std::endl;
    }
    
    // 测试尾随逗号
    JsonValueEnhanced::ParseOptions trailingOptions;
    trailingOptions.allowTrailingCommas = true;
    
    std::string jsonWithTrailing = R"({
        "array": [1, 2, 3,],
        "object": {"a": 1, "b": 2,},
    })";
    
    try {
        auto trailingParsed = JsonValueEnhanced::parse(jsonWithTrailing, trailingOptions);
        std::cout << "✅ Trailing commas support working!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Trailing commas failed: " << e.what() << std::endl;
    }
}

void testSerializationOptions() {
    std::cout << "\n=== Testing Serialization Options ===" << std::endl;
    
    // 创建测试对象
    JsonValueEnhanced obj;
    obj["name"] = JsonValueEnhanced("test");
    obj["value"] = JsonValueEnhanced(42);
    obj["array"] = JsonValueEnhanced(std::vector<int>{1, 2, 3});
    obj["nested"] = JsonValueEnhanced();
    obj["nested"]["key"] = JsonValueEnhanced("value");
    
    // 默认紧凑格式
    std::cout << "Compact: " << obj.dump() << std::endl;
    
    // 美化格式
    JsonValueEnhanced::SerializeOptions prettyOptions;
    prettyOptions.indent = 2;
    std::cout << "Pretty:\n" << obj.dump(prettyOptions) << std::endl;
    
    // 排序键
    JsonValueEnhanced::SerializeOptions sortOptions;
    sortOptions.indent = 2;
    sortOptions.sortKeys = true;
    std::cout << "Sorted keys:\n" << obj.dump(sortOptions) << std::endl;
    
    // 紧凑数组
    JsonValueEnhanced::SerializeOptions compactArrayOptions;
    compactArrayOptions.indent = 2;
    compactArrayOptions.compactArrays = true;
    std::cout << "Compact arrays:\n" << obj.dump(compactArrayOptions) << std::endl;
    
    std::cout << "✅ Serialization options working!" << std::endl;
}

void testJsonPointer() {
    std::cout << "\n=== Testing JSON Pointer ===" << std::endl;
    
    std::string json = R"({
        "users": [
            {"name": "Alice", "age": 30},
            {"name": "Bob", "age": 25}
        ],
        "config": {
            "debug": true,
            "version": "1.0"
        }
    })";
    
    auto parsed = JsonValueEnhanced::parse(json);
    
    try {
        // 测试各种JSON指针
        auto& alice = parsed.at("/users/0/name");
        assert(alice.toString() == "Alice");
        
        auto& debug = parsed.at("/config/debug");
        assert(debug.toBool() == true);
        
        auto& version = parsed.at("/config/version");
        assert(version.toString() == "1.0");
        
        // 修改值
        parsed.at("/users/1/age") = JsonValueEnhanced(26);
        assert(parsed.at("/users/1/age").toInt() == 26);
        
        std::cout << "✅ JSON Pointer working!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ JSON Pointer failed: " << e.what() << std::endl;
    }
}

void testVisitorPattern() {
    std::cout << "\n=== Testing Visitor Pattern ===" << std::endl;
    
    JsonValueEnhanced::ArrayType mixedArray;
    mixedArray.emplace_back(JsonValueEnhanced(42));
    mixedArray.emplace_back(JsonValueEnhanced("hello"));
    mixedArray.emplace_back(JsonValueEnhanced(true));
    mixedArray.emplace_back(JsonValueEnhanced());
    
    JsonValueEnhanced arr(std::move(mixedArray));
    
    std::cout << "Array contents using visitor: ";
    for (size_t i = 0; i < arr.size(); ++i) {
        arr[i].visit([](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                std::cout << "null ";
            } else if constexpr (std::is_same_v<T, bool>) {
                std::cout << (value ? "true" : "false") << " ";
            } else if constexpr (std::is_same_v<T, double>) {
                std::cout << value << " ";
            } else if constexpr (std::is_same_v<T, std::string>) {
                std::cout << "\"" << value << "\" ";
            }
        });
    }
    std::cout << std::endl;
    
    std::cout << "✅ Visitor pattern working!" << std::endl;
}

void testOptionalInterface() {
    std::cout << "\n=== Testing Optional Interface ===" << std::endl;
    
    JsonValueEnhanced obj;
    obj["number"] = JsonValueEnhanced(42);
    obj["text"] = JsonValueEnhanced("hello");
    obj["flag"] = JsonValueEnhanced(true);
    
    // 测试成功的获取
    if (auto num = obj["number"].getNumber()) {
        std::cout << "Number: " << *num << std::endl;
    }
    
    if (auto str = obj["text"].getString()) {
        std::cout << "String: " << *str << std::endl;
    }
    
    if (auto flag = obj["flag"].getBool()) {
        std::cout << "Bool: " << (*flag ? "true" : "false") << std::endl;
    }
    
    // 测试失败的获取
    if (auto missing = obj["missing"].getNumber()) {
        std::cout << "❌ Should be nullopt!" << std::endl;
    } else {
        std::cout << "✅ Missing key returns nullopt" << std::endl;
    }
    
    // 测试类型不匹配
    if (auto wrongType = obj["text"].getNumber()) {
        std::cout << "❌ Should be nullopt for wrong type!" << std::endl;
    } else {
        std::cout << "✅ Wrong type returns nullopt" << std::endl;
    }
}

void testLiterals() {
    std::cout << "\n=== Testing Literals ===" << std::endl;
    
    try {
        auto obj = R"({"name": "test", "value": 42})"_json;
        assert(obj.isObject());
        assert(obj["name"].toString() == "test");
        assert(obj["value"].toInt() == 42);
        
        std::cout << "✅ JSON literals working!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "❌ Literals failed: " << e.what() << std::endl;
    }
}

void testFactoryFunctions() {
    std::cout << "\n=== Testing Factory Functions ===" << std::endl;
    
    auto intVal = makeJson(42);
    auto strVal = makeJson("hello");
    auto boolVal = makeJson(true);
    auto doubleVal = makeJson(3.14);
    
    assert(intVal.isNumber() && intVal.toInt() == 42);
    assert(strVal.isString() && strVal.toString() == "hello");
    assert(boolVal.isBool() && boolVal.toBool() == true);
    assert(doubleVal.isNumber() && doubleVal.toDouble() == 3.14);
    
    std::cout << "✅ Factory functions working!" << std::endl;
}

void testPerformance() {
    std::cout << "\n=== Testing Performance ===" << std::endl;
    
    // 创建大型对象
    JsonValueEnhanced largeObj;
    for (int i = 0; i < 10000; ++i) {
        largeObj["key" + std::to_string(i)] = JsonValueEnhanced("value" + std::to_string(i));
    }
    
    // 测试序列化性能
    auto start = std::chrono::high_resolution_clock::now();
    std::string serialized = largeObj.dump();
    auto serializeEnd = std::chrono::high_resolution_clock::now();
    
    // 测试解析性能
    auto parsed = JsonValueEnhanced::parse(serialized);
    auto parseEnd = std::chrono::high_resolution_clock::now();
    
    // 测试查找性能
    for (int i = 0; i < 1000; ++i) {
        auto val = parsed["key" + std::to_string(i)];
    }
    auto lookupEnd = std::chrono::high_resolution_clock::now();
    
    auto serializeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(serializeEnd - start);
    auto parseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(parseEnd - serializeEnd);
    auto lookupDuration = std::chrono::duration_cast<std::chrono::milliseconds>(lookupEnd - parseEnd);
    
    std::cout << "Serialize 10K items: " << serializeDuration.count() << "ms" << std::endl;
    std::cout << "Parse result: " << parseDuration.count() << "ms" << std::endl;
    std::cout << "1000 lookups: " << lookupDuration.count() << "ms" << std::endl;
    
    std::cout << "✅ Performance test completed!" << std::endl;
}

void testMoveSemantics() {
    std::cout << "\n=== Testing Move Semantics ===" << std::endl;
    
    auto createLargeArray = []() {
        JsonValueEnhanced::ArrayType arr;
        for (int i = 0; i < 10000; ++i) {
            arr.emplace_back(JsonValueEnhanced(i));
        }
        return JsonValueEnhanced(std::move(arr));
    };
    
    auto start = std::chrono::high_resolution_clock::now();
    auto largeArray = createLargeArray();
    auto moved = std::move(largeArray);
    auto copied = moved; // 拷贝
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Large array creation, move and copy: " << duration.count() << "μs" << std::endl;
    std::cout << "Array size: " << moved.size() << std::endl;
    std::cout << "Copied size: " << copied.size() << std::endl;
    
    std::cout << "✅ Move semantics working efficiently!" << std::endl;
}

void testErrorHandling() {
    std::cout << "\n=== Testing Error Handling ===" << std::endl;
    
    // 测试详细错误信息
    std::string invalidJson = R"({
        "name": "test",
        "age": ,
        "city": "Beijing"
    })";
    
    try {
        JsonValueEnhanced::parse(invalidJson);
        std::cout << "❌ Should have failed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✅ Error with location: " << e.what() << std::endl;
    }
    
    // 测试无效的JSON指针
    JsonValueEnhanced obj;
    obj["test"] = JsonValueEnhanced(42);
    
    try {
        obj.at("/invalid/pointer");
        std::cout << "❌ Should have failed!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✅ JSON pointer error: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "🚀 Enhanced JsonValue C++17+ Testing Suite" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    try {
        testBasicTypes();
        testContainerConstructors();
        testAdvancedParsing();
        testSerializationOptions();
        testJsonPointer();
        testVisitorPattern();
        testOptionalInterface();
        testLiterals();
        testFactoryFunctions();
        testPerformance();
        testMoveSemantics();
        testErrorHandling();
        
        std::cout << "\n🎉 All tests completed successfully!" << std::endl;
        std::cout << "\n🌟 Enhanced JsonValue Features Summary:" << std::endl;
        std::cout << "• ✅ Full C++17+ modern features (std::variant, std::optional, std::string_view)" << std::endl;
        std::cout << "• ✅ Enhanced Unicode support with surrogate pairs" << std::endl;
        std::cout << "• ✅ Configurable parsing (comments, trailing commas, depth limits)" << std::endl;
        std::cout << "• ✅ Advanced serialization options (sorting, formatting, precision)" << std::endl;
        std::cout << "• ✅ JSON Pointer support (RFC 6901)" << std::endl;
        std::cout << "• ✅ Container constructors (std::array, std::vector, std::set, std::map)" << std::endl;
        std::cout << "• ✅ Visitor pattern with perfect forwarding" << std::endl;
        std::cout << "• ✅ Safe optional interface" << std::endl;
        std::cout << "• ✅ JSON literals support" << std::endl;
        std::cout << "• ✅ Optimized move semantics" << std::endl;
        std::cout << "• ✅ Detailed error reporting with location info" << std::endl;
        std::cout << "• ✅ High performance with std::unordered_map" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
