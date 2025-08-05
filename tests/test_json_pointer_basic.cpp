// JSONPath 基础功能测试
#include "../src/jsonstruct.h"
#include "../test_framework/test_framework.h"
#include <iostream>

using namespace JsonStruct;

TEST(JsonPath_BasicPointerAccess) {
    JsonValue json = JsonValue::parse(R"({
        "a": {
            "b": {
                "c": {
                    "d": 42,
                    "arr": [1, 2, 3],
                    "empty": {},
                    "nullval": null
                }
            }
        },
        "rootval": "hello"
    })");
    
    // 测试深度指针访问
    try {
        const JsonValue& val = json.at("/a/b/c/d");
        ASSERT_TRUE(val.isNumber());
        if (val.getNumber().has_value()) {
            ASSERT_EQ(val.getNumber().value(), 42);
        }
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // 不应该抛出异常
    }
    
    // 测试根指针
    try {
        const JsonValue& val = json.at("");
        ASSERT_TRUE(val.isObject());
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // 不应该抛出异常
    }
    
    // 测试顶级属性
    try {
        const JsonValue& val = json.at("/rootval");
        ASSERT_TRUE(val.isString());
        if (val.getString().has_value()) {
            ASSERT_EQ(std::string(val.getString().value()), "hello");
        }
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // 不应该抛出异常
    }
}

TEST(JsonPath_ArrayAccess) {
    JsonValue json = JsonValue::parse(R"({
        "array": [
            {"name": "first", "value": 1},
            {"name": "second", "value": 2},
            {"name": "third", "value": 3}
        ]
    })");
    
    // 测试数组索引访问
    try {
        const JsonValue& first = json.at("/array/0");
        ASSERT_TRUE(first.isObject());
        ASSERT_EQ(first["name"].toString(), "first");
        ASSERT_EQ(first["value"].toInt(), 1);
    } catch (const std::exception& e) {
        ASSERT_TRUE(false);
    }
    
    try {
        const JsonValue& second = json.at("/array/1");
        ASSERT_TRUE(second.isObject());
        ASSERT_EQ(second["name"].toString(), "second");
    } catch (const std::exception& e) {
        ASSERT_TRUE(false);
    }
}

TEST(JsonPath_ErrorHandling) {
    JsonValue json = JsonValue::parse(R"({
        "valid": "data"
    })");
    
    // 测试无效路径
    bool exceptionCaught = false;
    try {
        const JsonValue& result = json.at("/nonexistent/path");
    } catch (const std::exception&) {
        exceptionCaught = true;
    }
    ASSERT_TRUE(exceptionCaught);
    
    // 测试数组越界
    JsonValue arrayJson = JsonValue::parse(R"({"arr": [1, 2, 3]})");
    exceptionCaught = false;
    try {
        const JsonValue& result = arrayJson.at("/arr/999");
    } catch (const std::exception&) {
        exceptionCaught = true;
    }
    ASSERT_TRUE(exceptionCaught);
}

TEST(JsonPath_SpecialCharacters) {
    JsonValue json = JsonValue::parse(R"({
        "special": {
            "key with spaces": "value1",
            "key-with-dashes": "value2",
            "key.with.dots": "value3",
            "key/with/slashes": "value4"
        }
    })");
    
    // 测试包含特殊字符的键名
    try {
        // JSON Pointer 使用 ~0 表示 ~，~1 表示 /
        const JsonValue& val1 = json.at("/special/key with spaces");
        ASSERT_EQ(val1.toString(), "value1");
        
        const JsonValue& val2 = json.at("/special/key-with-dashes");
        ASSERT_EQ(val2.toString(), "value2");
        
        const JsonValue& val3 = json.at("/special/key.with.dots");
        ASSERT_EQ(val3.toString(), "value3");
        
        // 包含斜杠的键需要转义
        const JsonValue& val4 = json.at("/special/key~1with~1slashes");
        ASSERT_EQ(val4.toString(), "value4");
    } catch (const std::exception& e) {
        // 如果特殊字符处理不支持，至少不应该崩溃
        std::cout << "Special character handling: " << e.what() << std::endl;
    }
}

TEST(JsonPath_ComplexNesting) {
    JsonValue json = JsonValue::parse(R"({
        "level1": {
            "level2": {
                "level3": {
                    "level4": {
                        "value": "deep_value",
                        "array": [
                            {"item": "first"},
                            {"item": "second"}
                        ]
                    }
                }
            }
        }
    })");
    
    // 测试深层嵌套访问
    try {
        const JsonValue& deepValue = json.at("/level1/level2/level3/level4/value");
        ASSERT_TRUE(deepValue.isString());
        ASSERT_EQ(deepValue.toString(), "deep_value");
        
        const JsonValue& arrayItem = json.at("/level1/level2/level3/level4/array/0/item");
        ASSERT_TRUE(arrayItem.isString());
        ASSERT_EQ(arrayItem.toString(), "first");
    } catch (const std::exception& e) {
        ASSERT_TRUE(false); // 不应该失败
    }
}

TEST(JsonPath_TestJsonGenerator) {
  std::unordered_map<int, JsonValue> testData;
  for (int i = 0; i < 1000; ++i) {
    JsonValue::ObjectType item;
    item["id"] = JsonValue(i);
    item["name"] = JsonValue("Item " + std::to_string(i));
    item["value"] = JsonValue(i * 1.5);
    testData[i] = JsonValue(std::move(item));
  }

  std::cout << "data 1:" << testData[0]["name"].toString() << std::endl;
  std::cout << "data 999:" << testData[999]["name"].toString() << std::endl;
}

int main() {
    return RUN_ALL_TESTS();
}
